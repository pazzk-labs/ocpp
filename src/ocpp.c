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

struct message {
	struct list link;
	struct ocpp_message body;
	time_t expiry;
	uint32_t attempts; /**< The number of message sending attempts. */
};

static struct {
	ocpp_event_callback_t event_callback;
	void *event_callback_ctx;

	struct {
		struct message pool[OCPP_TX_POOL_LEN];
		struct list ready;
		struct list wait;

		time_t timestamp;
	} tx;

	struct {
		time_t timestamp;
	} rx;
} m;

static void put_msg_ready(struct message *msg)
{
	list_add_tail(&msg->link, &m.tx.ready);
}

static void put_msg_wait(struct message *msg)
{
	list_add_tail(&msg->link, &m.tx.wait);
}

static void del_msg_ready(struct message *msg)
{
	list_del(&msg->link, &m.tx.ready);
}

static void del_msg_wait(struct message *msg)
{
	list_del(&msg->link, &m.tx.wait);
}

static int count_messages_waiting(void)
{
	return list_count(&m.tx.wait);
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
	memset(msg, 0, sizeof(*msg));
}

static struct message *new_message(ocpp_message_role_t role,
		ocpp_message_t type)
{
	struct message *msg = alloc_message();

	if (msg == NULL) {
		return NULL;
	}

	msg->body.role = role;
	msg->body.type = type;
	msg->attempts = 0;

	if (role != OCPP_MSG_ROLE_CALLRESULT) {
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
	case OCPP_MSG_METER_VALUES: /* fall through */
		return true;
	default:
		return false;
	}
}

static bool should_drop(struct message *msg)
{
	uint32_t max_attempts = OCPP_DEFAULT_TX_RETRIES; /* non-transactional */

	/* never drop BootNotification */
	if (msg->body.type == OCPP_MSG_BOOTNOTIFICATION) {
		return false;
	} else if (is_transaction_related(msg)) {
		ocpp_get_configuration("TransactionMessageAttempts",
				&max_attempts, sizeof(max_attempts), NULL);
	}

	if (msg->attempts >= max_attempts) {
		return true;
	}

	return false;
}

static bool should_send_heartbeat(const time_t *now)
{
	uint32_t interval;

	ocpp_get_configuration("HeartbeatInterval",
			&interval, sizeof(interval), 0);

	if ((uint32_t)(*now - m.tx.timestamp) < interval ||
			list_count(&m.tx.ready) > 0 ||
			list_count(&m.tx.wait) > 0) {
		return false;
	}

	return true;
}

static time_t calc_message_timeout(const struct message *msg, const time_t *now)
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

static void send_message(struct message *msg, const time_t *now)
{
	msg->attempts++;
	msg->expiry = calc_message_timeout(msg, now);

	del_msg_ready(msg);

	if (ocpp_send(&msg->body) == 0) {
		if (msg->body.role == OCPP_MSG_ROLE_CALL) {
			put_msg_wait(msg);
		} else if (msg->body.role == OCPP_MSG_ROLE_CALLRESULT ||
				msg->body.role == OCPP_MSG_ROLE_CALLERROR) {
			free_message(msg);
		}

		m.tx.timestamp = *now;
	} else {
		put_msg_wait(msg);
	}
}

static void process_tx_timeout(const time_t *now)
{
	struct list *p, *t;

	list_for_each_safe(p, t, &m.tx.wait) {
		struct message *msg = container_of(p, struct message, link);
		if (msg->expiry > *now) {
			continue;
		}

		del_msg_wait(msg);

		if (should_drop(msg)) {
			free_message(msg);
		} else {
			put_msg_ready(msg);
		}
	}
}

static int process_queued_messages(const time_t *now)
{
	process_tx_timeout(now);

	if (count_messages_waiting() > 0) {
		/* wait for the response to the previous message */
		return -EBUSY;
	}

	struct list *p, *t;

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
		struct message *msg =
			new_message(OCPP_MSG_ROLE_CALL, OCPP_MSG_HEARTBEAT);
		if (msg) {
			put_msg_ready(msg);
			process_queued_messages(now);
		}
	}

	return 0;
}

static void process_central_request(const struct ocpp_message *received)
{
	(void)received;
}

static void process_central_response(const struct ocpp_message *received,
		struct message *req)
{
	(void)received;
	del_msg_wait(req);
	free_message(req);
}

static int process_incoming_messages(const time_t *now)
{
	struct ocpp_message received = { 0, };
	struct message *req = NULL;
	int err = 0;

	if ((err = ocpp_recv(&received)) != 0) {
		goto out;
	}

	switch (received.role) {
	case OCPP_MSG_ROLE_CALL:
		process_central_request(&received);
		break;
	case OCPP_MSG_ROLE_CALLRESULT: /* fall through */
	case OCPP_MSG_ROLE_CALLERROR:
		if ((req = find_msg_by_idstr(&m.tx.wait, received.id)) == NULL) {
			err = -ENOLINK;
			break;
		}
		process_central_response(&received, req);
		break;
	default:
		break;
	}

out:
	if (m.event_callback && err != -ENOMSG) {
		(*m.event_callback)(err, &received, m.event_callback_ctx);
	}

	return err;
}

const char *ocpp_stringify_type(ocpp_message_t msgtype)
{
	const char *msgstr[] = {
		[OCPP_MSG_AUTHORIZE] = "Authorize",
		[OCPP_MSG_BOOTNOTIFICATION] = "BootNotification",
		[OCPP_MSG_CHANGE_AVAILABILITY] = "ChangeAvailability",
		[OCPP_MSG_CHANGE_CONFIGURATION] = "ChangeConfiguration",
		[OCPP_MSG_CLEAR_CACHE] = "ClearCache",
		[OCPP_MSG_DATA_TRANSFER] = "DataTransfer",
		[OCPP_MSG_GET_CONFIGURATION] = "GetConfiguration",
		[OCPP_MSG_HEARTBEAT] = "HeartBeat",
		[OCPP_MSG_METER_VALUES] = "MeterValues",
		[OCPP_MSG_REMOTE_START_TRANSACTION] = "RemoteStartTransaction",
		[OCPP_MSG_REMOTE_STOP_TRANSACTION] = "RemoteStopTransaction",
		[OCPP_MSG_RESET] = "Reset",
		[OCPP_MSG_START_TRANSACTION] = "StartTransaction",
		[OCPP_MSG_STATUS_NOTIFICATION] = "StatusNotification",
		[OCPP_MSG_STOP_TRANSACTION] = "StopTransaction",
		[OCPP_MSG_UNLOCK_CONNECTOR] = "UnlockConnector",
		[OCPP_MSG_DIAGNOSTICS_NOTIFICATION] = "DiagnosticsStatusNotification",
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
	};

	return msgtype >= OCPP_MSG_MAX? NULL : msgstr[msgtype];
}

int ocpp_push_message(ocpp_message_role_t role, ocpp_message_t type,
		const void *data, size_t datasize)
{
	/* HeartBeat is sent internally on itself. */
	if (role == OCPP_MSG_ROLE_CALL && type == OCPP_MSG_HEARTBEAT) {
		return -EALREADY;
	}

	ocpp_lock();

	struct message *msg = new_message(role, type);

	if (msg) {
		memcpy(&msg->body.fmt, data, datasize);
		put_msg_ready(msg);
	}

	ocpp_unlock();

	return 0;
}

int ocpp_step(void)
{
	time_t now = time(NULL);

	ocpp_lock();

	process_incoming_messages(&now);
	process_queued_messages(&now);
	process_periodic_messages(&now);

	ocpp_unlock();

	return 0;
}

int ocpp_init(ocpp_event_callback_t cb, void *cb_ctx)
{
	memset(&m, 0, sizeof(m));

	list_init(&m.tx.ready);
	list_init(&m.tx.wait);

	m.event_callback = cb;
	m.event_callback_ctx = cb_ctx;

	ocpp_reset_configuration();

	return 0;
}
