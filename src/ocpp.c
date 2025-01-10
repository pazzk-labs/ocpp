/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "ocpp/ocpp.h"
#include "ocpp/list.h"

#include <string.h>
#include <errno.h>
#include <time.h>

#if !defined(OCPP_DEBUG)
#define OCPP_DEBUG(...)
#endif
#if !defined(OCPP_INFO)
#define OCPP_INFO(...)
#endif
#if !defined(OCPP_ERROR)
#define OCPP_ERROR(...)
#endif

#if !defined(OCPP_TX_POOL_LEN)
#define OCPP_TX_POOL_LEN			8
#endif
#if !defined(OCPP_DEFAULT_TX_RETRIES)
#define OCPP_DEFAULT_TX_RETRIES			3
#endif

#define container_of(ptr, type, member)		\
	((type *)(void *)((char *)(ptr) - offsetof(type, member)))

struct message {
	struct list link;
	struct ocpp_message body;
	time_t expiry;
	uint32_t attempts; /**< The number of message sending attempts. */
};

typedef void (*list_add_func_t)(struct message *);

static struct {
	ocpp_event_callback_t event_callback;
	void *event_callback_ctx;

	struct {
		struct message pool[OCPP_TX_POOL_LEN];
		struct list ready;
		struct list wait;
		struct list timer;

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
			ocpp_stringify_type(msg->body.type));
}

static void put_msg_ready(struct message *msg)
{
	add_last_to_list(msg, &m.tx.ready);
	OCPP_DEBUG("%s pushed to ready list",
			ocpp_stringify_type(msg->body.type));
}

static void put_msg_wait(struct message *msg)
{
	add_last_to_list(msg, &m.tx.wait);
	OCPP_DEBUG("%s pushed to wait list",
			ocpp_stringify_type(msg->body.type));
}

static void put_msg_timer(struct message *msg)
{
	add_last_to_list(msg, &m.tx.timer);
	OCPP_DEBUG("%s pushed to timer list",
			ocpp_stringify_type(msg->body.type));
}

static void del_msg_ready(struct message *msg)
{
	del_from_list(msg, &m.tx.ready);
	OCPP_DEBUG("%s removed from ready list",
			ocpp_stringify_type(msg->body.type));
}

static void del_msg_wait(struct message *msg)
{
	del_from_list(msg, &m.tx.wait);
	OCPP_DEBUG("%s removed from wait list",
			ocpp_stringify_type(msg->body.type));
}

static void del_msg_timer(struct message *msg)
{
	del_from_list(msg, &m.tx.timer);
	OCPP_DEBUG("%s removed from timer list",
			ocpp_stringify_type(msg->body.type));
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
	for (int i = 0; i < OCPP_TX_POOL_LEN; i++) {
		if (m.tx.pool[i].body.role != OCPP_MSG_ROLE_NONE) {
			continue;
		}

		m.tx.pool[i].body.role = OCPP_MSG_ROLE_ALLOC;

		return &m.tx.pool[i];
	}

	return NULL;
}

static void free_message(struct message *msg)
{
	dispatch_event(OCPP_EVENT_MESSAGE_FREE, &msg->body);
	memset(msg, 0, sizeof(*msg));
}

static struct message *new_message(const char *id,
		ocpp_message_t type, bool err)
{
	struct message *msg = alloc_message();

	if (msg == NULL) {
		return NULL;
	}

	msg->body.type = type;
	msg->attempts = 0;

	if (id) {
		msg->body.role = err?
			OCPP_MSG_ROLE_CALLERROR : OCPP_MSG_ROLE_CALLRESULT;
		memcpy(msg->body.id, id, sizeof(msg->body.id));
	} else {
		msg->body.role = OCPP_MSG_ROLE_CALL;
		ocpp_generate_message_id(msg->body.id, sizeof(msg->body.id));
	}

	return msg;
}

static struct message *find_msg_by_idstr(struct list *list_head,
		const char *msgid)
{
	struct list *p;

	list_for_each(p, list_head) {
		struct message *msg = container_of(p, struct message, link);
		if (memcmp(msgid, msg->body.id, strlen(msgid)) == 0) {
			return msg;
		}
	}

	return NULL;
}

static bool is_transaction_related(const struct message *msg)
{
	switch (msg->body.type) {
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
		msg->body.type != OCPP_MSG_BOOTNOTIFICATION;
}

static bool should_drop(struct message *msg)
{
	const uint32_t max_attempts = OCPP_DEFAULT_TX_RETRIES;

	if (!is_droppable(msg)) {
		return false;
	}

	if (msg->attempts < max_attempts) {
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
	const uint32_t elapsed = (uint32_t)(*now - m.tx.timestamp);

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
	} else if (msg->body.type == OCPP_MSG_BOOTNOTIFICATION ||
			msg->body.type == OCPP_MSG_HEARTBEAT) {
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
			ocpp_stringify_type(msg->body.type),
			msg->attempts, OCPP_DEFAULT_TX_RETRIES,
			(unsigned long)(msg->expiry - *now));

	if (ocpp_send(&msg->body) == 0) {
		if (msg->body.role == OCPP_MSG_ROLE_CALL) {
			put_msg_wait(msg);
			return;
		}
	} else {
		if (msg->attempts < OCPP_DEFAULT_TX_RETRIES ||
				is_transaction_related(msg) ||
				msg->body.type == OCPP_MSG_BOOTNOTIFICATION) {
			put_msg_wait(msg);
			return;
		}
	}

	free_message(msg);
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
			OCPP_INFO("Dropping message %s",
					ocpp_stringify_type(msg->body.type));
			free_message(msg);
		} else {
			OCPP_INFO("Retrying message %s",
					ocpp_stringify_type(msg->body.type));
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

static int process_periodic_messages(const time_t *now)
{
	if (should_send_heartbeat(now)) {
		struct message *msg = new_message(NULL, OCPP_MSG_HEARTBEAT, 0);

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

static void process_central_request(const struct ocpp_message *received)
{
	OCPP_INFO("rx: %s.req", ocpp_stringify_type(received->type));
}

static bool process_central_response_error(const struct ocpp_message *received,
		struct message *req, const time_t *now)
{
	if (!is_transaction_related(req)) {
		return true;
	}

	uint32_t max_attempts = OCPP_DEFAULT_TX_RETRIES;

	ocpp_get_configuration("TransactionMessageAttempts",
			&max_attempts, sizeof(max_attempts), NULL);

	if (req->attempts < max_attempts) {
		update_message_expiry(req, now);
		put_msg_wait(req);

		OCPP_INFO("%s will be sent again at %ld (%d/%d)",
				ocpp_stringify_type(req->body.type),
				req->expiry, req->attempts, max_attempts);
		return false;
	}

	return true;
}

static bool process_central_response_result(const struct ocpp_message *received,
		struct message *req, const time_t *now)
{
	if (received->type == OCPP_MSG_BOOTNOTIFICATION) {
		const struct ocpp_BootNotification_conf *p =
			(const struct ocpp_BootNotification_conf *)
			received->payload.fmt.response;

		if (p->status == OCPP_BOOT_STATUS_ACCEPTED) {
			set_boot_accepted(true);
		}
	}

	return true;
}

static int process_central_response(const struct ocpp_message *received,
		const time_t *now)
{
	struct message *req = find_msg_by_idstr(&m.tx.wait, received->id);
	bool free_req = true;

	if (req == NULL) {
		OCPP_ERROR("No matching request for response %s",
				ocpp_stringify_type(received->type));
		return -ENOLINK;
	}

	del_msg_wait(req);
	OCPP_INFO("rx: %s.conf", ocpp_stringify_type(req->body.type));

	if (received->role == OCPP_MSG_ROLE_CALLRESULT) {
		free_req = process_central_response_result(received, req, now);
	} else if (received->role == OCPP_MSG_ROLE_CALLERROR) {
		free_req = process_central_response_error(received, req, now);
	} else {
		OCPP_ERROR("Invalid message role: %d", received->role);
	}

	/* Note that tx timestamp is updated when the response of the message is
	 * received. */
	update_last_tx_timestamp(now);
	if (free_req) {
		free_message(req);
	}

	return 0;
}

static int process_incoming_messages(const time_t *now)
{
	struct ocpp_message received = { 0, };

	ocpp_unlock();
	int err = ocpp_recv(&received);
	ocpp_lock();

	if (err != 0) {
		goto out;
	}

	switch (received.role) {
	case OCPP_MSG_ROLE_CALL:
		process_central_request(&received);
		break;
	case OCPP_MSG_ROLE_CALLRESULT: /* fall through */
	case OCPP_MSG_ROLE_CALLERROR:
		err = process_central_response(&received, now);
		break;
	default:
		err = -EINVAL;
		OCPP_ERROR("Invalid message role: %d", received.role);
		break;
	}

out:
	if (err != -ENOMSG) {
		if (err >= 0) {
			update_last_rx_timestamp(now);
		}
		dispatch_event(err, &received);
	}

	return err;
}

static int push_message(const char *id, ocpp_message_t type,
		const void *data, size_t datasize,
		time_t timer, list_add_func_t f, bool err)
{
	struct message *msg = new_message(id, type, err);

	if (!msg) {
		return -ENOMEM;
	}

	msg->body.payload.fmt.request = data;
	msg->body.payload.size = datasize;
	msg->expiry = timer;
	(*f)(msg);

	return 0;
}

static int remove_oldest(void)
{
	struct list *p;
	struct list *t;

	list_for_each_safe(p, t, &m.tx.ready) {
		struct message *msg = container_of(p, struct message, link);
		if (msg->body.type != OCPP_MSG_BOOTNOTIFICATION &&
				msg->body.type != OCPP_MSG_START_TRANSACTION &&
				msg->body.type != OCPP_MSG_STOP_TRANSACTION) {
			OCPP_ERROR("Removing the oldest message: %s",
					ocpp_stringify_type(msg->body.type));
			del_msg_ready(msg);
			free_message(msg);
			return 0;
		}
	}

	return -ENOMEM;
}

static const char **get_typestr_array(void)
{
	static const char *msgstr[] = {
		[OCPP_MSG_AUTHORIZE] = "Authorize",
		[OCPP_MSG_BOOTNOTIFICATION] = "BootNotification",
		[OCPP_MSG_CHANGE_AVAILABILITY] = "ChangeAvailability",
		[OCPP_MSG_CHANGE_CONFIGURATION] = "ChangeConfiguration",
		[OCPP_MSG_CLEAR_CACHE] = "ClearCache",
		[OCPP_MSG_DATA_TRANSFER] = "DataTransfer",
		[OCPP_MSG_GET_CONFIGURATION] = "GetConfiguration",
		[OCPP_MSG_HEARTBEAT] = "Heartbeat",
		[OCPP_MSG_METER_VALUES] = "MeterValues",
		[OCPP_MSG_REMOTE_START_TRANSACTION] = "RemoteStartTransaction",
		[OCPP_MSG_REMOTE_STOP_TRANSACTION] = "RemoteStopTransaction",
		[OCPP_MSG_RESET] = "Reset",
		[OCPP_MSG_START_TRANSACTION] = "StartTransaction",
		[OCPP_MSG_STATUS_NOTIFICATION] = "StatusNotification",
		[OCPP_MSG_STOP_TRANSACTION] = "StopTransaction",
		[OCPP_MSG_UNLOCK_CONNECTOR] = "UnlockConnector",
		[OCPP_MSG_DIAGNOSTICS_NOTIFICATION] =
			"DiagnosticsStatusNotification",
		[OCPP_MSG_FIRMWARE_NOTIFICATION] = "FirmwareStatusNotification",
		[OCPP_MSG_GET_DIAGNOSTICS] = "GetDiagnostics",
		[OCPP_MSG_UPDATE_FIRMWARE] = "UpdateFirmware",
		[OCPP_MSG_GET_LOCAL_LIST_VERSION] = "GetLocalListVersion",
		[OCPP_MSG_SEND_LOCAL_LIST] = "SendLocalList",
		[OCPP_MSG_CANCEL_RESERVATION] = "CancelReservation",
		[OCPP_MSG_RESERVE_NOW] = "ReserveNow",
		[OCPP_MSG_CLEAR_CHARGING_PROFILE] = "ClearChargingProfile",
		[OCPP_MSG_GET_COMPOSITE_SCHEDULE] = "GetCompositeSchedule",
		[OCPP_MSG_SET_CHARGING_PROFILE] = "SetChargingProfile",
		[OCPP_MSG_TRIGGER_MESSAGE] = "TriggerMessage",
		[OCPP_MSG_CERTIFICATE_SIGNED] = "CertificateSigned",
		[OCPP_MSG_DELETE_CERTIFICATE] = "DeleteCertificate",
		[OCPP_MSG_EXTENDED_TRIGGER_MESSAGE] = "ExtendedTriggerMessage",
		[OCPP_MSG_GET_INSTALLED_CERTIFICATE_IDS] =
			"GetInstalledCertificateIds",
		[OCPP_MSG_GET_LOG] = "GetLog",
		[OCPP_MSG_INSTALL_CERTIFICATE] = "InstallCertificate",
		[OCPP_MSG_LOG_STATUS_NOTIFICATION] = "LogStatusNotification",
		[OCPP_MSG_SECURITY_EVENT_NOTIFICATION] =
			"SecurityEventNotification",
		[OCPP_MSG_SIGN_CERTIFICATE] = "SignCertificate",
		[OCPP_MSG_SIGNED_FIRMWARE_STATUS_NOTIFICATION] =
			"SignedFirmwareStatusNotification",
		[OCPP_MSG_SIGNED_UPDATE_FIRMWARE] = "SignedUpdateFirmware",
	};

	return msgstr;
}

const char *ocpp_stringify_type(ocpp_message_t msgtype)
{
	const char **msgstr = get_typestr_array();
	return msgtype >= OCPP_MSG_MAX? "UnknownMessage" : msgstr[msgtype];
}

ocpp_message_t ocpp_get_type_from_string(const char *typestr)
{
	const char **msgstr = get_typestr_array();

	for (ocpp_message_t i = 0; i < OCPP_MSG_MAX; i++) {
		if (strcmp(typestr, msgstr[i]) == 0) {
			return i;
		}
	}

	return OCPP_MSG_MAX;
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

	return req->body.type;
}

size_t ocpp_count_pending_requests(void)
{
	size_t count = 0;

	ocpp_lock();
	{
		count = (size_t)count_messages_ready();
		count += (size_t)count_messages_waiting();
		count += (size_t)count_messages_ticking();
	}
	ocpp_unlock();

	return count;
}

int ocpp_push_request(ocpp_message_t type, const void *data, size_t datasize,
		bool force)
{
	int rc = 0;

	ocpp_lock();
	{
		rc = push_message(NULL, type, data, datasize, 0,
				put_msg_ready, 0);

		if (rc != 0 && force) {
			remove_oldest();
			rc = push_message(NULL, type, data, datasize, 0,
					put_msg_ready, 0);
		}
	}
	ocpp_unlock();

	return rc;
}

int ocpp_push_request_defer(ocpp_message_t type,
		const void *data, size_t datasize, uint32_t timer_sec)
{
	list_add_func_t f = put_msg_timer;
	int rc = 0;

	if (timer_sec == 0) {
		f = put_msg_ready;
	}

	ocpp_lock();
	{
		rc = push_message(NULL, type, data, datasize,
				time(NULL) + (time_t)timer_sec, f, 0);
	}
	ocpp_unlock();

	return rc;
}

int ocpp_push_response(const struct ocpp_message *req,
		const void *data, size_t datasize, bool err)
{
	int rc = 0;

	ocpp_lock();
	{
		rc = push_message(req->id, req->type, data, datasize,
				0, put_msg_ready, err);
	}
	ocpp_unlock();

	return rc;
}

int ocpp_step(void)
{
	const time_t now = time(NULL);

	ocpp_lock();
	{
		process_queued_messages(&now);
		process_incoming_messages(&now);
		process_periodic_messages(&now);
		process_timer_messages(&now);
	}
	ocpp_unlock();

	return 0;
}

int ocpp_init(ocpp_event_callback_t cb, void *cb_ctx)
{
	const time_t now = time(NULL);

	memset(&m, 0, sizeof(m));

	list_init(&m.tx.ready);
	list_init(&m.tx.wait);
	list_init(&m.tx.timer);

	m.event_callback = cb;
	m.event_callback_ctx = cb_ctx;

	update_last_tx_timestamp(&now);
	update_last_rx_timestamp(&now);

	ocpp_reset_configuration();

	return 0;
}
