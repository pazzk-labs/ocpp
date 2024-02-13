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

typedef enum {
	CT_INTEGER,
	CT_BOOL,
	CT_CSL,
	CT_STRING,
} configuration_value_t;

typedef enum {
#define OCPP_CONFIG(key, accessbility, type, default_value)	key,
#include OCPP_CONFIGURATION_DEFINES
#undef OCPP_CONFIG
	CONFIGURATION_MAX,
	UnknownConfiguration,
} configuration_t;

#define CSL int
#define STR uint8_t
#define OCPP_CONFIG(key, accessbility, type, default_value)	+ sizeof(type)
static uint8_t configurations_pool[0
#include OCPP_CONFIGURATION_DEFINES
];
#undef OCPP_CONFIG
#undef STR
#undef CSL

static struct ocpp_configuration {
	uint8_t *value;
} configurations[CONFIGURATION_MAX];

static const char * const confstr[] = {
#define OCPP_CONFIG(key, accessbility, type, default_value)	[key] = #key,
#include OCPP_CONFIGURATION_DEFINES
#undef OCPP_CONFIG
};
_Static_assert(sizeof(confstr) / sizeof(confstr[0]) == CONFIGURATION_MAX, "");

static size_t get_value_cap(configuration_t key)
{
	const uint8_t size[CONFIGURATION_MAX] = {
#define CSL int
#define STR uint8_t
#define OCPP_CONFIG(key, accessbility, type, default_value)	\
	[key] = sizeof(type),
#include OCPP_CONFIGURATION_DEFINES
#undef OCPP_CONFIG
#undef STR
#undef CSL
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
		bool v_bool;
		int v_int;
		int v_CSL;
		char *v_STR;
	} value;

	const char *cmpstr = "STR";

#define CSL int
#define STR uint8_t
#define OCPP_CONFIG(key, accessbility, type, default_value)	\
	if (strncmp(#type, cmpstr, strlen(cmpstr)) == 0) { \
		const char *s = (const char *)default_value; \
		if (s) { \
			strncpy((char *)configurations[key].value, \
					s, sizeof(type)); \
		} \
	} else { \
		value.v_ ## type = default_value; \
		memcpy(configurations[key].value, \
				&value.v_ ## type, sizeof(type)); \
	}
#include OCPP_CONFIGURATION_DEFINES
#undef OCPP_CONFIG
#undef STR
#undef CSL
}

static bool is_writable(configuration_t key)
{
	switch (key) {
#define R					false
#define RW					true
#define OCPP_CONFIG(key, accessbility, type, value)	\
	case key: return accessbility;
#include OCPP_CONFIGURATION_DEFINES
#undef OCPP_CONFIG
#undef RW
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
	if (data == NULL || datasize != sizeof(configurations_pool)) {
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

int ocpp_get_configuration(const char * const keystr,
		void *buf, size_t bufsize, bool *readonly)
{
	configuration_t key = get_key_from_keystr(keystr);

	if (key == UnknownConfiguration) {
		return -EINVAL;
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

bool ocpp_is_configuration_writable(const char * const keystr)
{
	configuration_t key = get_key_from_keystr(keystr);

	if (key == UnknownConfiguration) {
		return false;
	}

	return is_writable(key);
}

void ocpp_reset_configuration(void)
{
	memset(&configurations_pool, 0, sizeof(configurations_pool));

	ocpp_configuration_lock();

	link_configuration_pool();
	set_default_value();

	ocpp_configuration_unlock();
}
