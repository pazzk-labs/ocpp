#include <time.h>
#include "ocpp/ocpp.h"

#if !defined(CHARGER_MAX_CONNECTOR)
#define CHARGER_MAX_CONNECTOR		1
#endif

typedef enum {
	CONNECTOR_STATUS_READY,
	CONNECTOR_STATUS_OCCUPIED,
	CONNECTOR_STATUS_CHARGING,
	CONNECTOR_STATUS_UNAVAILABLE,
	CONNECTOR_STATUS_MAX,
} connector_status_t;

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

	connector_status_t status;
	connector_status_t status_previous;
	time_t time_status_changed;
};

static struct charger {
	struct connector connectors[CHARGER_MAX_CONNECTOR];
} charger;

static void change_connector_status(struct connector *connector,
		connector_status_t status)
{
	if (connector->status == status) {
		return;
	}

	connector->status_previous = connector->status;
	connector->status = status;
	connector->time_status_changed = time(NULL);
}

static void send_meter_value_periodic(const time_t base, uint32_t interval_sec,
		const time_t now)
{
	if (!interval_sec) {
		return;
	}

	const time_t next = base + interval_sec;
	if (next <= now) {
	}
}

static void do_meter(struct connector *connector, const time_t now)
{
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

static void do_session(struct connector *connector, const time_t now)
{
	uint32_t conn_timeout_sec;
	ocpp_get_configuration("ConnectionTimeout",
			&conn_timeout_sec, sizeof(conn_timeout_sec), 0);

	if ((now - connector->time_started) >= conn_timeout_sec) {
	}
}

static void process_ready(struct connector *connector, const time_t now)
{
}

static void process_occupied(struct connector *connector, const time_t now)
{
}

static void process_charging(struct connector *connector, const time_t now)
{
	do_meter(connector, now);
}

static void process_unavailable(struct connector *connector, const time_t now)
{
}

static void (*process[CONNECTOR_STATUS_MAX])(struct connector *connector,
		const time_t now) = {
	[CONNECTOR_STATUS_READY] = process_ready,
	[CONNECTOR_STATUS_OCCUPIED] = process_occupied,
	[CONNECTOR_STATUS_CHARGING] = process_charging,
	[CONNECTOR_STATUS_UNAVAILABLE] = process_unavailable,
};

int charger_step(void)
{
	const time_t now = time(NULL);

	for (int i = 0; i < CHARGER_MAX_CONNECTOR; i++) {
		struct connector *connector = &charger.connectors[i];
		(*process[connector->status])(connector, now);
	}

	return 0;
}
