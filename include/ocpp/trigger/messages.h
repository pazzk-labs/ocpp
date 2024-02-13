/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef OCPP_TRIGGER_MESSAGES_H
#define OCPP_TRIGGER_MESSAGES_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <time.h>
#include "ocpp/type.h"

struct ocpp_TriggerMessage {
	ocpp_trigger_message_t requestedMessage;
	int connectorId;
};

struct ocpp_TriggerMessage_conf {
	ocpp_status_t status;
};

#if defined(__cplusplus)
}
#endif

#endif /* OCPP_TRIGGER_MESSAGES_H */
