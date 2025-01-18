/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "ocpp/stringify.h"

const char *ocpp_stringify_fw_update_status(ocpp_comm_status_t status) {
	const char *tbl[] = {
		[OCPP_COMM_IDLE] = "Idle",
		[OCPP_COMM_UPLOADED] = "Uploaded",
		[OCPP_COMM_UPLOAD_FAILED] = "UploadFailed",
		[OCPP_COMM_UPLOADING] = "Uploading",
		[OCPP_COMM_DOWNLOADED] = "Downloaded",
		[OCPP_COMM_DOWNLOAD_FAILED] = "DownloadFailed",
		[OCPP_COMM_DOWNLOADING] = "Downloading",
		[OCPP_COMM_INSTALLATION_FAILED] = "InstallationFailed",
		[OCPP_COMM_INSTALLING] = "Installing",
		[OCPP_COMM_INSTALLED] = "Installed",
	};

	return tbl[status];
}

const char *ocpp_stringify_error(ocpp_error_t err)
{
	const char *tbl[] = {
		[OCPP_ERROR_NONE] = "NoError",
		[OCPP_ERROR_CONNECTOR_LOCK_FAILURE] = "ConnectorLockFailure",
		[OCPP_ERROR_EV_COMMUNICATION] = "EVCommunicationError",
		[OCPP_ERROR_GROUND] = "GroundFailure",
		[OCPP_ERROR_HIGH_TEMPERATURE] = "HighTemperature",
		[OCPP_ERROR_INTERNAL] = "InternalError",
		[OCPP_ERROR_LOCAL_LIST_CONFLICT] = "LocalListConflict",
		[OCPP_ERROR_OTHER] = "OtherError",
		[OCPP_ERROR_OVER_CURRENT] = "OverCurrentFailure",
		[OCPP_ERROR_OVER_VOLTAGE] = "OverVoltage",
		[OCPP_ERROR_POWER_METER] = "PowerMeterFailure",
		[OCPP_ERROR_POWER_SWITCH] = "PowerSwitchFailure",
		[OCPP_ERROR_READER] = "ReaderFailure",
		[OCPP_ERROR_RESET] = "ResetFailure",
		[OCPP_ERROR_UNDER_VOLTAGE] = "UnderVoltage",
		[OCPP_ERROR_WEAK_SIGNAL] = "WeakSignal",
	};

	return tbl[err];
}

const char *ocpp_stringify_status(ocpp_status_t status)
{
	const char *tbl[] = {
		[OCPP_STATUS_AVAILABLE] = "Available",
		[OCPP_STATUS_PREPARING] = "Preparing",
		[OCPP_STATUS_CHARGING] = "Charging",
		[OCPP_STATUS_SUSPENDED_EVSE] = "SuspendedEVSE",
		[OCPP_STATUS_SUSPENDED_EV] = "SuspendedEV",
		[OCPP_STATUS_FINISHING] = "Finishing",
		[OCPP_STATUS_RESERVED] = "Reserved",
		[OCPP_STATUS_UNAVAILABLE] = "Unavailable",
		[OCPP_STATUS_FAULTED] = "Faulted",
	};

	return tbl[status];
}

const char *ocpp_stringify_availability_status(ocpp_availability_status_t status)
{
	const char *tbl[] = {
		[OCPP_AVAILABILITY_STATUS_ACCEPTED] = "Accepted",
		[OCPP_AVAILABILITY_STATUS_REJECTED] = "Rejected",
		[OCPP_AVAILABILITY_STATUS_SCHEDULED] = "Scheduled",
	};

	return tbl[status];
}

const char *ocpp_stringify_config_status(ocpp_config_status_t status)
{
	const char *tbl[] = {
		[OCPP_CONFIG_STATUS_ACCEPTED] = "Accepted",
		[OCPP_CONFIG_STATUS_REJECTED] = "Rejected",
		[OCPP_CONFIG_STATUS_REBOOT_REQUIRED] = "RebootRequired",
		[OCPP_CONFIG_STATUS_NOT_SUPPORTED] = "NotSupported",
	};

	return tbl[status];
}

const char *ocpp_stringify_data_status(ocpp_data_status_t status)
{
	const char *tbl[] = {
		[OCPP_DATA_STATUS_ACCEPTED] = "Accepted",
		[OCPP_DATA_STATUS_REJECTED] = "Rejected",
		[OCPP_DATA_STATUS_UNKNOWN_VENDOR_ID] = "UnknownVendorId",
		[OCPP_DATA_STATUS_UNKNOWN_MESSAGE_ID] = "UnknownMessageId",
	};

	return tbl[status];
}

const char *ocpp_stringify_profile_status(ocpp_profile_status_t status)
{
	const char *tbl[] = {
		[OCPP_PROFILE_STATUS_ACCEPTED] = "Accepted",
		[OCPP_PROFILE_STATUS_REJECTED] = "Rejected",
		[OCPP_PROFILE_STATUS_NOT_SUPPORTED] = "NotSupported",
		[OCPP_PROFILE_STATUS_UNKNOWN] = "Unknown",
	};

	return tbl[status];
}

const char *ocpp_stringify_remote_status(ocpp_remote_status_t status)
{
	const char *tbl[] = {
		[OCPP_REMOTE_STATUS_ACCEPTED] = "Accepted",
		[OCPP_REMOTE_STATUS_REJECTED] = "Rejected",
	};

	return tbl[status];
}

const char *ocpp_stringify_reservation_status(ocpp_reservation_status_t status)
{
	const char *tbl[] = {
		[OCPP_RESERVE_STATUS_ACCEPTED] = "Accepted",
		[OCPP_RESERVE_STATUS_FAULTED] = "Faulted",
		[OCPP_RESERVE_STATUS_OCCUPIED] = "Occupied",
		[OCPP_RESERVE_STATUS_REJECTED] = "Rejected",
		[OCPP_RESERVE_STATUS_UNAVAILABLE] = "Unavailable",
	};

	return tbl[status];
}

const char *ocpp_stringify_trigger_status(ocpp_trigger_status_t status)
{
	const char *tbl[] = {
		[OCPP_TRIGGER_STATUS_ACCEPTED] = "Accepted",
		[OCPP_TRIGGER_STATUS_REJECTED] = "Rejected",
		[OCPP_TRIGGER_STATUS_NOT_IMPLEMENTED] = "NotImplemented",
	};

	return tbl[status];
}

const char *ocpp_stringify_stop_reason(ocpp_stop_reason_t reason)
{
	const char *tbl[] = {
		[OCPP_STOP_REASON_LOCAL] = "Local",
		[OCPP_STOP_REASON_DEAUTHORIZED] = "DeAuthorized",
		[OCPP_STOP_REASON_EMERGENCY_STOP] = "EmergencyStop",
		[OCPP_STOP_REASON_EV_DISCONNECTED] = "EVDisconnected",
		[OCPP_STOP_REASON_HARD_RESET] = "HardReset",
		[OCPP_STOP_REASON_OTHER] = "Other",
		[OCPP_STOP_REASON_POWER_LOSS] = "PowerLoss",
		[OCPP_STOP_REASON_REBOOT] = "Reboot",
		[OCPP_STOP_REASON_REMOTE] = "Remote",
		[OCPP_STOP_REASON_SOFT_RESET] = "SoftReset",
		[OCPP_STOP_REASON_UNLOCK_COMMAND] = "UnlockCommand",
	};

	return tbl[reason];
}
