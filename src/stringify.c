/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "ocpp/type.h"

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
