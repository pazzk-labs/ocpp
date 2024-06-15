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

enum ocpp_event {
	OCPP_EVENT_MESSAGE_INCOMING,
	OCPP_EVENT_MESSAGE_OUTGOING,
	OCPP_EVENT_MESSAGE_FREE,
	/* negative for errors */
};

typedef int ocpp_event_t;
typedef void (*ocpp_event_callback_t)(ocpp_event_t event_type,
		const struct ocpp_message *message, void *ctx);

struct ocpp_message {
	char id[OCPP_MESSAGE_ID_MAXLEN];
	ocpp_message_role_t role;
	ocpp_message_t type;

	struct {
		union {
			const void *request;
			const void *response;
			void *data;
		} fmt;
		size_t size;
	} payload;
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
