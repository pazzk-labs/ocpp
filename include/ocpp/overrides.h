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

struct ocpp_message;

int ocpp_send(const struct ocpp_message *msg);
int ocpp_recv(struct ocpp_message *msg);

void ocpp_generate_message_id(void *buf, size_t bufsize);

int ocpp_lock(void);
int ocpp_unlock(void);

int ocpp_configuration_lock(void);
int ocpp_configuration_unlock(void);

#if defined(__cplusplus)
}
#endif

#endif /* OCPP_OVERRIDES_H */
