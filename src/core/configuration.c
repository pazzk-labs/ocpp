/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "ocpp/core/configuration.h"
#include "ocpp/overrides.h"
#include <string.h>
#include <errno.h>
#include <stdio.h>

#if !defined(OCPP_CONFIGURATION_DEFINES)
#define OCPP_CONFIGURATION_DEFINES	"ocpp_configuration.def.template"
#endif

#if !defined(MIN)
#define MIN(a, b)			(((a) > (b))? (b) : (a))
#endif

#if !defined(ARRAY_COUNT)
#define ARRAY_COUNT(x)			(sizeof(x) / sizeof((x)[0]))
#endif

#define CONF_SIZE(x)			(x)
#define BOOL				CONF_SIZE(sizeof(bool))
#define INT				CONF_SIZE(sizeof(int))
#define CSL				CONF_SIZE(sizeof(int))
#define STR				CONF_SIZE

typedef enum {
#define OCPP_CONFIG(key, accessbility, type, default_value)	key,
#include OCPP_CONFIGURATION_DEFINES
#undef OCPP_CONFIG
	CONFIGURATION_MAX,
	UnknownConfiguration,
} configuration_t;

#define OCPP_CONFIG(key, accessbility, type, default_value)	+ type
static uint8_t configurations_pool[0
#include OCPP_CONFIGURATION_DEFINES
];
#undef OCPP_CONFIG

static struct ocpp_configuration {
	uint8_t *value;
} configurations[CONFIGURATION_MAX];

static const char * const confstr[] = {
#define OCPP_CONFIG(key, accessbility, type, default_value)	[key] = #key,
#include OCPP_CONFIGURATION_DEFINES
#undef OCPP_CONFIG
};

static ocpp_configuration_data_t get_value_type(configuration_t key)
{
	const ocpp_configuration_data_t value_types[CONFIGURATION_MAX] = {
#define STR_TYPE(x) OCPP_CONF_TYPE_STR
#undef STR
#undef CSL
#undef INT
#undef BOOL
#define BOOL CONF_SIZE(OCPP_CONF_TYPE_BOOL)
#define INT CONF_SIZE(OCPP_CONF_TYPE_INT)
#define CSL CONF_SIZE(OCPP_CONF_TYPE_CSL)
#define STR STR_TYPE
#define OCPP_CONFIG(key, accessbility, type, default_value)	\
	[key] = type,
#include OCPP_CONFIGURATION_DEFINES
#undef OCPP_CONFIG
#undef STR
#undef CSL
#undef INT
#undef BOOL
	};
	return value_types[key];
}

static size_t get_value_cap(configuration_t key)
{
	const uint8_t size[CONFIGURATION_MAX] = {
#define BOOL CONF_SIZE(sizeof(bool))
#define INT CONF_SIZE(sizeof(int))
#define CSL CONF_SIZE(sizeof(int))
#define STR CONF_SIZE
#define OCPP_CONFIG(key, accessbility, type, default_value)	\
	[key] = type,
#include OCPP_CONFIGURATION_DEFINES
#undef OCPP_CONFIG
#undef STR
#undef CSL
#undef INT
#undef BOOL
	};
	return (size_t)size[key];
}

static void link_configuration_pool(void)
{
	size_t cidx = 0;

	for (configuration_t i = 0; i < CONFIGURATION_MAX; i++) {
		configurations[i].value = &configurations_pool[cidx];
		cidx += get_value_cap(i);
	}
}

static void set_default_value(void)
{
	union {
		bool v_BOOL;
		int v_INT;
		int v_CSL;
		const char *v_STR;
	} v;

#define BOOL CONF_SIZE(sizeof(bool))
#define INT CONF_SIZE(sizeof(int))
#define CSL CONF_SIZE(sizeof(int))
#define STR CONF_SIZE
#define OCPP_CONFIG(key, accessbility, type, default_value)	\
	switch (get_value_type(key)) { \
	case OCPP_CONF_TYPE_STR: \
		v.v_STR = (const char *)(default_value); \
		if (v.v_STR) { \
			strncpy((char *)configurations[key].value, \
					v.v_STR, type); \
		} \
		break; \
	case OCPP_CONF_TYPE_INT: /* fall through */ \
	case OCPP_CONF_TYPE_CSL: /* fall through */ \
		v.v_INT = (int)(uintptr_t)(default_value); \
		memcpy(configurations[key].value, &v.v_INT, \
				MIN(type, sizeof(v.v_INT))); \
		break; \
	case OCPP_CONF_TYPE_BOOL: /* fall through */\
		v.v_BOOL = (bool)(uintptr_t)(default_value); \
		memcpy(configurations[key].value, &v.v_BOOL, \
				MIN(type, sizeof(v.v_BOOL))); \
		break; \
	default: /*fall through */ \
	case OCPP_CONF_TYPE_UNKNOWN: \
		break; \
	}
#include OCPP_CONFIGURATION_DEFINES
#undef OCPP_CONFIG
#undef STR
#undef CSL
#undef INT
#undef BOOL
}

static bool is_readable(configuration_t key)
{
	switch (key) {
#define R					true
#define W					false
#define RW					true
#define OCPP_CONFIG(key, accessbility, type, value)	\
	case key: return accessbility;
#include OCPP_CONFIGURATION_DEFINES
#undef OCPP_CONFIG
#undef RW
#undef W
#undef R
	case CONFIGURATION_MAX: /* fall through */
	case UnknownConfiguration: /* fall through */
	default:
		return false;
	}
}

static bool is_writable(configuration_t key)
{
	switch (key) {
#define R					false
#define W					true
#define RW					true
#define OCPP_CONFIG(key, accessbility, type, value)	\
	case key: return accessbility;
#include OCPP_CONFIGURATION_DEFINES
#undef OCPP_CONFIG
#undef RW
#undef W
#undef R
	case CONFIGURATION_MAX: /* fall through */
	case UnknownConfiguration: /* fall through */
	default:
		return false;
	}
}

static configuration_t get_key_from_keystr(const char * const keystr)
{
	for (int i = 0; i < CONFIGURATION_MAX; i++) {
		if (strcmp(keystr, confstr[i]) == 0) {
			return (configuration_t)i;
		}
	}

	return UnknownConfiguration;
}

static int get_configuration(configuration_t key,
		void *buf, size_t bufsize, bool *readonly)
{
	if (key >= CONFIGURATION_MAX) {
		return -EINVAL;
	}

	if (readonly) {
		*readonly = !is_writable(key) && is_readable(key);
	}

	memcpy(buf, configurations[key].value,
			MIN(get_value_cap(key), bufsize));

	return 0;
}

static size_t addstr_if(const int mask, const int value, const char *str,
		char *buf, const size_t bufsize, bool comma)
{
	int len = 0;

	if (mask & value) {
		len = snprintf(buf, bufsize, "%s%s", comma? "," : "", str);
	}

	return len > 0? (size_t)len : 0;
}

static void stringify_connector_phase_rotation(const int value,
		char *buf, size_t bufsize)
{
	if (value < 0 || value > OCPP_PHASE_ROTATION_TSR) {
		return;
	}

	const char *str[] = {
		[OCPP_PHASE_ROTATION_NOT_APPLICABLE] = "NotApplicable",
		[OCPP_PHASE_ROTATION_UNKNOWN] = "Unknown",
		[OCPP_PHASE_ROTATION_RST] = "RST",
		[OCPP_PHASE_ROTATION_RTS] = "RTS",
		[OCPP_PHASE_ROTATION_SRT] = "SRT",
		[OCPP_PHASE_ROTATION_STR] = "STR",
		[OCPP_PHASE_ROTATION_TRS] = "TRS",
		[OCPP_PHASE_ROTATION_TSR] = "TSR",
	};

	snprintf(buf, bufsize, "%s", str[value]);
}


static void stringify_meter_data(const int value, char *buf, size_t bufsize)
{
	size_t len = addstr_if(value, OCPP_MEASURAND_CURRENT_EXPORT,
			"Current.Export", buf, bufsize, false);
	len += addstr_if(value, OCPP_MEASURAND_CURRENT_IMPORT,
			"Current.Import", &buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_MEASURAND_CURRENT_OFFERED,
			"Current.Offered", &buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_MEASURAND_ENERGY_ACTIVE_EXPORT_REGISTER,
			"Energy.Active.Export.Register",
			&buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_MEASURAND_ENERGY_ACTIVE_IMPORT_REGISTER,
			"Energy.Active.Import.Register",
			&buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_MEASURAND_ENERGY_REACTIVE_EXPORT_REGISTER,
			"Energy.Reactive.Export.Register",
			&buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_MEASURAND_ENERGY_REACTIVE_IMPORT_REGISTER,
			"Energy.Reactive.Import.Register",
			&buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_MEASURAND_ENERGY_ACTIVE_EXPORT_INTERVAL,
			"Energy.Active.Export.Interval",
			&buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_MEASURAND_ENERGY_ACTIVE_IMPORT_INTERVAL,
			"Energy.Active.Import.Interval",
			&buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_MEASURAND_ENERGY_REACTIVE_EXPORT_INTERVAL,
			"Energy.Reactive.Export.Interval",
			&buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_MEASURAND_ENERGY_REACTIVE_IMPORT_INTERVAL,
			"Energy.Reactive.Import.Interval",
			&buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_MEASURAND_FREQUENCY,
			"Frequency", &buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_MEASURAND_POWER_ACTIVE_EXPORT,
			"Power.Active.Export", &buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_MEASURAND_POWER_ACTIVE_IMPORT,
			"Power.Active.Import", &buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_MEASURAND_POWER_FACTOR,
			"Power.Factor", &buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_MEASURAND_POWER_OFFERED,
			"Power.Offered", &buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_MEASURAND_POWER_REACTIVE_EXPORT,
			"Power.Reactive.Export",
			&buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_MEASURAND_POWER_REACTIVE_IMPORT,
			"Power.Reactive.Import",
			&buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_MEASURAND_RPM,
			"RPM", &buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_MEASURAND_SOC,
			"SoC", &buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_MEASURAND_TEMPERATURE,
			"Temperature", &buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_MEASURAND_VOLTAGE,
			"Voltage", &buf[len], bufsize - len, !!len);
}

static void stringify_profile(const int value, char *buf, size_t bufsize)
{
	size_t len = addstr_if(value, OCPP_PROFILE_CORE,
			"Core", buf, bufsize, false);
	len += addstr_if(value, OCPP_PROFILE_FW_MGMT,
			"FirmwareManagement", &buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_PROFILE_LOCAL_AUTH,
			"LocalAuthListManagement",
			&buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_PROFILE_RESERVATION,
			"Reservation", &buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_PROFILE_SMART_CHARGING,
			"SmartCharging", &buf[len], bufsize - len, !!len);
	len += addstr_if(value, OCPP_PROFILE_REMOTE_TRIGGER,
			"RemoteTrigger", &buf[len], bufsize - len, !!len);
}

static void stringify_charging_rate_unit(const int value,
		char *buf, size_t bufsize)
{
	size_t len = addstr_if(value, OCPP_CHARGING_UNIT_WATT,
			"Power", buf, bufsize, false);
	len += addstr_if(value, OCPP_CHARGING_UNIT_AMPERE,
			"Current", &buf[len], bufsize - len, !!len);
}

static void stringify_csl(const configuration_t key,
		int value, char *buf, size_t bufsize)
{
	const struct {
		const configuration_t key;
		void (*fn)(const int value, char *buf, size_t bufsize);
	} csl_handler[] = {
		{ ConnectorPhaseRotation, stringify_connector_phase_rotation },
		{ MeterValuesAlignedData, stringify_meter_data },
		{ MeterValuesSampledData, stringify_meter_data },
		{ StopTxnAlignedData, stringify_meter_data },
		{ StopTxnSampledData, stringify_meter_data },
		{ SupportedFeatureProfiles, stringify_profile },
		{ ChargingScheduleAllowedChargingRateUnit,
			stringify_charging_rate_unit },
	};

	for (size_t i = 0; i < ARRAY_COUNT(csl_handler); i++) {
		if (key == csl_handler[i].key) {
			csl_handler[i].fn(value, buf, bufsize);
			break;
		}
	}
}

const char *ocpp_stringify_configuration_value(const char *keystr,
		char *buf, size_t bufsize)
{
	const configuration_t key = get_key_from_keystr(keystr);

	if (key >= CONFIGURATION_MAX || !buf || bufsize == 0) {
		return NULL;
	}

	const ocpp_configuration_data_t value_type = get_value_type(key);
	const size_t value_size = get_value_cap(key);
	uint8_t value[value_size];
	bool readonly;
	int ival;

	buf[0] = '\0';

	ocpp_configuration_lock();
	get_configuration(key, value, value_size, &readonly);
	ocpp_configuration_unlock();

	switch (value_type) {
	case OCPP_CONF_TYPE_STR:
		strncpy(buf, (const char *)value, bufsize);
		break;
	case OCPP_CONF_TYPE_BOOL:
		snprintf(buf, bufsize, "%s",
				*(const bool *)value? "true" : "false");
		break;
	case OCPP_CONF_TYPE_INT:
		memcpy(&ival, value, sizeof(ival));
		snprintf(buf, bufsize, "%d", ival);
		break;
	case OCPP_CONF_TYPE_CSL:
		memcpy(&ival, value, sizeof(ival));
		stringify_csl(key, ival, buf, bufsize);
		break;
	default:
		return NULL;
	}

	return buf;
}

bool ocpp_has_configuration(const char * const keystr)
{
	return get_key_from_keystr(keystr) != UnknownConfiguration;
}

size_t ocpp_count_configurations(void)
{
	return CONFIGURATION_MAX;
}

size_t ocpp_compute_configuration_size(void)
{
	return sizeof(configurations_pool);
}

int ocpp_copy_configuration_from(const void *data, size_t datasize)
{
	if (data == NULL || datasize > sizeof(configurations_pool)) {
		return -EINVAL;
	}

	ocpp_configuration_lock();

	memcpy(configurations_pool, data, datasize);
	link_configuration_pool();

	ocpp_configuration_unlock();

	return 0;
}

int ocpp_copy_configuration_to(void *buf, size_t bufsize)
{
	if (buf == NULL || bufsize < sizeof(configurations_pool)) {
		return -EINVAL;
	}

	ocpp_configuration_lock();
	memcpy(buf, configurations_pool, sizeof(configurations_pool));
	ocpp_configuration_unlock();

	return 0;
}

int ocpp_set_configuration(const char * const keystr,
		const void *value, size_t value_size)
{
	configuration_t key = get_key_from_keystr(keystr);

	if (key == UnknownConfiguration || value_size > get_value_cap(key)) {
		return -EINVAL;
	}

	if (!is_writable(key)) {
		return -EPERM;
	}

	ocpp_configuration_lock();
	memcpy(configurations[key].value, value, value_size);
	ocpp_configuration_unlock();

	return 0;
}

size_t ocpp_get_configuration_size(const char * const keystr)
{
	configuration_t key = get_key_from_keystr(keystr);

	if (key == UnknownConfiguration) {
		return 0;
	}

	return get_value_cap(key);
}

ocpp_configuration_data_t ocpp_get_configuration_data_type(
		const char * const keystr)
{
	configuration_t key = get_key_from_keystr(keystr);

	if (key == UnknownConfiguration) {
		return OCPP_CONF_TYPE_UNKNOWN;
	}

	return get_value_type(key);
}

int ocpp_get_configuration(const char * const keystr,
		void *buf, size_t bufsize, bool *readonly)
{
	configuration_t key = get_key_from_keystr(keystr);

	ocpp_configuration_lock();
	int rc = get_configuration(key, buf, bufsize, readonly);
	ocpp_configuration_unlock();

	return rc;
}

int ocpp_get_configuration_by_index(int index,
		void *buf, size_t bufsize, bool *readonly)
{
	ocpp_configuration_lock();
	int rc = get_configuration((configuration_t)index,
			buf, bufsize, readonly);
	ocpp_configuration_unlock();

	return rc;
}

const char *ocpp_get_configuration_keystr_from_index(int index)
{
	if (index < 0 || index >= CONFIGURATION_MAX) {
		return NULL;
	}

	return confstr[index];
}

bool ocpp_is_configuration_writable(const char * const keystr)
{
	configuration_t key = get_key_from_keystr(keystr);

	if (key == UnknownConfiguration) {
		return false;
	}

	return is_writable(key);
}

bool ocpp_is_configuration_readable(const char * const keystr)
{
	configuration_t key = get_key_from_keystr(keystr);

	if (key == UnknownConfiguration) {
		return false;
	}

	return is_readable(key);
}

void ocpp_reset_configuration(void)
{
	memset(&configurations_pool, 0, sizeof(configurations_pool));

	ocpp_configuration_lock();

	link_configuration_pool();
	set_default_value();

	ocpp_configuration_unlock();
}
