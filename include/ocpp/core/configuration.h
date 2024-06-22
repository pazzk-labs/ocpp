/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_OCPP_CONFIGURATION_H
#define LIBMCU_OCPP_CONFIGURATION_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "ocpp/type.h"

typedef enum {
	OCPP_CONF_TYPE_UNKNOWN,
	OCPP_CONF_TYPE_INT,
	OCPP_CONF_TYPE_CSL,
	OCPP_CONF_TYPE_STR,
	OCPP_CONF_TYPE_BOOL,
} ocpp_configuration_data_t;

bool ocpp_has_configuration(const char * const keystr);
/**
 * @brief Count the number of configurations.
 *
 * @return the number of configurations.
 */
size_t ocpp_count_configurations(void);
/**
 * @brief Compute the total configuration size.
 *
 * @return the total configuration size.
 */
size_t ocpp_compute_configuration_size(void);
int ocpp_copy_configuration_from(const void *data, size_t datasize);
int ocpp_copy_configuration_to(void *buf, size_t bufsize);
void ocpp_reset_configuration(void);
int ocpp_set_configuration(const char * const keystr,
		const void *value, size_t value_size);
/**
 * @brief Get the configuration for the key string.
 *
 * @param[in] keystr key string
 * @param[in] buf buffer
 * @param[in] bufsize size of buffer
 * @param[out] readonly true if readonly. null if not needed
 *
 * @return 0 for success, otherwise an error.
 */
int ocpp_get_configuration(const char * const keystr,
		void *buf, size_t bufsize, bool *readonly);
int ocpp_get_configuration_by_index(int index,
		void *buf, size_t bufsize, bool *readonly);
bool ocpp_is_configuration_writable(const char * const keystr);
bool ocpp_is_configuration_readable(const char * const keystr);
size_t ocpp_get_configuration_size(const char * const keystr);
ocpp_configuration_data_t ocpp_get_configuration_data_type(
		const char * const keystr);
const char *ocpp_get_configuration_keystr_from_index(int index);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_OCPP_CONFIGURATION_H */
