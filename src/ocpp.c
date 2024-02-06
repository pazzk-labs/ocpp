/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "ocpp/ocpp.h"

#include <string.h>
#include <errno.h>

#if !defined(OCPP_TX_POOL_LEN)
#define OCPP_TX_POOL_LEN			8
#endif
#if !defined(OCPP_DEFAULT_TX_TIMEOUT_SEC)
#define OCPP_DEFAULT_TX_TIMEOUT_SEC		5
#endif
#if !defined(OCPP_DEFAULT_TX_RETRIES)
#define OCPP_DEFAULT_TX_RETRIES			1
#endif

#define container_of(ptr, type, member)		\
	((type *)(void *)((char *)(ptr) - offsetof(type, member)))

static struct {
	struct {
		struct ocpp_message pool[OCPP_TX_POOL_LEN];
		struct list ready;
		struct list wait;

		time_t timestamp;
	} tx;

	struct {
		time_t timestamp;
	} rx;
} m;

static struct ocpp_message *alloc_message(void)
{
	for (int i = 0; i < OCPP_TX_POOL_LEN; i++) {
		if (m.tx.pool[i].role != OCPP_MSG_ROLE_NONE) {
			continue;
		}

		m.tx.pool[i].role = OCPP_MSG_ROLE_ALLOC;

		return &m.tx.pool[i];
	}

	return NULL;
}

static void free_message(struct ocpp_message *msg)
{
	if (msg->data != NULL) {
		ocpp_free_data(msg->data);
	}

	memset(msg, 0, sizeof(*msg));
}

static struct ocpp_message *find_msg_by_idstr_json(struct list *list_head,
		const char *msgid)
{
	struct list *p;

	list_for_each(p, list_head) {
		struct ocpp_message *msg =
			container_of(p, struct ocpp_message, link);
		const char *json = (const char *)msg->data;
		if (strncmp(msgid, &json[4], strlen(msgid)) == 0) {
			return msg;
		}
	}

	return NULL;
}

static struct ocpp_message *find_msg_by_type(struct list *list_head,
		ocpp_message_t type)
{
	struct list *p;

	list_for_each(p, list_head) {
		struct ocpp_message *msg
			= container_of(p, struct ocpp_message, link);
		if (msg->type == type) {
			return msg;
		}
	}

	return NULL;
}

static bool is_transaction_related(const struct ocpp_message *msg)
{
	switch (msg->type) {
	case OCPP_MSG_CORE_START_TRANSACTION: /* fall through */
	case OCPP_MSG_CORE_STOP_TRANSACTION: /* fall through */
	case OCPP_MSG_CORE_METER_VALUES: /* fall through */
		return true;
	default:
		return false;
	}
}

static bool should_drop(struct ocpp_message *msg)
{
	if (!is_transaction_related(msg) &&
			msg->type != OCPP_MSG_CORE_BOOTNOTIFICATION) {
		if (msg->attempts >= OCPP_DEFAULT_TX_RETRIES) {
			return true;
		}
		return false;
	}

	int max_attempts;
	ocpp_get_configuration("TransactionMessageAttempts",
			&max_attempts, sizeof(max_attempts), NULL);

	if (msg->attempts >= max_attempts) {
		return true;
	}

	return false;
}

static bool should_send_heartbeat(void)
{
	time_t now = time(NULL);
	uint32_t interval;

	ocpp_get_configuration("HeartbeatInterval",
			&interval, sizeof(interval), 0);

	if ((uint32_t)(now - m.tx.timestamp) < interval ||
			list_count(&m.tx.ready) > 0 ||
			list_count(&m.tx.wait) > 0) {
		return false;
	}

	return true;
}

static time_t calc_message_timeout(const struct ocpp_message *msg)
{
	uint32_t interval = OCPP_DEFAULT_TX_TIMEOUT_SEC;
	time_t now = time(NULL);

	if (is_transaction_related(msg)) {
		ocpp_get_configuration("TransactionMessageRetryInterval",
				&interval, sizeof(interval), 0);
		interval = interval * msg->attempts;
	}

	return now + interval;
}

static void send_message(struct ocpp_message *msg)
{
	msg->attempts++;
	msg->expiry = calc_message_timeout(msg);

	if (ocpp_send(msg->data, msg->datasize) == (int)msg->datasize) {
		list_del(&msg->link, &m.tx.ready);

		if (msg->role == OCPP_MSG_ROLE_CALL) {
			list_add_tail(&msg->link, &m.tx.wait);
		} else if (msg->role == OCPP_MSG_ROLE_CALLRESULT) {
			free_message(msg);
		}

		m.tx.timestamp = time(NULL);
	} else {
		list_del(&msg->link, &m.tx.ready);
		list_add_tail(&msg->link, &m.tx.wait);
	}
}

static void process_tx_timeout(void)
{
	struct list *p, *t;
	time_t now = time(NULL);

	list_for_each_safe(p, t, &m.tx.wait) {
		struct ocpp_message *msg =
			container_of(p, struct ocpp_message, link);
		if (msg->expiry > now) {
			continue;
		}

		list_del(p, &m.tx.wait);

		if (should_drop(msg)) {
			free_message(msg);
		} else {
			list_add_tail(p, &m.tx.ready);
		}
	}
}

static void process_queued_messages(void)
{
	process_tx_timeout();

	if (list_count(&m.tx.wait) > 0) {
		return; /* wait for the response to the previous message */
	}

	struct list *p, *t;

	list_for_each_safe(p, t, &m.tx.ready) {
		struct ocpp_message *msg =
			container_of(p, struct ocpp_message, link);
		send_message(msg);
		return; /* send one by one */
	}

	if (should_send_heartbeat()) {
		//ocpp_heartbeat(cp);
	}
}

static void process_periodic_messages(void)
{
}

static void process_incoming_messages(void)
{
}

int ocpp_push_message(ocpp_message_role_t role, ocpp_message_t type,
		const void *data, size_t datasize)
{
	if (data == NULL) {
		return -EINVAL;
	}

	ocpp_lock();

	struct ocpp_message *msg = alloc_message();
	int err;

	if (msg == NULL) {
		ocpp_unlock();
		return -ENOMEM;
	}

	msg->data = data;
	msg->datasize = datasize;
	msg->role = role;
	msg->type = type;
	msg->attempts = 0;

	list_add_tail(&msg->link, &m.tx.ready);

	ocpp_unlock();

	return 0;
}

int ocpp_step(void)
{
	ocpp_lock();

	process_queued_messages();
	process_periodic_messages();
	process_incoming_messages();

	ocpp_unlock();

	return 0;
}

int ocpp_init(ocpp_event_callback_t cb, void *cb_ctx)
{
	for (int i = 0; i < OCPP_TX_QUEUE_LEN; i++) {
		free_message(&m.tx.pool[i]);
	}

	list_init(&m.tx.ready);
	list_init(&m.tx.wait);
}
