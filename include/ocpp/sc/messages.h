/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_OCPP_SMART_CHARGING_MESSAGES_H
#define LIBMCU_OCPP_SMART_CHARGING_MESSAGES_H

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
	ocpp_profile_status_t status;
	int connectorId;
	time_t scheduleStart;
	uint8_t chargingSchedule[]; /* struct ocpp_ChargingSchedule */
};

struct ocpp_SetChargingProfile {
	int connectorId;
	uint8_t csChargingProfiles[]; /* struct ocpp_ChargingProfile */
};

struct ocpp_SetChargingProfile_conf {
	ocpp_profile_status_t status;
};

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_OCPP_SMART_CHARGING_MESSAGES_H */
