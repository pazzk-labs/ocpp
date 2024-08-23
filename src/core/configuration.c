/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "ocpp/core/configuration.h"
#include "ocpp/overrides.h"
#include <string.h>
#include <errno.h>

#if !defined(OCPP_LIBRARY_VERSION)
#define OCPP_LIBRARY_VERSION		0
#endif

#if !defined(OCPP_CONFIGURATION_DEFINES)
#define OCPP_CONFIGURATION_DEFINES	"ocpp_configuration.def.template"
#endif

#if !defined(MIN)
#define MIN(a, b)			(((a) > (b))? (b) : (a))
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
			strncpy((char *)configurations[key].value, v.v_STR, type); \
		} \
		break; \
	case OCPP_CONF_TYPE_INT: /* fall through */ \
	case OCPP_CONF_TYPE_CSL: /* fall through */ \
		v.v_INT = (int)(uintptr_t)(default_value); \
		memcpy(configurations[key].value, &v.v_INT, type); \
		break; \
	case OCPP_CONF_TYPE_BOOL: /* fall through */\
		v.v_BOOL = (bool)(uintptr_t)(default_value); \
		memcpy(configurations[key].value, &v.v_BOOL, type); \
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

	if (key == UnknownConfiguration) {
		return -EINVAL;
	}
	if (!is_readable(key)) {
		return -EACCES;
	}

	if (readonly) {
		*readonly = !is_writable(key);
	}

	ocpp_configuration_lock();

	memcpy(buf, configurations[key].value,
			MIN(get_value_cap(key), bufsize));

	ocpp_configuration_unlock();

	return 0;
}

int ocpp_get_configuration_by_index(int index,
		void *buf, size_t bufsize, bool *readonly)
{
	if (index >= CONFIGURATION_MAX) {
		return -EINVAL;
	}

	if (readonly) {
		*readonly = !is_writable((configuration_t)index);
	}

	ocpp_configuration_lock();

	memcpy(buf, configurations[index].value,
			MIN(get_value_cap((configuration_t)index), bufsize));

	ocpp_configuration_unlock();

	return 0;
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
