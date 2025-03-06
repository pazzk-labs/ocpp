/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "ocpp/strconv.h"
#include <string.h>

#if !defined(ARRAY_COUNT)
#define ARRAY_COUNT(x)		(sizeof(x) / sizeof((x)[0]))
#endif

const char *ocpp_stringify_comm_status(ocpp_comm_status_t status)
{
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

ocpp_measurand_t ocpp_get_measurand_from_string(const char *str,
		const size_t len)
{
	const struct {
		const char *str;
		ocpp_measurand_t code;
	} supported[] = {
		{ "Current.Export", OCPP_MEASURAND_CURRENT_EXPORT },
		{ "Current.Import", OCPP_MEASURAND_CURRENT_IMPORT },
		{ "Current.Offered", OCPP_MEASURAND_CURRENT_OFFERED },
		{ "Energy.Active.Export.Register", OCPP_MEASURAND_ENERGY_ACTIVE_EXPORT_REGISTER },
		{ "Energy.Active.Import.Register", OCPP_MEASURAND_ENERGY_ACTIVE_IMPORT_REGISTER },
		{ "Energy.Reactive.Export.Register", OCPP_MEASURAND_ENERGY_REACTIVE_EXPORT_REGISTER },
		{ "Energy.Reactive.Import.Register", OCPP_MEASURAND_ENERGY_REACTIVE_IMPORT_REGISTER },
		{ "Energy.Active.Export.Interval", OCPP_MEASURAND_ENERGY_ACTIVE_EXPORT_INTERVAL },
		{ "Energy.Active.Import.Interval", OCPP_MEASURAND_ENERGY_ACTIVE_IMPORT_INTERVAL },
		{ "Energy.Reactive.Export.Interval", OCPP_MEASURAND_ENERGY_REACTIVE_EXPORT_INTERVAL },
		{ "Energy.Reactive.Import.Interval", OCPP_MEASURAND_ENERGY_REACTIVE_IMPORT_INTERVAL },
		{ "Frequency", OCPP_MEASURAND_FREQUENCY },
		{ "Power.Active.Export", OCPP_MEASURAND_POWER_ACTIVE_EXPORT },
		{ "Power.Active.Import", OCPP_MEASURAND_POWER_ACTIVE_IMPORT },
		{ "Power.Factor", OCPP_MEASURAND_POWER_FACTOR },
		{ "Power.Offered", OCPP_MEASURAND_POWER_OFFERED },
		{ "Power.Reactive.Export", OCPP_MEASURAND_POWER_REACTIVE_EXPORT },
		{ "Power.Reactive.Import", OCPP_MEASURAND_POWER_REACTIVE_IMPORT },
		{ "RPM", OCPP_MEASURAND_RPM },
		{ "SoC", OCPP_MEASURAND_SOC },
		{ "Temperature", OCPP_MEASURAND_TEMPERATURE },
		{ "Voltage", OCPP_MEASURAND_VOLTAGE },
	};

	for (size_t i = 0; i < ARRAY_COUNT(supported); i++) {
		if (strncmp(str, supported[i].str, len) == 0) {
			return supported[i].code;
		}
	}

	return (ocpp_measurand_t)0;
}

ocpp_auth_status_t ocpp_get_auth_status_from_string(const char *str)
{
	if (strcmp(str, "Accepted") == 0) {
		return OCPP_AUTH_STATUS_ACCEPTED;
	} else if (strcmp(str, "Blocked") == 0) {
		return OCPP_AUTH_STATUS_BLOCKED;
	} else if (strcmp(str, "Expired") == 0) {
		return OCPP_AUTH_STATUS_EXPIRED;
	} else if (strcmp(str, "Invalid") == 0) {
		return OCPP_AUTH_STATUS_INVALID;
	} else if (strcmp(str, "ConcurrentTx") == 0) {
		return OCPP_AUTH_STATUS_CONCURRENT_TX;
	} else {
		return OCPP_AUTH_STATUS_UNKNOWN;
	}
}

ocpp_boot_status_t ocpp_get_boot_status_from_string(const char *str)
{
	if (strcmp(str, "Accepted") == 0) {
		return OCPP_BOOT_STATUS_ACCEPTED;
	} else if (strcmp(str, "Rejected") == 0) {
		return OCPP_BOOT_STATUS_REJECTED;
	} else if (strcmp(str, "Pending") == 0) {
		return OCPP_BOOT_STATUS_PENDING;
	} else {
		return OCPP_BOOT_STATUS_UNKNOWN;
	}
}

const char *ocpp_stringify_context(ocpp_reading_context_t ctx)
{
	const char *tbl[] = {
		[OCPP_READ_CTX_UNKNOWN] = "Unknown",
		[OCPP_READ_CTX_INT_BEGIN] = "Interruption.Begin",
		[OCPP_READ_CTX_INT_END] = "Interruption.End",
		[OCPP_READ_CTX_OTHER] = "Other",
		[OCPP_READ_CTX_SAMPLE_CLOCK] = "Sample.Clock",
		[OCPP_READ_CTX_SAMPLE_PERIODIC] = "Sample.Periodic",
		[OCPP_READ_CTX_TRANSACTION_BEGIN] = "Transaction.Begin",
		[OCPP_READ_CTX_TRANSACTION_END] = "Transaction.End",
		[OCPP_READ_CTX_TRIGGER] = "Trigger",
	};

	return tbl[ctx];
}

const char *ocpp_stringify_unit(ocpp_measure_unit_t unit)
{
	const char *tbl[] = {
		[OCPP_UNIT_UNKNOWN] = "Unknown",
		[OCPP_UNIT_WH] = "Wh",
		[OCPP_UNIT_KWH] = "kWh",
		[OCPP_UNIT_VARH] = "varh",
		[OCPP_UNIT_KVARH] = "kvarh",
		[OCPP_UNIT_W] = "W",
		[OCPP_UNIT_KW] = "kW",
		[OCPP_UNIT_VA] = "VA",
		[OCPP_UNIT_KVA] = "kVA",
		[OCPP_UNIT_VAR] = "var",
		[OCPP_UNIT_KVAR] = "kvar",
		[OCPP_UNIT_A] = "A",
		[OCPP_UNIT_V] = "V",
		[OCPP_UNIT_CELSIUS] = "Celsius",
		[OCPP_UNIT_FAHRENHEIT] = "Fahrenheit",
		[OCPP_UNIT_K] = "K",
		[OCPP_UNIT_PERCENT] = "Percent",
	};

	return tbl[unit];
}

const char *ocpp_stringify_measurand(ocpp_measurand_t measurand)
{
	const char *tbl[] = {
		"Current.Export",
		"Current.Import",
		"Current.Offered",
		"Energy.Active.Export.Register",
		"Energy.Active.Import.Register",
		"Energy.Reactive.Export.Register",
		"Energy.Reactive.Import.Register",
		"Energy.Active.Export.Interval",
		"Energy.Active.Import.Interval",
		"Energy.Reactive.Export.Interval",
		"Energy.Reactive.Import.Interval",
		"Frequency",
		"Power.Active.Export",
		"Power.Active.Import",
		"Power.Factor",
		"Power.Offered",
		"Power.Reactive.Export",
		"Power.Reactive.Import",
		"RPM",
		"SoC",
		"Temperature",
		"Voltage",
	};

	return tbl[__builtin_ctz(measurand)];
}
