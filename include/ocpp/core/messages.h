/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_OCPP_CORE_MESSAGES_H
#define LIBMCU_OCPP_CORE_MESSAGES_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "ocpp/type.h"
#include <stdint.h>

struct ocpp_Authorize {
	char idTag[OCPP_CiString20];
};

struct ocpp_Authorize_conf {
	struct ocpp_idTagInfo idTagInfo;
};

struct ocpp_BootNotification {
	char chargeBoxSerialNumber[OCPP_CiString25];
	char chargePointModel[OCPP_CiString20];		/* required */
	char chargePointSerialNumber[OCPP_CiString25];
	char chargePointVendor[OCPP_CiString20];	/* required */
	char firmwareVersion[OCPP_CiString50];
	char iccid[OCPP_CiString20];
	char imsi[OCPP_CiString20];
	char meterSerialNumber[OCPP_CiString25];
	char meterType[OCPP_CiString25];
};

struct ocpp_BootNotification_conf {
	time_t currentTime;
	int interval;
	ocpp_boot_status_t status;
};

struct ocpp_ChangeAvailability {
	int connectorId;
	ocpp_availability_t type;
};

struct ocpp_ChangeAvailability_conf {
	ocpp_availability_status_t status;
};

struct ocpp_ChangeConfiguration {
	char key[OCPP_CiString50];
	char value[OCPP_CiString500];
};

struct ocpp_ChangeConfiguration_conf {
	ocpp_config_status_t status;
};

struct ocpp_ClearCache {
	int none;
};

struct ocpp_ClearCache_conf {
	ocpp_remote_status_t status;
};

struct ocpp_DataTransfer {
	char vendorId[OCPP_CiString255]; /* required */
	char messageId[OCPP_CiString50];
	char padding[13];
	char data[];
};

struct ocpp_DataTransfer_conf {
	ocpp_data_status_t status;
	char data[];
};

struct ocpp_GetConfiguration {
	char dummy;
	char keys[];
};

struct ocpp_GetConfiguration_conf {
	struct ocpp_KeyValue *configurationKey;
	char *unknownKey;
};

struct ocpp_Heartbeat {
	int none;
};

struct ocpp_Heartbeat_conf {
	time_t currentTime;
};

struct ocpp_MeterValues {
	int connectorId;
	int transactionId;
	uint8_t meterValue[]; /* struct ocpp_MeterValue */
};

struct ocpp_MeterValues_conf {
	int none;
};

struct ocpp_RemoteStartTransaction {
	int connectorId;
	char idTag[OCPP_CiString20];
	uint8_t chargingProfile[]; /* struct ocpp_ChargingProfile */
};

struct ocpp_RemoteStartTransaction_conf {
	ocpp_remote_status_t status;
};

struct ocpp_RemoteStopTransaction {
	int transactionId;
};

struct ocpp_RemoteStopTransaction_conf {
	ocpp_remote_status_t status;
};

struct ocpp_Reset {
	ocpp_reset_t type;
};

struct ocpp_Reset_conf {
	ocpp_remote_status_t status;
};

struct ocpp_StartTransaction {
	int connectorId;
	char idTag[OCPP_CiString20];
	uint64_t meterStart;
	int reservationId;
	time_t timestamp;
};

struct ocpp_StartTransaction_conf {
	struct ocpp_idTagInfo idTagInfo;
	int transactionId;
};

struct ocpp_StatusNotification {
	int connectorId;		/* required */
	ocpp_error_t errorCode;		/* required */
	char info[OCPP_CiString50];
	ocpp_status_t status;		/* required */
	time_t timestamp;
	char vendorId[OCPP_CiString255];
	char vendorErrorCode[OCPP_CiString50];
};

struct ocpp_StatusNotification_conf {
	int none;
};

struct ocpp_StopTransaction {
	char idTag[20+1];
	uint64_t meterStop;
	time_t timestamp;
	int transactionId;
	ocpp_stop_reason_t reason;
	uint8_t meterValue[]; /* struct ocpp_MeterValue */
};

struct ocpp_StopTransaction_conf {
	struct ocpp_idTagInfo idTagInfo;
};

struct ocpp_UnlockConnector {
	int connectorId;
};

struct ocpp_UnlockConnector_conf {
	ocpp_unlock_status_t status;
};

int ocpp_send_bootnotification(const struct ocpp_BootNotification *msg);
int ocpp_send_datatransfer(const struct ocpp_DataTransfer *msg);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_OCPP_CORE_MESSAGES_H */
