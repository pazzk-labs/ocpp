/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_OCPP_H
#define LIBMCU_OCPP_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "ocpp/core/configuration.h"

#include "ocpp/core/messages.h"
#include "ocpp/fwmgmt/messages.h"
#include "ocpp/local/messages.h"
#include "ocpp/reserve/messages.h"
#include "ocpp/sc/messages.h"
#include "ocpp/trigger/messages.h"

#include "ocpp/overrides.h"

typedef void (*ocpp_event_callback_t)(int err,
		const struct ocpp_message *message, void *ctx);

union ocpp_message_req {
	struct ocpp_Authorize Authorize;
	struct ocpp_BootNotification BootNotification;
	struct ocpp_ChangeAvailability ChangeAvailability;
	struct ocpp_ChangeConfiguration ChangeConfiguration;
	struct ocpp_ClearCache ClearCache;
	struct ocpp_DataTransfer DataTransfer;
	struct ocpp_GetConfiguration GetConfiguration;
	struct ocpp_Heartbeat HeartBeat;
	struct ocpp_MeterValues MeterValues;
	struct ocpp_RemoteStartTransaction RemoteStartTransaction;
	struct ocpp_RemoteStopTransaction RemoteStopTransaction;
	struct ocpp_Reset Reset;
	struct ocpp_StartTransaction StartTransaction;
	struct ocpp_StatusNotification StatusNotification;
	struct ocpp_StopTransaction StopTransaction;
	struct ocpp_UnlockConnector UnlockConnector;

	struct ocpp_DiagnosticsStatusNotification DiagnosticsStatusNotification;
	struct ocpp_FirmwareStatusNotification FirmwareStatusNotification;
	struct ocpp_GetDiagnostics GetDiagnostics;
	struct ocpp_UpdateFirmware UpdateFirmware;

	struct ocpp_GetLocalListVersion GetLocalListVersion;
	struct ocpp_SendLocalList SendLocalList;

	struct ocpp_CancelReservation CancelReservation;
	struct ocpp_ReserveNow ReserveNow;

	struct ocpp_ClearChargingProfile ClearChargingProfile;
	struct ocpp_GetCompositeSchedule GetCompositeSchedule;
	struct ocpp_SetChargingProfile SetChargingProfile;

	struct ocpp_TriggerMessage TriggerMessage;
};

union ocpp_message_conf {
	struct ocpp_Authorize_conf Authorize;
	struct ocpp_BootNotification_conf BootNotification;
	struct ocpp_ChangeAvailability_conf ChangeAvailability;
	struct ocpp_ChangeConfiguration_conf ChangeConfiguration;
	struct ocpp_ClearCache_conf ClearCache;
	struct ocpp_DataTransfer_conf DataTransfer;
	struct ocpp_GetConfiguration_conf GetConfiguration;
	struct ocpp_Heartbeat_conf HeartBeat;
	struct ocpp_MeterValues_conf MeterValues;
	struct ocpp_RemoteStartTransaction_conf RemoteStartTransaction;
	struct ocpp_RemoteStopTransaction_conf RemoteStopTransaction;
	struct ocpp_Reset_conf Reset;
	struct ocpp_StartTransaction_conf StartTransaction;
	struct ocpp_StatusNotification_conf StatusNotification;
	struct ocpp_StopTransaction_conf StopTransaction;
	struct ocpp_UnlockConnector_conf UnlockConnector;

	struct ocpp_DiagnosticsStatusNotification_conf DiagnosticsStatusNotification;
	struct ocpp_FirmwareStatusNotification_conf FirmwareStatusNotification;
	struct ocpp_GetDiagnostics_conf GetDiagnostics;
	struct ocpp_UpdateFirmware_conf UpdateFirmware;

	struct ocpp_GetLocalListVersion_conf GetLocalListVersion;
	struct ocpp_SendLocalList_conf SendLocalList;

	struct ocpp_CancelReservation_conf CancelReservation;
	struct ocpp_ReserveNow_conf ReserveNow;

	struct ocpp_ClearChargingProfile_conf ClearChargingProfile;
	struct ocpp_GetCompositeSchedule_conf GetCompositeSchedule;
	struct ocpp_SetChargingProfile_conf SetChargingProfile;

	struct ocpp_TriggerMessage_conf TriggerMessage;
};

struct ocpp_message {
	char id[OCPP_MESSAGE_ID_MAXLEN];
	ocpp_message_role_t role;
	ocpp_message_t type;

	union {
		union ocpp_message_req req;
		union ocpp_message_conf conf;
	} fmt;
};

int ocpp_init(ocpp_event_callback_t cb, void *cb_ctx);
int ocpp_step(void);
int ocpp_push_request(ocpp_message_t type, const void *data, size_t datasize);
int ocpp_push_request_defer(ocpp_message_t type,
		const void *data, size_t datasize, uint32_t timer_sec);
int ocpp_push_response(const struct ocpp_message *req,
		const void *data, size_t datasize, bool err);
/**
 * @brief Save the current OCPP context as a snapshot.
 *
 * @param[out] buf buffer for the snapshot to be saved
 * @param[in] bufsize size of the buffer
 *
 * @note A header is included in the snapshot for validation upon restore,
 *       which is processed internally.
 *
 * @return 0 for success, otherwise an error.
 */
int ocpp_save_snapshot(void *buf, size_t bufsize);
/**
 * @brief Restore the OCPP context from a snapshot.
 *
 * @param[in] snapshot snapshot to be loaded
 *
 * @note No need to call `ocpp_init()` when this function is used.
 *
 * @return 0 for success, otherwise an error.
 */
int ocpp_restore_snapshot(const void *snapshot);
size_t ocpp_compute_snapshot_size(void);

const char *ocpp_stringify_type(ocpp_message_t msgtype);

ocpp_message_t ocpp_get_type_from_string(const char *typestr);
/**
 * @brief Get message type from ID string
 *
 * @param[in] idstr ID string
 *
 * @return Type of message. `OCPP_MSG_MAX` if no matching found.
 */
ocpp_message_t ocpp_get_type_from_idstr(const char *idstr);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_OCPP_H */
