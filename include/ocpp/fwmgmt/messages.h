/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_OCPP_FWMGMT_MESSAGES_H
#define LIBMCU_OCPP_FWMGMT_MESSAGES_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <time.h>
#include "ocpp/type.h"

struct ocpp_DiagnosticsStatusNotification {
	ocpp_comm_status_t status;
};

struct ocpp_DiagnosticsStatusNotification_conf {
	int none;
};

struct ocpp_FirmwareStatusNotification {
	ocpp_comm_status_t status;
};

struct ocpp_FirmwareStatusNotification_conf {
	int none;
};

struct ocpp_GetDiagnostics {
	char url[256+1];
	int retries;
	int retryInterval;
	time_t startTime;
	time_t stopTime;
};

struct ocpp_GetDiagnostics_conf {
	char fileName[255+1];
};

struct ocpp_UpdateFirmware {
	char url[256+1];
	int retries;
	time_t retrieveDate;
	int retryInterval;
};

struct ocpp_UpdateFirmware_conf {
	int none;
};

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_OCPP_FWMGMT_MESSAGES_H */
