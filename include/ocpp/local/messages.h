/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef OCPP_LOCAL_AUTH_MESSAGES_H
#define OCPP_LOCAL_AUTH_MESSAGES_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "ocpp/type.h"

struct ocpp_GetLocalListVersion {
	int none;
};

struct ocpp_GetLocalListVersion_conf {
	int listVersion;
};

struct ocpp_SendLocalList {
	int listVersion;
	struct ocpp_AuthorizationData localAuthorizationList;
	ocpp_update_t updateType;
};

struct ocpp_SendLocalList_conf {
	ocpp_update_status_t status;
};

#if defined(__cplusplus)
}
#endif

#endif /* OCPP_LOCAL_AUTH_MESSAGES_H */
