#include <time.h>
#include <string.h>
#include "ocpp/ocpp.h"
#include "libmcu/fsm.h"

#if !defined(CHARGER_MAX_CONNECTOR)
#define CHARGER_MAX_CONNECTOR		1
#endif

#define RFID_FRESH_TIMEOUT_MS		1000

typedef enum {
	S_READY,
	S_OCCUPIED,
	S_CHARGING,
	S_UNAVAILABLE,
} charger_state_t;

struct session {
	uint8_t user_id[OCPP_ID_TOKEN_MAXLEN];
	uint8_t parent_id[OCPP_ID_TOKEN_MAXLEN];
	uint8_t tmp_id[OCPP_ID_TOKEN_MAXLEN];
	uint32_t transaction_id;
	bool remotely_started;
};

struct meter {
	time_t time_clock_periodic_delivered;
	time_t time_sample_periodic_delivered;
};

struct connector {
	int id;
	time_t time_occupied;

	struct session session;
	struct meter meter;

	struct fsm fsm;

	cp_status_t status;
};

static struct charger {
	struct connector connectors[CHARGER_MAX_CONNECTOR];
} charger;

static bool got_plugged_in(fsm_state_t state, fsm_state_t next_state, void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	cp_status_t status = cp_get_status(connector->id);

	if (status != connector->status) {
		return status == CP_STATUS_B;
	}

	return false;
}

static bool got_plugged_out(fsm_state_t state, fsm_state_t next_state,
		void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	cp_status_t status = cp_get_status(connector->id);

	if (status != connector->status) {
		return status == CP_STATUS_A;
	}

	return false;
}

static bool is_rfid_tagged(fsm_state_t state, fsm_state_t next_state,
		void *ctx)
{
	struct connector *connector = (struct connector *)ctx;

	if (state == S_OCCUPIED && connector->status == CP_STATUS_A) {
                /* if not plugged yet but occupied, it means already remotely
                 * started or rfid tagged. */
		return false;
	} else if (state == S_CHARGING) {
		uint8_t uid[OCPP_ID_TOKEN_MAXLEN];
		if (rfid_get_tagged_uid(uid, sizeof(uid),
				RFID_FRESH_TIMEOUT_MS)) {
			if (memcmp(uid, connector->session.user_id, sizeof(uid)) == 0 ||
					memcmp(uid, connector->session.parent_id, sizeof(uid)) == 0) {
				return true;
			}
		}
		return false;
	}

	return rfid_get_tagged_uid(NULL, 0, RFID_FRESH_TIMEOUT_MS);
}

static bool got_remotely_started(fsm_state_t state, fsm_state_t next_state,
		void *ctx)
{
	struct connector *connector = (struct connector *)ctx;

	if (state == S_OCCUPIED && connector->status == CP_STATUS_A) {
                /* if not plugged yet but occupied, it means already remotely
                 * started or rfid tagged. */
                return false;
	} else if (state >= S_CHARGING) {
		return false;
	} else if (connector->session.remotely_started) {
		return true;
	}

	return false;
}

static bool is_remotely_stopped(fsm_state_t state, fsm_state_t next_state,
		void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	return false;
}

static bool is_hardware_error(fsm_state_t state, fsm_state_t next_state,
		void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	return false;
}

static bool is_hardware_recovered(fsm_state_t state, fsm_state_t next_state,
		void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	return false;
}

static bool is_connection_timed_out(fsm_state_t state, fsm_state_t next_state,
		void *ctx)
{
	struct connector *connector = (struct connector *)ctx;

	if (state != S_OCCUPIED) {
		return false;
	}

	uint32_t conn_timeout_sec;
	ocpp_get_configuration("ConnectionTimeout",
			&conn_timeout_sec, sizeof(conn_timeout_sec), 0);

	if ((now - connector->time_occupied) >= conn_timeout_sec) {
		if (connector->status == CP_STATUS_A) {
			return true;
		}
	}

	return false;
}

static bool is_suspended_by_ev(fsm_state_t state, fsm_state_t next_state,
		void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	return false;
}

static bool is_resumed_from_suspended(fsm_state_t state, fsm_state_t next_state,
		void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	cp_status_t status = cp_get_status(connector->id);

	if (status != connector->status) {
		assert(connector->status == CP_STATUS_B);
		return status == CP_STATUS_C;
	}

	return false;
}

static bool is_charging(fsm_state_t state, fsm_state_t next_state, void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	return true;
}

static void turn_relay_on(struct connector *connector)
{
}

static void turn_relay_off(struct connector *connector)
{
}

static void clean_session(fsm_state_t state, fsm_state_t next_state,
		void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	memset(&connector->session, 0, sizeof(connector->session));

	connector->status = cp_get_status(connector->id);
}

static void prepare_to_charge(fsm_state_t state, fsm_state_t next_state,
		void *ctx)
{
	struct connector *connector = (struct connector *)ctx;

	if (state != S_READY) {
		return;
	}

	if (got_plugged_in(state, next_state, ctx)) {
		connector->status = CP_STATUS_B;
	} else if (is_rfid_tagged(state, next_state, ctx)) {
		rfid_get_tagged_uid_and_clear(connector->session.user_id,
				sizeof(connector->session.user_id),
				RFID_FRESH_TIMEOUT_MS);
	} else if (got_remotely_started(state, next_state, ctx)) {
		memcpy(connector->session.user_id, connector->session.tmp_id,
				sizeof(connector->session.tmp_id));
	} else {
		assert(0);
	}

	connector->time_occupied = time(NULL);

	struct ocpp_StatusNotification msg = {
		.connectorId = connector->id,
		.timestamp = connector->time_occupied,
		.status = OCPP_STATUS_PREPARING,
		.errorCode = OCPP_ERROR_NONE,
	};

	ocpp_push_message(OCPP_MSG_ROLE_CALL, OCPP_MSG_STATUS_NOTIFICATION,
			&msg, sizeof(msg));
}

static void start_charging(fsm_state_t state, fsm_state_t next_state,
		void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	turn_relay_on(connector);
}

static void stop_charging(fsm_state_t state, fsm_state_t next_state, void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	turn_relay_off(connector);
	clean_session(ctx);
	// TODO: send StopTransaction
}

static void suspend_charging(fsm_state_t state, fsm_state_t next_state,
		void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	turn_relay_off(connector);

	connector->status = cp_get_status(connector->id);
}

static void resume_charging(fsm_state_t state, fsm_state_t next_state,
		void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	turn_relay_on(connector);

	connector->status = cp_get_status(connector->id);
}

static void send_meter_value_periodic(const time_t base, uint32_t interval_sec,
		const time_t now)
{
	if (!interval_sec) {
		return;
	}

	const time_t next = base + interval_sec;
	if (next <= now) {
		// TODO: send MeterValue
	}
}

static void do_metering(fsm_state_t state, fsm_state_t next_state, void *ctx)
{
	struct connector *connector = (struct connector *)ctx;

	uint32_t clock_interval = 0;
	uint32_t sample_interval = 0;

	ocpp_get_configuration("ClockAlignedDataInterval",
			&clock_interval, sizeof(clock_interval), 0);
	ocpp_get_configuration("MeterValueSampleInterval",
			&sample_interval, sizeof(sample_interval), 0);

	send_meter_value_periodic(connector->meter.time_clock_periodic_delivered,
			clock_interval, now);
	send_meter_value_periodic(connector->meter.time_sample_periodic_delivered,
			sample_interval, now);
}

static const struct fsm_item transitions[] = {
	FSM_ITEM(S_READY, got_plugged_in, prepare_to_charge, S_OCCUPIED),
	FSM_ITEM(S_READY, is_rfid_tagged, prepare_to_charge, S_OCCUPIED),
	FSM_ITEM(S_READY, got_remotely_started, prepare_to_charge, S_OCCUPIED),
	FSM_ITEM(S_READY, is_hardware_error, NULL, S_UNAVAILABLE),

	FSM_ITEM(S_OCCUPIED, got_plugged_in, start_charging, S_CHARGING),
	FSM_ITEM(S_OCCUPIED, is_rfid_tagged, start_charging, S_CHARGING),
	FSM_ITEM(S_OCCUPIED, got_remotely_started, start_charging, S_CHARGING),
	FSM_ITEM(S_OCCUPIED, is_connection_timed_out, clean_session, S_READY),
	FSM_ITEM(S_OCCUPIED, got_plugged_out, clean_session, S_READY),
	FSM_ITEM(S_OCCUPIED, is_hardware_error, clean_session, S_UNAVAILABLE),

	FSM_ITEM(S_CHARGING, is_rfid_tagged, stop_charging, S_OCCUPIED),
	FSM_ITEM(S_CHARGING, is_remotely_stopped, stop_charging, S_OCCUPIED),
	FSM_ITEM(S_CHARGING, got_plugged_out, stop_charging, S_READY),
	FSM_ITEM(S_CHARGING, is_suspended_by_ev, suspend_charging, S_CHARGING),
	FSM_ITEM(S_CHARGING, is_resumed_from_suspended, resume_charging, S_CHARGING),
	FSM_ITEM(S_CHARGING, is_hardware_error, stop_charging, S_UNAVAILABLE),
	FSM_ITEM(S_CHARGING, is_charging, do_metering, S_CHARGING),

	FSM_ITEM(S_UNAVAILABLE, is_hardware_recovered, NULL, S_READY),
};

static void on_ocpp_event(int err, const struct ocpp_message *message,
		void *ctx)
{
	struct charger *p = (struct charger *)ctx;
	struct connector *connector = NULL;

	if (err) {
		return;
	}

	if (message->role == OCPP_MSG_ROLE_CALLRESULT) {
		return;
	}

	switch (message->type) {
	case OCPP_MSG_REMOTE_START_TRANSACTION:
		assert(message->fmt.req.RemoteStartTransaction.connectorId <= CHARGER_MAX_CONNECTOR);
		connector = &p->connectors[message->fmt.req.RemoteStartTransaction.connectorId];

		if (!connector->session.remotely_started) {
			memcpy(connector->session.tmp_id, message->fmt.req.RemoteStartTransaction.idTag,
					sizeof(message->fmt.req.RemoteStartTransaction.idTag));
			connector->session.remotely_started = true;
			if (got_remotely_started(fsm_state(&connector->fsm), 0, connector)) {
				// TODO: accepted
				break;
			}
		}
		// TODO: rejected
		break;
	case OCPP_MSG_REMOTE_STOP_TRANSACTION:
	default:
		break;
	}
}

int charger_step(void)
{
	for (int i = 0; i < CHARGER_MAX_CONNECTOR; i++) {
		struct connector *connector = &charger.connectors[i];
		fsm_step(&connector->fsm);
	}

	return 0;
}

int charger_init(void)
{
	for (int i = 0; i < CHARGER_MAX_CONNECTOR; i++) {
		struct connector *connector = &charger.connectors[i];
		fsm_init(&fsm, transitions,
				sizeof(transitions) / sizeof(*transitions),
				connector);
	}

	return ocpp_init(on_ocpp_event, &charger);
}
