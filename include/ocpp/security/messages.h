/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
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
} ocpp_security_status_t;

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
	char certificateChain[0];
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
	char remoteLocation[0];
};

struct ocpp_GetLog {
	ocpp_log_t logType;
	int requestId;
	int retries;
	int retryInterval;
	struct ocpp_LogParameters log;
};

struct ocpp_GetLog_conf {
	ocpp_security_status_t status;
	char filename[255+1];
};

struct ocpp_InstallCertificate {
	ocpp_security_cert_t certificateType;
	char certificate[0];
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
	char type[50+1];
	time_t timestamp;
	char techInfo[255+1];
};

struct ocpp_SecurityEventNotification_conf {
	int none;
};

struct ocpp_SignCertificate {
	char csr[0];
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
