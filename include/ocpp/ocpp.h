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
#include "ocpp/security/messages.h"

#include "ocpp/overrides.h"

#if !defined(OCPP_DEFAULT_TX_TIMEOUT_SEC)
#define OCPP_DEFAULT_TX_TIMEOUT_SEC		10
#endif

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

/**
 * @brief Initializes the OCPP module.
 *
 * This function initializes the OCPP (Open Charge Point Protocol) module and
 * sets up the event callback function that will be called for various OCPP
 * events.
 *
 * @param[in] cb The callback function to handle OCPP events.
 * @param[in] cb_ctx A user-defined context that will be passed to the callback
 *            function.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int ocpp_init(ocpp_event_callback_t cb, void *cb_ctx);

/**
 * @brief Executes a single step of the OCPP state machine.
 *
 * This function performs one iteration of the OCPP (Open Charge Point Protocol)
 * state machine, processing any pending requests or responses and handling any
 * necessary state transitions.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int ocpp_step(void);

/**
 * @brief Pushes an OCPP request message.
 *
 * This function creates and pushes an OCPP request message of the specified
 * type.
 *
 * @param[in] type The type of the OCPP message to be pushed.
 * @param[in] data A pointer to the data to be included in the message.
 * @param[in] datasize The size of the data to be included in the message.
 * @param[out] created A pointer to a pointer to an ocpp_message structure
 *             where the created message will be stored.
 *
 * @return Returns 0 if the request was successfully pushed, non-zero
 *         otherwise.
 */
int ocpp_push_request(ocpp_message_t type, const void *data, size_t datasize,
		struct ocpp_message **created);

/**
 * @brief Pushes an OCPP request message forcefully.
 *
 * This function creates and pushes an OCPP request message of the specified
 * type, ensuring that the message is pushed even if the queue is full.
 *
 * @note The oldest request will be dropped if the queue is full set. If the
 *       oldest request is StartTransaction, StopTransaction or
 *       BootNotification, the next oldest request will be dropped.
 *
 * @param[in] type The type of the OCPP message to be pushed.
 * @param[in] data A pointer to the data to be included in the message.
 * @param[in] datasize The size of the data to be included in the message.
 * @param[out] created A pointer to a pointer to an ocpp_message structure
 *             where the created message will be stored.
 *
 * @return Returns 0 if the request was successfully pushed, non-zero
 *         otherwise.
 */
int ocpp_push_request_force(ocpp_message_t type, const void *data,
		size_t datasize, struct ocpp_message **created);

/**
 * @brief Pushes a deferred OCPP request.
 *
 * This function pushes an OCPP (Open Charge Point Protocol) request to be
 * deferred. The request will be processed after the specified timer expires.
 *
 * @param[in] type The type of the OCPP message.
 * @param[in] data Pointer to the data associated with the request.
 * @param[in] datasize Size of the data in bytes.
 * @param[in] timer_sec The timer duration in seconds after which the request
 *            will be processed.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int ocpp_push_request_defer(ocpp_message_t type,
		const void *data, size_t datasize, uint32_t timer_sec);

/**
 * @brief Pushes an OCPP response.
 *
 * This function pushes a response to a previously received OCPP (Open Charge
 * Point Protocol) request.
 *
 * @param[in] req Pointer to the original OCPP request message.
 * @param[in] data Pointer to the data associated with the response.
 * @param[in] datasize Size of the data in bytes.
 * @param[in] err Boolean flag indicating if the response is an error (true) or
 *            not (false).
 *
 * @return 0 on success, or a negative error code on failure.
 */
int ocpp_push_response(const struct ocpp_message *req,
		const void *data, size_t datasize, bool err);

/**
 * @brief Counts the number of pending OCPP requests.
 *
 * This function returns the total number of OCPP (Open Charge Point Protocol)
 * requests that are currently pending and have not yet been processed.
 *
 * @return The number of pending OCPP requests.
 */
size_t ocpp_count_pending_requests(void);

/**
 * @brief Converts an OCPP message type to its string representation.
 *
 * @param[in] msgtype The OCPP message type to be converted.
 *
 * @return A constant character pointer to the string representation of the
 * message type.
 */
const char *ocpp_stringify_type(ocpp_message_t msgtype);

/**
 * @brief Get message type from message type string
 *
 * @param[in] typestr The string representation of the OCPP message type.
 *
 * @return Type of message. `OCPP_MSG_MAX` if no matching found.
 */
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
