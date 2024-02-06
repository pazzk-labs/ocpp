/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef OCPP_OVERRIDES_H
#define OCPP_OVERRIDES_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

int ocpp_send(const void *data, size_t datasize);
int ocpp_recv(void *buf, size_t bufsize);

int ocpp_lock(void);
int ocpp_unlock(void);

int ocpp_free_data(void *data);

#if defined(__cplusplus)
}
#endif

#endif /* OCPP_OVERRIDES_H */
