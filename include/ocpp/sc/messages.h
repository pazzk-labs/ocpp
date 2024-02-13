/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef OCPP_SMART_CHARGING_MESSAGES_H
#define OCPP_SMART_CHARGING_MESSAGES_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "ocpp/type.h"

struct ocpp_ClearChargingProfile {
	int implement;
};

struct ocpp_ClearChargingProfile_conf {
	int implement;
};

struct ocpp_GetCompositeSchedule {
	int connectorId;
	int duration;
	ocpp_charging_unit_t chargingRateUnit;
};

struct ocpp_GetCompositeSchedule_conf {
	ocpp_status_t status;
	int connectorId;
	time_t scheduleStart;
	struct ocpp_ChargingSchedule chargingSchedule;
};

struct ocpp_SetChargingProfile {
	int connectorId;
	struct ocpp_ChargingProfile csChargingProfiles;
};

struct ocpp_SetChargingProfile_conf {
	ocpp_status_t status;
};

#if defined(__cplusplus)
}
#endif

#endif /* OCPP_SMART_CHARGING_MESSAGES_H */