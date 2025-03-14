/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_OCPP_SECURITY_MESSAGES_H
#define LIBMCU_OCPP_SECURITY_MESSAGES_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <time.h>
#include "ocpp/type.h"

typedef enum {
	OCPP_SECURITY_STATUS_ACCEPTED,
	OCPP_SECURITY_STATUS_REJECTED,
	OCPP_SECURITY_STATUS_FAILED,
	OCPP_SECURITY_STATUS_NOT_FOUND,
	OCPP_SECURITY_STATUS_ACCEPTED_CANCELED,
	OCPP_SECURITY_STATUS_NOT_IMPLEMENTED,
	OCPP_SECURITY_STATUS_INVALID_CERTIFICATE,
	OCPP_SECURITY_STATUS_REVOKED_CERTIFICATE,
	OCPP_SECURITY_STATUS_BAD_MESSAGE,
	OCPP_SECURITY_STATUS_IDLE,
	OCPP_SECURITY_STATUS_NOT_SUPPORTED,
	OCPP_SECURITY_STATUS_PERMISSION_DENIED,
	OCPP_SECURITY_STATUS_UPLOADED,
	OCPP_SECURITY_STATUS_UPLOAD_FAILED,
	OCPP_SECURITY_STATUS_UPLOADING,

	OCPP_SECURITY_STATUS_DOWNLOADED,
	OCPP_SECURITY_STATUS_DOWNLOAD_FAILED,
	OCPP_SECURITY_STATUS_DOWNLOADING,
	OCPP_SECURITY_STATUS_DOWNLOAD_SCHEDULED,
	OCPP_SECURITY_STATUS_DOWNLOAD_PAUSED,
	OCPP_SECURITY_STATUS_INSTALLATION_FAILED,
	OCPP_SECURITY_STATUS_INSTALLING,
	OCPP_SECURITY_STATUS_INSTALLED,
	OCPP_SECURITY_STATUS_INSTALL_REBOOTING,
	OCPP_SECURITY_STATUS_INSTALL_SCHEDULED,
	OCPP_SECURITY_STATUS_INSTALL_VERIFICATION_FAILED,
	OCPP_SECURITY_STATUS_INVALID_SIGNATURE,
	OCPP_SECURITY_STATUS_SIGNATURE_VERIFIED,
} ocpp_security_status_t;

typedef enum {
	OCPP_SECURITY_EVENT_FIRMWARE_UPDATED,
	OCPP_SECURITY_EVENT_FAILED_AUTHENTICATE_AT_CSMS,
	OCPP_SECURITY_EVENT_CENTRAL_SYSTEM_FAILED_TO_AUTHENTICATE,
	OCPP_SECURITY_EVENT_SETTING_SYSTEM_TIME,
	OCPP_SECURITY_EVENT_STARTUP_DEVICE,
	OCPP_SECURITY_EVENT_REBOOT,
	OCPP_SECURITY_EVENT_LOG_CLEARED,
	OCPP_SECURITY_EVENT_PARAMETERS_UPDATED,
	OCPP_SECURITY_EVENT_MEMORY_EXHAUSTION,
	OCPP_SECURITY_EVENT_INVALID_MESSAGE,
	OCPP_SECURITY_EVENT_ATTEMPTED_REPLAY_ATTACK,
	OCPP_SECURITY_EVENT_TAMPER_DETECTED,
	OCPP_SECURITY_EVENT_INVALID_FIRMWARE_SIGNATURE,
	OCPP_SECURITY_EVENT_INVALID_FIRMWARE_SIGNING,
	OCPP_SECURITY_EVENT_INVALID_CSMS_CERTIFICATE,
	OCPP_SECURITY_EVENT_INVALID_CHARGE_POINT_CERTIFICATE,
	OCPP_SECURITY_EVENT_INVALID_TLS_VERSION,
	OCPP_SECURITY_EVENT_INVALID_TLS_CIPHER_SUITE,
} ocpp_security_event_t;

typedef enum {
	OCPP_SECURITY_CERT_TYPE_ROOT_CSMS,
	OCPP_SECURITY_CERT_TYPE_ROOT_MANUFACTURER,
} ocpp_security_cert_t;

struct ocpp_CertificateHashData {
	ocpp_hash_t hashAlgorithm;
	char issuerNameHash[128+1];
	char issuerKeyHash[128+1];
	char serialNumber[40+1];
};

struct ocpp_CertificateSigned {
	char dummy;
	char certificateChain[];
};

struct ocpp_CertificateSigned_conf {
	ocpp_security_status_t status;
};

struct ocpp_DeleteCertificate {
	struct ocpp_CertificateHashData certificateHashData;
};

struct ocpp_DeleteCertificate_conf {
	ocpp_security_status_t status;
};

struct ocpp_ExtendedTriggerMessage {
	ocpp_trigger_message_t requestedMessage;
	int connectorId;
};

struct ocpp_ExtendedTriggerMessage_conf {
	ocpp_trigger_status_t status;
};

struct ocpp_GetInstalledCertificateIds {
	ocpp_security_cert_t certificateType;
};

struct ocpp_GetInstalledCertificateIds_conf {
	ocpp_security_status_t status;
	struct ocpp_CertificateHashData certificateHashData;
};

struct ocpp_LogParameters {
	time_t oldestTimestamp;
	time_t latestTimestamp;
	char remoteLocation[];
};

struct ocpp_GetLog {
	ocpp_log_t logType;
	int requestId;
	int retries;
	int retryInterval;
	uint8_t log[];
};

struct ocpp_GetLog_conf {
	ocpp_security_status_t status;
	char filename[];
};

struct ocpp_InstallCertificate {
	ocpp_security_cert_t certificateType;
	char certificate[];
};

struct ocpp_InstallCertificate_conf {
	ocpp_security_status_t status;
};

struct ocpp_LogStatusNotification {
	ocpp_security_status_t status;
	int requestId;
};

struct ocpp_LogStatusNotification_conf {
	int none;
};

struct ocpp_SecurityEventNotification {
	ocpp_security_event_t type;
	time_t timestamp;
	char techInfo[];
};

struct ocpp_SecurityEventNotification_conf {
	int none;
};

struct ocpp_SignCertificate {
	char dummy;
	char csr[];
};

struct ocpp_SignCertificate_conf {
	ocpp_security_status_t status;
};

struct ocpp_SignedFirmwareStatusNotification {
	ocpp_security_status_t status;
	int requestId;
};

struct ocpp_SignedFirmwareStatusNotification_conf {
	int none;
};

struct ocpp_Firmware {
	time_t retrieveDateTime;
	time_t installDateTime;
	char *location;
	char *signature;
	char *signingCertificate;
};

struct ocpp_SignedUpdateFirmware {
	int retries;
	int retryInterval;
	int requestId;
	struct ocpp_Firmware firmware;
};

struct ocpp_SignedUpdateFirmware_conf {
	ocpp_security_status_t status;
};

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_OCPP_SECURITY_MESSAGES_H */
