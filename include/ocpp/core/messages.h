/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef OCPP_CORE_MESSAGES_H
#define OCPP_CORE_MESSAGES_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "ocpp/type.h"

struct ocpp_Authorize {
	char idTag[20+1];
};

struct ocpp_Authorize_conf {
	struct ocpp_idTagInfo idTagInfo;
};

struct ocpp_BootNotification {
	char chargeBoxSerialNumber[25+1];
	char chargePointModel[20+1];		/* required */
	char chargePointSerialNumber[25+1];
	char chargePointVendor[20+1];		/* required */
	char firmwareVersion[50+1];
	char iccid[20+1];
	char imsi[20+1];
	char meterSerialNumber[25+1];
	char meterType[25+1];
};

struct ocpp_BootNotification_conf {
	time_t currentTime;
	int interval;
	ocpp_boot_status_t status;
};

struct ocpp_ChangeAvailability {
	int connectorId;
};

struct ocpp_ChangeAvailability_conf {
	ocpp_availability_t type;
};

struct ocpp_ChangeConfiguration {
	char key[50+1];
	char value[500+1];
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
	char vendorId[255+1]; /* required */
	char messageId[50+1];
	char data[0];
};

struct ocpp_DataTransfer_conf {
	ocpp_data_status_t status;
	char data[0];
};

struct ocpp_GetConfiguration {
	char key[50+1];
};

struct ocpp_GetConfiguration_conf {
	struct ocpp_KeyValue configurationKey;
	char unknownKey[50+1];
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
	struct ocpp_MeterValue meterValue;
};

struct ocpp_MeterValues_conf {
	int none;
};

struct ocpp_RemoteStartTransaction {
	int connectorId;
	char idTag[20+1];
	struct ocpp_ChargingProfile chargingProfile;
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
	char idTag[20+1];
	int meterStart;
	int reservationId;
	time_t timestamp;
};

struct ocpp_StartTransaction_conf {
	struct ocpp_idTagInfo idTagInfo;
	int transactionId;
};

struct ocpp_StatusNotification {
	int connectorId;
	ocpp_error_t errorCode;
	char info[50+1];
	ocpp_status_t status;
	time_t timestamp;
	char vendorId[255+1];
	char vendorErrorCode[50+1];
};

struct ocpp_StatusNotification_conf {
	int none;
};

struct ocpp_StopTransaction {
	char idTag[20+1];
	int meterStop;
	time_t timestamp;
	int transactionId;
	ocpp_stop_reason_t reason;
	struct ocpp_MeterValue transactionData;
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

#endif /* OCPP_CORE_MESSAGES_H */
