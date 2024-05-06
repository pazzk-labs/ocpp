#include <time.h>
#include <string.h>
#include "ocpp/ocpp.h"
#include "libmcu/fsm.h"

#if !defined(CHARGER_MAX_CONNECTOR)
#define CHARGER_MAX_CONNECTOR		1
#endif

typedef enum {
	S_READY,
	S_OCCUPIED,
	S_CHARGING,
	S_UNAVAILABLE,
} charger_state_t;

struct session {
	char user_id[OCPP_ID_TOKEN_MAXLEN];
	char parent_id[OCPP_ID_TOKEN_MAXLEN];
	uint32_t transaction_id;
};

struct meter {
	time_t time_clock_periodic_delivered;
	time_t time_sample_periodic_delivered;
};

struct connector {
	int id;
	time_t time_started;

	struct session session;
	struct meter meter;

	struct fsm fsm;
};

static struct charger {
	struct connector connectors[CHARGER_MAX_CONNECTOR];
} charger;


static bool is_plugged_in(void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	return false;
}

static bool is_plugged_out(void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	return false;
}

static bool is_rfis_tagged(void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	return false;
}

static bool is_remotely_started(void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	return false;
}

static bool is_remotely_stopped(void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	return false;
}

static bool is_hardware_error(void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	return false;
}

static bool is_hardware_recovered(void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	return false;
}

static bool is_timed_out(void *ctx)
{
	struct connector *connector = (struct connector *)ctx;

	uint32_t conn_timeout_sec;
	ocpp_get_configuration("ConnectionTimeout",
			&conn_timeout_sec, sizeof(conn_timeout_sec), 0);

	if ((now - connector->time_started) >= conn_timeout_sec) {
		return true;
	}

	return false;
}

static bool is_suspended_by_ev(void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	return false;
}

static bool is_resumed_from_suspended(void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	return false;
}

static bool is_charging(void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	return false;
}

static void turn_relay_on(struct connector *connector)
{
}

static void turn_relay_off(struct connector *connector)
{
}

static void clean_session(void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	memset(&connector->session, 0, sizeof(connector->session));
}

static void start_charging(void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	turn_relay_on(connector);
}

static void stop_charging(void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	turn_relay_off(connector);
	clean_session(ctx);
}

static void suspend_charging(void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	turn_relay_off(connector);
}

static void resume_charging(void *ctx)
{
	struct connector *connector = (struct connector *)ctx;
	turn_relay_on(connector);
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

static void do_metering(void *ctx)
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
	FSM_ITEM(S_READY, is_plugged_in,       NULL, S_OCCUPIED),
	FSM_ITEM(S_READY, is_rfid_tagged,      NULL, S_OCCUPIED),
	FSM_ITEM(S_READY, is_remotely_started, NULL, S_OCCUPIED),
	FSM_ITEM(S_READY, is_hardware_error,   NULL, S_UNAVAILABLE),

	FSM_ITEM(S_OCCUPIED, is_plugged_in, start_charging, S_CHARGING),
	FSM_ITEM(S_OCCUPIED, is_rfid_tagged, start_charging, S_CHARGING),
	FSM_ITEM(S_OCCUPIED, is_remotely_started, start_charging, S_CHARGING),
	FSM_ITEM(S_OCCUPIED, is_timed_out, clean_session, S_READY),
	FSM_ITEM(S_OCCUPIED, is_plugged_out, clean_session, S_READY),
	FSM_ITEM(S_OCCUPIED, is_hardware_error, clean_session, S_UNAVAILABLE),

	FSM_ITEM(S_CHARGING, is_rfid_tagged, stop_charging, S_OCCUPIED),
	FSM_ITEM(S_CHARGING, is_remotely_stopped, stop_charging, S_OCCUPIED),
	FSM_ITEM(S_CHARGING, is_plugged_out, stop_charging, S_READY),
	FSM_ITEM(S_CHARGING, is_suspended_by_ev, suspend_charging, S_CHARGING),
	FSM_ITEM(S_CHARGING, is_resumed_from_suspended, resume_charging, S_CHARGING),
	FSM_ITEM(S_CHARGING, is_hardware_error, stop_charging, S_UNAVAILABLE),
	FSM_ITEM(S_CHARGING, is_charging, do_metering, S_CHARGING),

	FSM_ITEM(S_UNAVAILABLE, is_hardware_recovered, NULL, S_READY),
};

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

	return 0;
}
