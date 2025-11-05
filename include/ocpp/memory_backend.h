/*
 * SPDX-FileCopyrightText: 2025 권경환 Kyunghwan Kwon <k@pazzk.net>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef PAZZK_OCPP_MEMORY_BACKEND_H
#define PAZZK_OCPP_MEMORY_BACKEND_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "ocpp/backend.h"

struct ocpp_backend *ocpp_memory_backend_create(void);
void ocpp_memory_backend_destroy(struct ocpp_backend *backend);

#if defined(__cplusplus)
}
#endif

#endif /* PAZZK_OCPP_MEMORY_BACKEND_H */
