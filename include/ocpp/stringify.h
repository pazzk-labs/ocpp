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

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_OCPP_STRINGIFY_H */
