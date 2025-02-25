/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
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

/**
 * @brief Checks if a configuration key exists.
 *
 * This function takes a configuration key as input and checks if the key
 * exists in the configuration.
 *
 * @param[in] keystr The configuration key to be checked.
 *
 * @return true if the configuration key exists, false otherwise.
 */
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

/**
 * @brief Copies configuration data from a source buffer.
 *
 * This function copies configuration data from the provided source buffer
 * into the internal configuration storage.
 *
 * @param[in] data The source buffer containing the configuration data.
 * @param[in] datasize The size of the source buffer.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int ocpp_copy_configuration_from(const void *data, size_t datasize);

/**
 * @brief Copies configuration data to a destination buffer.
 *
 * This function copies the internal configuration data to the provided
 * destination buffer.
 *
 * @param[out] buf The destination buffer where the configuration data will be
 *             copied.
 * @param[in] bufsize The size of the destination buffer.
 *
 * @return The number of bytes copied on success, or a negative error code on
 *         failure.
 */
int ocpp_copy_configuration_to(void *buf, size_t bufsize);

/**
 * @brief Resets the configuration to its default values.
 *
 * This function resets all configuration settings to their default values.
 */
void ocpp_reset_configuration(void);

/**
 * @brief Sets a configuration value for a given key.
 *
 * This function sets the configuration value for the specified key.
 *
 * @param[in] keystr The configuration key to be set.
 * @param[in] value The value to be set for the specified key.
 * @param[in] value_size The size of the value.
 *
 * @return 0 on success, or a negative error code on failure.
 */
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

/**
 * @brief Retrieves the configuration value by index.
 *
 * This function retrieves the configuration value at the specified index.
 * The result is stored in the provided buffer.
 *
 * @param[in] index The index of the configuration value to retrieve.
 * @param[out] buf The buffer where the configuration value will be stored.
 * @param[in] bufsize The size of the buffer.
 * @param[out] readonly A pointer to a boolean that will be set to true if the
 *             configuration is read-only.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int ocpp_get_configuration_by_index(int index,
		void *buf, size_t bufsize, bool *readonly);

/**
 * @brief Checks if a configuration key is writable.
 *
 * This function checks if the specified configuration key is writable.
 *
 * @param[in] keystr The configuration key to check.
 *
 * @return true if the configuration key is writable, false otherwise.
 */
bool ocpp_is_configuration_writable(const char * const keystr);

/**
 * @brief Checks if a configuration key is readable.
 *
 * This function checks if the specified configuration key is readable.
 *
 * @param[in] keystr The configuration key to check.
 *
 * @return true if the configuration key is readable, false otherwise.
 */
bool ocpp_is_configuration_readable(const char * const keystr);

/**
 * @brief Gets the size of the configuration value for a given key.
 *
 * This function returns the size of the configuration value associated with the
 * specified key.
 *
 * @param[in] keystr The configuration key to check.
 *
 * @return The size of the configuration value.
 */
size_t ocpp_get_configuration_size(const char * const keystr);

/**
 * @brief Gets the data type of the configuration value for a given key.
 *
 * This function returns the data type of the configuration value associated
 * with the specified key.
 *
 * @param[in] keystr The configuration key to check.
 *
 * @return The data type of the configuration value.
 */
ocpp_configuration_data_t ocpp_get_configuration_data_type(
		const char * const keystr);

/**
 * @brief Gets the configuration key string by index.
 *
 * This function returns the configuration key string at the specified index.
 *
 * @param[in] index The index of the configuration key string to retrieve.
 * @return A pointer to the configuration key string.
 */
const char *ocpp_get_configuration_keystr_from_index(int index);

/**
 * @brief Converts a configuration key to its corresponding string value.
 *
 * This function takes a configuration key as input and finds the corresponding
 * value. The value is then converted to a string and stored in the provided
 * buffer. If the key does not exist, the function returns null.
 *
 * @param[in] keystr The configuration key to be converted.
 * @param[out] buf The buffer where the resulting string value will be stored.
 * @param[in] bufsize The size of the buffer.
 *
 * @return A pointer to the resulting string value, or null if the key does not
 *         exist.
 */
const char *ocpp_stringify_configuration_value(const char *keystr,
		char *buf, size_t bufsize);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_OCPP_CONFIGURATION_H */
