/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_OCPP_STRINGIFY_H
#define LIBMCU_OCPP_STRINGIFY_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "ocpp/type.h"

const char *ocpp_stringify_fw_update_status(ocpp_comm_status_t status);
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

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_OCPP_STRINGIFY_H */
