/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "ocpp/ocpp.h"
#include "ocpp/list.h"
#include "ocpp/strconv.h"

#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>

#if !defined(OCPP_DEBUG)
#define OCPP_DEBUG(...)
#endif
#if !defined(OCPP_INFO)
#define OCPP_INFO(...)
#endif
#if !defined(OCPP_ERROR)
#define OCPP_ERROR(...)
#endif

#if !defined(OCPP_DEFAULT_TX_RETRIES)
#define OCPP_DEFAULT_TX_RETRIES			3
#endif

#define container_of(ptr, type, member)		\
	((type *)(void *)((char *)(ptr) - offsetof(type, member)))

struct ocpp_backend {
	struct ocpp_backend_api api;
};

struct backend_foreach_ctx {
	ocpp_iterate_cb_t user_cb;
	void *user_ctx;
};

/* NOTE: Messages sent to the backend must be flat, but the payload of messages
 * processed within the OCPP module is implemented to be handled as a pointer.
 * In other words, the `ocpp_backend_message.payload` field is serialized as
 * flat only when being sent to the backend. Within the OCPP module, the
 * `payload` field is used as a pointer. Refer to the `get/set_payload_ptr`
 * functions. */
struct ocpp_message {
	struct ocpp_backend_message data;
};

struct message {
	struct list link;
	uint32_t attempts; /**< The number of message sending attempts. */
	time_t expiry;
	bool stored; /**< Whether the message is stored in backend. */
	struct ocpp_message body; /* must be last */
};

typedef void (*list_add_func_t)(struct message *);

static struct {
	struct ocpp_backend *backend;

	ocpp_event_callback_t event_callback;
	void *event_callback_ctx;

	struct {
		struct list ready;
		struct list wait;
		struct list timer;
		struct list dead; /* keep requests to be freed later due to
				possible ongoing processing in user handler */
		time_t timestamp;
	} tx;

	struct {
		time_t timestamp;
	} rx;

	bool boot_accepted;
} m;

static void add_last_to_list(struct message *msg, struct list *head)
{
	list_add_tail(&msg->link, head);
}

static void add_first_to_list(struct message *msg, struct list *head)
{
	list_add(&msg->link, head);
}

static void del_from_list(struct message *msg, struct list *head)
{
	list_del(&msg->link, head);
}

static void put_msg_ready_infront(struct message *msg)
{
	add_first_to_list(msg, &m.tx.ready);
	OCPP_DEBUG("%s pushed in front to ready list",
			ocpp_stringify_type(msg->body.data.header.type));
}

static void put_msg_ready(struct message *msg)
{
	add_last_to_list(msg, &m.tx.ready);
	OCPP_DEBUG("%s pushed to ready list",
			ocpp_stringify_type(msg->body.data.header.type));
}

static void put_msg_wait(struct message *msg)
{
	add_last_to_list(msg, &m.tx.wait);
	OCPP_DEBUG("%s pushed to wait list",
			ocpp_stringify_type(msg->body.data.header.type));
}

static void put_msg_timer(struct message *msg)
{
	add_last_to_list(msg, &m.tx.timer);
	OCPP_DEBUG("%s pushed to timer list",
			ocpp_stringify_type(msg->body.data.header.type));
}

static void put_msg_dead(struct message *msg)
{
	add_first_to_list(msg, &m.tx.dead);
	OCPP_DEBUG("%s pushed to dead list",
			ocpp_stringify_type(msg->body.data.header.type));
}

static void del_msg_ready(struct message *msg)
{
	del_from_list(msg, &m.tx.ready);
	OCPP_DEBUG("%s removed from ready list",
			ocpp_stringify_type(msg->body.data.header.type));
}

static void del_msg_wait(struct message *msg)
{
	del_from_list(msg, &m.tx.wait);
	OCPP_DEBUG("%s removed from wait list",
			ocpp_stringify_type(msg->body.data.header.type));
}

static void del_msg_timer(struct message *msg)
{
	del_from_list(msg, &m.tx.timer);
	OCPP_DEBUG("%s removed from timer list",
			ocpp_stringify_type(msg->body.data.header.type));
}

static int count_messages_waiting(void)
{
	return list_count(&m.tx.wait);
}

static int count_messages_ticking(void)
{
	return list_count(&m.tx.timer);
}

static int count_messages_ready(void)
{
	return list_count(&m.tx.ready);
}

static int count_backend_messages(void)
{
	if (m.backend && m.backend->api.count) {
		return m.backend->api.count(m.backend);
	}
	return 0;
}

static void set_payload_ptr(struct ocpp_message *msg,
		void *ptr, size_t size)
{
	memcpy(msg->data.payload, &ptr, sizeof(ptr));
	msg->data.payload_size = (uint32_t)size;
}

static void *get_payload_ptr(const struct ocpp_message *msg)
{
	void *ptr;
	memcpy(&ptr, msg->data.payload, sizeof(ptr));
	return ptr;
}

static bool is_payload_ptr_null(const struct ocpp_message *msg)
{
	return msg->data.payload_size == 0 || get_payload_ptr(msg) == NULL;
}

static bool is_boot_accepted(void)
{
	return m.boot_accepted;
}

static void set_boot_accepted(bool accepted)
{
	m.boot_accepted = accepted;
}

static void update_last_tx_timestamp(const time_t *now)
{
	m.tx.timestamp = *now;
	OCPP_DEBUG("Last TX timestamp: %ld", m.tx.timestamp);
}

static void update_last_rx_timestamp(const time_t *now)
{
	m.rx.timestamp = *now;
	OCPP_DEBUG("Last RX timestamp: %ld", m.rx.timestamp);
}

static uint32_t get_elapsed_since_last_message(const time_t *now)
{
	const time_t last = m.tx.timestamp > m.rx.timestamp?
		m.tx.timestamp : m.rx.timestamp;
	return (uint32_t)(*now - last);
}

static void dispatch_event(ocpp_event_t event_type,
		const struct ocpp_message *msg)
{
	if (m.event_callback) {
		ocpp_unlock();
		(*m.event_callback)(event_type, msg, m.event_callback_ctx);
		ocpp_lock();
	}
}

static struct message *alloc_message(void)
{
	/* allocate extra space for payload pointer */
	return calloc(1, sizeof(struct message) + sizeof(void *));
}

static void free_message(struct message *msg, bool notify)
{
	if (notify) {
		dispatch_event(OCPP_EVENT_MESSAGE_FREE, &msg->body);
	}

	if (msg->stored) {
		ocpp_unlock();
		/* the first one must be the msg. foreach is not used here to
		 * avoid latency and complexity. */
		m.backend->api.drop(m.backend, 1);
		ocpp_lock();
	}

	if (!is_payload_ptr_null(&msg->body)) {
		free(get_payload_ptr(&msg->body));
	}

	free(msg);
}

static void clear_dead_messages(void)
{
	struct list *p, *n;

	list_for_each_safe(p, n, &m.tx.dead) {
		list_del(p, &m.tx.dead);
		struct message *msg = container_of(p, struct message, link);
		free_message(msg, true);
	}
}

static void set_header(struct ocpp_backend_message_header *header,
		ocpp_message_t type, const char *id, bool err, void *ctx)
{
	header->type = type;
	header->timestamp = time(NULL);
	header->custom = (uintptr_t)ctx;

	if (id) {
		header->role = err?
			OCPP_MSG_ROLE_CALLERROR : OCPP_MSG_ROLE_CALLRESULT;
		memcpy(header->id, id, sizeof(header->id));
	} else {
		header->role = OCPP_MSG_ROLE_CALL;
		ocpp_generate_message_id(header->id, sizeof(header->id));
	}
}

static struct message *new_message(const char *id,
		ocpp_message_t type, bool err, void *ctx)
{
	struct message *msg = alloc_message();

	if (msg == NULL) {
		return NULL;
	}

	set_header(&msg->body.data.header, type, id, err, ctx);
	set_payload_ptr(&msg->body, NULL, 0);
	msg->attempts = 0;

	return msg;
}

static struct message *find_msg_by_idstr(struct list *list_head,
		const char *msgid)
{
	struct list *p;

	list_for_each(p, list_head) {
		struct message *msg = container_of(p, struct message, link);
		if (strcmp(msgid, msg->body.data.header.id) == 0) {
			return msg;
		}
	}

	return NULL;
}

static int push_message(const char *id, ocpp_message_t type,
		const void *data, size_t datasize,
		time_t timer, list_add_func_t f, bool err, void *ctx)
{
	struct message *msg = new_message(id, type, err, ctx);
	uint8_t *payload = NULL;

	if (!msg) {
		return -ENOMEM;
	}
	if (data && datasize && !(payload = (uint8_t *)malloc(datasize))) {
		free_message(msg, false);
		return -ENOMEM;
	}

	if (data && datasize) {
		memcpy(payload, data, datasize);
		set_payload_ptr(&msg->body, payload, datasize);
	}

	msg->expiry = timer;

	(*f)(msg);

	return 0;
}

static int push_message_backend(const char *id, ocpp_message_t type,
		const void *data, size_t datasize,
		bool callerr, bool preemptive, void *ctx)
{
	struct ocpp_backend_message *msg = (struct ocpp_backend_message *)
		calloc(1, sizeof(struct ocpp_backend_message) + datasize);

	if (!msg) {
		return -ENOMEM;
	}

	set_header(&msg->header, type, id, callerr, ctx);
	msg->payload_size = datasize;

	if (data && datasize > 0) {
		memcpy(msg->payload, data, datasize);
	}

	/* FIXME: When pushing to the front, if there are already pending
	 * messages, the new message should be inserted after the pending ones
	 * to maintain the correct order. */
	int err = preemptive?
		m.backend->api.push_front(m.backend, msg) :
		m.backend->api.push(m.backend, msg);

	free(msg);

	return err;
}

static bool is_transaction_related(const struct message *msg)
{
	switch (msg->body.data.header.type) {
	case OCPP_MSG_START_TRANSACTION: /* fall through */
	case OCPP_MSG_STOP_TRANSACTION: /* fall through */
	case OCPP_MSG_METER_VALUES:
		return true;
	default:
		return false;
	}
}

static bool is_droppable(const struct message *msg)
{
	/* never drop BootNotification and transaction-related messages. */
	return !is_transaction_related(msg) &&
		msg->body.data.header.type != OCPP_MSG_BOOTNOTIFICATION;
}

static bool should_drop(struct message *msg)
{
	const uint32_t max_attempts = OCPP_DEFAULT_TX_RETRIES;

	if (!is_droppable(msg) || msg->attempts < max_attempts) {
		return false;
	}

	return true;
}

static bool should_send_heartbeat(const time_t *now)
{
	uint32_t interval;
	ocpp_get_configuration("HeartbeatInterval",
			&interval, sizeof(interval), 0);
	const bool disabled = interval == 0;
	const uint32_t elapsed = get_elapsed_since_last_message(now);

	if (disabled || elapsed < interval || !is_boot_accepted() ||
			count_messages_ready() > 0 ||
			count_messages_waiting() > 0) {
		return false;
	}

	return true;
}

/* Retry interval for the message that is not delivered to the server. */
static time_t get_retry_interval(const struct message *msg, const time_t *now)
{
	(void)msg;
	uint32_t interval = OCPP_DEFAULT_TX_TIMEOUT_SEC;
	return *now + interval;
}

/* Next period to send the message that is delivered to the server, but not
 * processed properly by the server. */
static time_t get_next_period(const struct message *msg, const time_t *now)
{
	uint32_t interval = OCPP_DEFAULT_TX_TIMEOUT_SEC;

	if (is_transaction_related(msg)) {
		ocpp_get_configuration("TransactionMessageRetryInterval",
				&interval, sizeof(interval), 0);
		interval = interval * msg->attempts;
	} else if (msg->body.data.header.type == OCPP_MSG_BOOTNOTIFICATION ||
			msg->body.data.header.type == OCPP_MSG_HEARTBEAT) {
		ocpp_get_configuration("HeartbeatInterval",
				&interval, sizeof(interval), 0);
	}

	return *now + interval;
}

static void update_message_expiry(struct message *msg, const time_t *now)
{
	msg->expiry = get_next_period(msg, now);
}

static void send_message(struct message *msg, const time_t *now)
{
	msg->attempts++;
	msg->expiry = get_retry_interval(msg, now);

	del_msg_ready(msg);

	OCPP_INFO("tx: %s.req (%d/%d) waiting up to %lu seconds",
			ocpp_stringify_type(msg->body.data.header.type),
			msg->attempts, OCPP_DEFAULT_TX_RETRIES,
			(unsigned long)(msg->expiry - *now));

	if (ocpp_send(&msg->body) == 0) {
		if (msg->body.data.header.role == OCPP_MSG_ROLE_CALL) {
			put_msg_wait(msg);
			return;
		}
	} else {
		if (msg->body.data.header.type == OCPP_MSG_BOOTNOTIFICATION ||
				msg->attempts < OCPP_DEFAULT_TX_RETRIES ||
				is_transaction_related(msg)) {
			put_msg_wait(msg);
			return;
		}
	}

	free_message(msg, true);
}

static void process_tx_timeout(const time_t *now)
{
	struct list *p;
	struct list *t;

	list_for_each_safe(p, t, &m.tx.wait) {
		struct message *msg = container_of(p, struct message, link);
		if (msg->expiry > *now) {
			continue;
		}

		del_msg_wait(msg);

		if (should_drop(msg)) {
			OCPP_INFO("Dropping message %s", ocpp_stringify_type(
					msg->body.data.header.type));
			free_message(msg, true);
		} else {
			OCPP_INFO("Retrying message %s", ocpp_stringify_type(
					msg->body.data.header.type));
			put_msg_ready_infront(msg);
		}
	}
}

static int process_queued_messages(const time_t *now)
{
	process_tx_timeout(now);

	/* do not send a message if there is a message waiting for a response.
	 * This is to prevent the server from being overwhelmed by the client,
	 * sending multiple messages before the server responds to the previous
	 * message. */
	if (count_messages_waiting() > 0) {
		return -EBUSY;
	}

	struct list *p;
	struct list *t;

	list_for_each_safe(p, t, &m.tx.ready) {
		struct message *msg = container_of(p, struct message, link);
		send_message(msg, now);
		return 0; /* send one by one */
	}

	return 0;
}

static int process_periodic_messages(const time_t *now, size_t nr_msg_stored)
{
	if (should_send_heartbeat(now) && !nr_msg_stored) {
		struct message *msg = new_message(NULL,
				OCPP_MSG_HEARTBEAT, false, NULL);

		if (!msg) {
			return -ENOMEM;
		}

		put_msg_ready(msg);
		process_queued_messages(now);
	}

	return 0;
}

static int process_timer_messages(const time_t *now)
{
	if (count_messages_ticking() <= 0) {
		return 0;
	}

	struct list *p;
	struct list *t;

	list_for_each_safe(p, t, &m.tx.timer) {
		struct message *msg = container_of(p, struct message, link);
		if (msg->expiry > *now) {
			continue;
		}

		del_msg_timer(msg);
		put_msg_ready(msg);
	}

	return 0;
}

static void process_central_request(const struct ocpp_message *r)
{
	(void)r;
	OCPP_INFO("rx: %s.req", ocpp_stringify_type(r->data.header.type));
}

static bool process_central_response_error(const struct ocpp_message *r,
		struct message *req, const time_t *now)
{
	(void)r;

	if (!is_transaction_related(req)) {
		return true;
	}

	uint32_t max_attempts = OCPP_DEFAULT_TX_RETRIES;

	ocpp_get_configuration("TransactionMessageAttempts",
			&max_attempts, sizeof(max_attempts), NULL);

	if (req->attempts < max_attempts) {
		update_message_expiry(req, now);
		put_msg_wait(req);

		OCPP_INFO("%s will be sent again at %lu (%d/%d)",
				ocpp_stringify_type(req->body.data.header.type),
				(unsigned long)req->expiry,
				req->attempts, max_attempts);
		return false;
	}

	return true;
}

static bool process_central_response_result(const struct ocpp_message *r,
		struct message *req, const time_t *now)
{
	(void)req;
	(void)now;
	const struct ocpp_backend_message_header *h = &r->data.header;

	if (h->type == OCPP_MSG_BOOTNOTIFICATION) {
		const struct ocpp_BootNotification_conf *p =
			(const struct ocpp_BootNotification_conf *)
			get_payload_ptr(r);

		if (p && p->status == OCPP_BOOT_STATUS_ACCEPTED) {
			set_boot_accepted(true);
		}
	}

	return true;
}

static int process_central_response(const struct ocpp_message *r,
		const time_t *now)
{
	const struct ocpp_backend_message_header *h = &r->data.header;
	struct message *req = find_msg_by_idstr(&m.tx.wait, h->id);
	bool done = true;

	if (req == NULL) {
		OCPP_ERROR("No matching request for response %s",
				ocpp_stringify_type(r->data.header.type));
		return -ENOLINK;
	}

	del_msg_wait(req);
	OCPP_INFO("rx: %s.conf", ocpp_stringify_type(h->type));

	if (h->role == OCPP_MSG_ROLE_CALLRESULT) {
		done = process_central_response_result(r, req, now);
	} else if (h->role == OCPP_MSG_ROLE_CALLERROR) {
		done = process_central_response_error(r, req, now);
	} else {
		OCPP_ERROR("Invalid message role: %d", h->role);
	}

	/* Note that tx timestamp is updated when the response of the message is
	 * received. */
	update_last_tx_timestamp(now);

	if (done) {
		put_msg_dead(req);
	}

	return 0;
}

static int process_incoming_messages(const time_t *now)
{
	/* allocate extra space for payload pointer */
	uint8_t buf[sizeof(struct ocpp_message) + sizeof(void *)] = { 0, };
	struct ocpp_message *r = (struct ocpp_message *)buf;
	const struct ocpp_backend_message_header *h = &r->data.header;

	ocpp_unlock();
	int err = ocpp_recv(r);
	ocpp_lock();

	if (err && err != -ENOTSUP) {
		goto out;
	}

	switch (h->role) {
	case OCPP_MSG_ROLE_CALL:
		process_central_request(r);
		break;
	case OCPP_MSG_ROLE_CALLRESULT: /* fall through */
	case OCPP_MSG_ROLE_CALLERROR:
		err = process_central_response(r, now);
		break;
	default:
		err = -EINVAL;
		OCPP_ERROR("Invalid message role: %d", h->role);
		break;
	}

	update_last_rx_timestamp(now);
	dispatch_event(err, r);
	clear_dead_messages();
out:
	if (err && err != -ENOENT && h->role == OCPP_MSG_ROLE_CALL) {
		/* Send CallError if the message could not be processed. */
		push_message(h->id, h->type, NULL, 0, 0,
				put_msg_ready, true, NULL);
	}

	if (!is_payload_ptr_null(r)) {
		free(get_payload_ptr(r));
	}

	return err;
}

static int process_backend_messages(size_t nr_msg_stored, size_t nr_msg_pending)
{
	if (nr_msg_stored == 0 || nr_msg_pending) {
		return -EBUSY;
	}

	struct ocpp_backend_message h = { 0, };
	if (m.backend->api.peek(m.backend, &h, sizeof(h)) == 0) {
		struct message *msg = new_message(h.header.id,
				h.header.type,
				h.header.role == OCPP_MSG_ROLE_CALLERROR,
				(void *)h.header.custom);
		if (!msg) {
			return -ENOMEM;
		}

		memcpy(&msg->body.data, &h, sizeof(h));
		msg->stored = true;

		ocpp_lock();
		put_msg_ready(msg);
		ocpp_unlock();

		return 0;
	}

	return -ENOENT;
}

static bool
on_backend_foreach(const struct ocpp_backend_message *msg, void *ctx)
{
	struct backend_foreach_ctx *p = (struct backend_foreach_ctx *)ctx;
	return p->user_cb((const struct ocpp_message *)msg, p->user_ctx);
}

ocpp_message_t ocpp_get_type_from_idstr(const char *idstr)
{
	const struct message *req = NULL;

	ocpp_lock();
	{
		req = find_msg_by_idstr(&m.tx.wait, idstr);
	}
	ocpp_unlock();

	if (req == NULL) {
		return OCPP_MSG_MAX;
	}

	return req->body.data.header.type;
}

size_t ocpp_count_pending_requests(void)
{
	size_t count = 0;

	ocpp_lock();
	{
		count += (size_t)count_messages_ready();
		count += (size_t)count_messages_waiting();
		count += (size_t)count_messages_ticking();
	}
	ocpp_unlock();

	return count;
}

size_t ocpp_count_stored_requests(void)
{
	return count_backend_messages();
}

void ocpp_iterate_pending_requests(ocpp_iterate_cb_t cb, void *ctx)
{
	ocpp_lock();
	{
		struct list *p;

		list_for_each(p, &m.tx.ready) {
			const struct message *msg =
				container_of(p, struct message, link);
			(*cb)(&msg->body, ctx);
		}

		list_for_each(p, &m.tx.wait) {
			struct message *msg =
				container_of(p, struct message, link);
			(*cb)(&msg->body, ctx);
		}

		list_for_each(p, &m.tx.timer) {
			struct message *msg =
				container_of(p, struct message, link);
			(*cb)(&msg->body, ctx);
		}
	}
	ocpp_unlock();
}

void ocpp_iterate_stored_requests(ocpp_iterate_cb_t cb, void *ctx)
{
	if (!m.backend || !m.backend->api.foreach) {
		return;
	}

	struct backend_foreach_ctx args = {
		.user_cb = cb,
		.user_ctx = ctx,
	};

	m.backend->api.foreach(m.backend, on_backend_foreach, &args);
}

int ocpp_set_message_header(struct ocpp_message *msg,
		ocpp_message_role_t role, ocpp_message_t type,
		const uint8_t *id, size_t id_size)
{
	if (!msg) {
		return -EINVAL;
	}
	if (id && id_size > sizeof(msg->data.header.id)-1) {
		return -EINVAL;
	}

	msg->data.header.role = role;
	msg->data.header.type = type;
	msg->data.header.timestamp = time(NULL);

	if (id) {
		memcpy(msg->data.header.id, id, id_size);
		msg->data.header.id[id_size] = '\0';
	}

	return 0;
}

int ocpp_copy_payload(struct ocpp_message *msg,
		const void *data, size_t datasize)
{
	if (!msg) {
		return -EINVAL;
	}
	if (!data && datasize > 0) {
		return -EINVAL;
	}
	if (!is_payload_ptr_null(msg)) {
		return -EALREADY;
	}

	uint8_t *payload = NULL;

	if (data && datasize) {
		if (!(payload = (uint8_t *)malloc(datasize))) {
			return -ENOMEM;
		}
		memcpy(payload, data, datasize);
	}

	set_payload_ptr(msg, payload, datasize);

	return 0;
}

int ocpp_read_payload(const struct ocpp_message *msg,
		void *buf, size_t bufsize)
{
	if (!msg || !buf) {
		return -EINVAL;
	}
	if (bufsize < msg->data.payload_size) {
		return -ENOSPC;
	}

	if (msg->data.payload_size == 0) {
		return 0;
	}

	const struct message *p = container_of(msg, struct message, body);

	if (!is_payload_ptr_null(msg)) {
		memcpy(buf, get_payload_ptr(msg), msg->data.payload_size);
		return (int)msg->data.payload_size;
	}

	if (p->stored) {
		return m.backend->api.peek_payload(m.backend, buf, bufsize);
	}

	return -ENOENT;
}

ocpp_message_t ocpp_get_message_type(const struct ocpp_message *msg)
{
	if (msg) {
		return msg->data.header.type;
	}
	return OCPP_MSG_MAX;
}

ocpp_message_role_t ocpp_get_message_role(const struct ocpp_message *msg)
{
	if (msg) {
		return msg->data.header.role;
	}
	return OCPP_MSG_ROLE_NONE;
}

const char *ocpp_get_message_id(const struct ocpp_message *msg)
{
	if (msg) {
		return msg->data.header.id;
	}
	return NULL;
}

size_t ocpp_get_message_payload_size(const struct ocpp_message *msg)
{
	if (msg) {
		return (size_t)msg->data.payload_size;
	}
	return 0;
}

void *ocpp_get_message_user_ctx(const struct ocpp_message *msg)
{
	if (msg) {
		return (void *)(uintptr_t)msg->data.header.custom;
	}
	return NULL;
}

struct ocpp_message *
ocpp_get_message_by_id(const char id[OCPP_MESSAGE_ID_MAXLEN])
{
	struct ocpp_message *msg = NULL;

	ocpp_lock();
	{
		struct message *p;

		if ((p = find_msg_by_idstr(&m.tx.wait, id)) ||
			(p = find_msg_by_idstr(&m.tx.ready, id)) ||
			(p = find_msg_by_idstr(&m.tx.timer, id)) ||
			(p = find_msg_by_idstr(&m.tx.dead, id))) {
			msg = &p->body;
		}
	}
	ocpp_unlock();

	return msg;
}

int ocpp_push_request(ocpp_message_t type,
		const void *data, size_t datasize, void *ctx)
{
	return push_message_backend(NULL, type, data, datasize,
			false, false, ctx);
}

int ocpp_push_request_front(ocpp_message_t type,
		const void *data, size_t datasize, void *ctx)
{
	int err;

	ocpp_lock();
	{
		err = push_message(NULL, type, data, datasize,
				0, put_msg_ready_infront, false, ctx);
	}
	ocpp_unlock();

	return err;
}

int ocpp_push_request_defer(ocpp_message_t type, const void *data,
		size_t datasize, uint32_t timer_sec, void *ctx)
{
	list_add_func_t f = put_msg_timer;
	int err = 0;

	if (timer_sec == 0) {
		f = put_msg_ready;
	}

	ocpp_lock();
	{
		err = push_message(NULL, type, data, datasize,
				time(NULL) + (time_t)timer_sec, f, 0, ctx);
	}
	ocpp_unlock();

	return err;
}

int ocpp_push_response(const struct ocpp_message *req,
		const void *data, size_t datasize, bool callerr, void *ctx)
{
	int err = 0;

	ocpp_lock();
	{
		err = push_message(req->data.header.id, req->data.header.type,
				data, datasize, 0, put_msg_ready, callerr, ctx);
	}
	ocpp_unlock();

	return err;
}

int ocpp_step(void)
{
	const time_t now = time(NULL);
	const size_t nr_msg_stored = count_backend_messages();
	size_t nr_msg_pending;

	ocpp_lock();
	{
		nr_msg_pending = (size_t)count_messages_ready() +
			(size_t)count_messages_waiting();
	}
	ocpp_unlock();

	process_backend_messages(nr_msg_stored, nr_msg_pending);

	ocpp_lock();
	{
		process_queued_messages(&now);
		process_incoming_messages(&now);
		process_periodic_messages(&now, nr_msg_stored);
		process_timer_messages(&now);
	}
	ocpp_unlock();

	return 0;
}

int ocpp_init(struct ocpp_backend *backend,
		ocpp_event_callback_t cb, void *cb_ctx)
{
	if (!backend || !backend->api.push || !backend->api.push_front ||
			!backend->api.pop || !backend->api.peek ||
			!backend->api.peek_payload || !backend->api.drop ||
			!backend->api.clear) {
		return -EINVAL;
	}

	const time_t now = time(NULL);

	memset(&m, 0, sizeof(m));

	list_init(&m.tx.ready);
	list_init(&m.tx.wait);
	list_init(&m.tx.timer);
	list_init(&m.tx.dead);

	m.backend = backend;
	m.event_callback = cb;
	m.event_callback_ctx = cb_ctx;

	update_last_tx_timestamp(&now);
	update_last_rx_timestamp(&now);

	ocpp_reset_configuration();

	return 0;
}
