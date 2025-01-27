/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_OCPP_STRINGIFY_H
#define LIBMCU_OCPP_STRINGIFY_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "ocpp/type.h"

/**
 * @brief Converts an ocpp_profile_t enum value to its string representation.
 *
 * This function takes an ocpp_profile_t enum value and converts it to a string
 * representation, storing the result in the provided buffer. The buffer size
 * should be sufficient to hold the resulting string. If the profile is not
 * valid, the function will return NULL.
 *
 * @param[out] buf The buffer to store the string representation.
 * @param[in] bufsize The size of the buffer.
 * @param[in] profile The ocpp_profile_t enum value to be converted.
 *
 * @return A pointer to the buffer containing the string representation, or NULL
 *         if the profile is not valid.
 */
const char *ocpp_stringify_profile(char *buf, const size_t bufsize,
		const ocpp_profile_t profile);

/**
 * @brief Converts an ocpp_measurand_t enum value to its string representation.
 *
 * This function takes an ocpp_measurand_t enum value and converts it to a
 * string representation, storing the result in the provided buffer. The buffer
 * size should be sufficient to hold the resulting string. If the measurand is
 * not valid, the function will return NULL.
 *
 * @param[out] buf The buffer to store the string representation.
 * @param[in] bufsize The size of the buffer.
 * @param[in] measurand The ocpp_measurand_t enum value to be converted.
 *
 * @return A pointer to the buffer containing the string representation, or NULL
 *         if the measurand is not valid.
 */
const char *ocpp_stringify_measurand(char *buf, const size_t bufsize,
		const ocpp_measurand_t measurand);

const char *ocpp_stringify_comm_status(ocpp_comm_status_t status);
const char *ocpp_stringify_error(ocpp_error_t err);
const char *ocpp_stringify_status(ocpp_status_t status);
const char *ocpp_stringify_availability_status(ocpp_availability_status_t status);
const char *ocpp_stringify_config_status(ocpp_config_status_t status);
const char *ocpp_stringify_data_status(ocpp_data_status_t status);
const char *ocpp_stringify_profile_status(ocpp_profile_status_t status);
const char *ocpp_stringify_remote_status(ocpp_remote_status_t status);
const char *ocpp_stringify_reservation_status(ocpp_reservation_status_t status);
const char *ocpp_stringify_trigger_status(ocpp_trigger_status_t status);
const char *ocpp_stringify_stop_reason(ocpp_stop_reason_t reason);

/**
 * @brief Converts a string to an ocpp_measurand_t enum value.
 *
 * This function takes a string representation of a measurand and converts it
 * to the corresponding ocpp_measurand_t enum value. If the string does not
 * match any known measurand, the function will return 0.
 *
 * @param[in] str The string representation of the measurand.
 * @param[in] len The length of the string.
 *
 * @return The corresponding ocpp_measurand_t enum value, or 0 if no match is
 *         found.
 */
ocpp_measurand_t ocpp_get_measurand_from_string(const char *str,
		const size_t len);

ocpp_auth_status_t ocpp_get_auth_status_from_string(const char *str);
ocpp_boot_status_t ocpp_get_boot_status_from_string(const char *str);

const char *ocpp_stringify_context(ocpp_reading_context_t ctx);
const char *ocpp_stringify_unit(ocpp_measure_unit_t unit);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_OCPP_STRINGIFY_H */
