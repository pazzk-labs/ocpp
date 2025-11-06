/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
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
#include "ocpp/backend.h"

/* If a response is received from the server, operate as defined in the
 * specification (drop immediately if it is not a transaction-related message).
 * If no response is received from the server, retry sending the message up to
 * `OCPP_DEFAULT_TX_RETRIES` times before discarding it. If set to 0, retry
 * indefinitely. */
#if !defined(OCPP_DEFAULT_TX_RETRIES)
#define OCPP_DEFAULT_TX_RETRIES			0
#endif

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

struct ocpp_message;

typedef void (*ocpp_event_callback_t)(ocpp_event_t event_type,
		const struct ocpp_message *message, void *ctx);

/**
 * @brief Callback function type for iterating over OCPP messages.
 *
 * This callback is used to process each OCPP message during iteration.
 * The function should return `true` to continue the iteration or `false`
 * to stop it.
 *
 * @param[in] msg Pointer to the current OCPP message being processed.
 * @param[in] ctx User-defined context passed to the callback.
 * @return `true` to continue iteration, `false` to stop.
 */
typedef bool (*ocpp_iterate_cb_t)(const struct ocpp_message *msg, void *ctx);

/**
 * @brief Initializes the OCPP module.
 *
 * This function initializes the OCPP (Open Charge Point Protocol) module and
 * sets up the event callback function that will be called for various OCPP
 * events. It also associates the backend and its context for communication.
 *
 * @param[in] backend Pointer to the OCPP backend structure.
 * @param[in] cb The callback function to handle OCPP events.
 * @param[in] cb_ctx A user-defined context that will be passed to the callback
 *                   function.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int ocpp_init(struct ocpp_backend *backend,
		ocpp_event_callback_t cb, void *cb_ctx);
void ocpp_deinit(void);

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
 * @brief Pushes a new OCPP request message.
 *
 * This function pushes a new OCPP request message of the specified type with
 * the given data. The user-defined context can be associated with the message
 * for later retrieval.
 *
 * @note The request will be stored in the backend queue for processing.
 *
 * @param[in] type The type of the OCPP message.
 * @param[in] data Pointer to the data to be included in the message.
 * @param[in] datasize Size of the data in bytes.
 * @param[in] ctx User-defined context to be associated with the message.
 *
 * @return Returns 0 if the request was successfully pushed, non-zero otherwise.
 */
int ocpp_push_request(ocpp_message_t type,
		const void *data, size_t datasize, void *ctx);

/**
 * @brief Pushes an OCPP request message for immediate processing.
 *
 * This function creates and pushes an OCPP request message of the specified
 * type for immediate processing. The message is not pushed to the backend
 * queue but is handled directly within the OCPP module.
 *
 * @param[in] type The type of the OCPP message.
 * @param[in] data Pointer to the data to be included in the message.
 * @param[in] datasize Size of the data in bytes.
 * @param[in] ctx User-defined context to be associated with the message.
 *
 * @return Returns 0 if the request was successfully processed,
 *         non-zero otherwise.
 */
int ocpp_push_request_front(ocpp_message_t type,
		const void *data, size_t datasize, void *ctx);

/**
 * @brief Pushes a deferred OCPP request.
 *
 * This function pushes an OCPP (Open Charge Point Protocol) request to be
 * deferred.
 *
 * @note The request will not be stored in the backend queue but will be
 *       processed directly after the specified timer expires.
 *
 * @note If the `timer_sec` parameter is set to 0, the request will be processed
 *       immediately without being stored in the backend queue.
 *
 * @param[in] type The type of the OCPP message.
 * @param[in] data Pointer to the data associated with the request.
 * @param[in] datasize Size of the data in bytes.
 * @param[in] timer_sec The timer duration in seconds after which the request
 *            will be processed. If set to 0, the request is processed
 *            immediately.
 * @param[in] ctx User-defined context to be associated with the message.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int ocpp_push_request_defer(ocpp_message_t type, const void *data,
		size_t datasize, uint32_t timer_sec, void *ctx);

/**
 * @brief Pushes an OCPP response.
 *
 * This function pushes a response to a previously received OCPP (Open Charge
 * Point Protocol) request.
 *
 * @note The response is not stored in the backend queue but is processed
 *       immediately in memory.
 *
 * @param[in] req Pointer to the original OCPP request message.
 * @param[in] data Pointer to the data associated with the response.
 * @param[in] datasize Size of the data in bytes.
 * @param[in] err Boolean flag indicating if the response is an error (true) or
 *            not (false).
 * @param[in] ctx User-defined context to be associated with the message.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int ocpp_push_response(const struct ocpp_message *req,
		const void *data, size_t datasize, bool err, void *ctx);

/**
 * @brief Sets the header fields for an OCPP message.
 *
 * This function initializes the header fields of the given OCPP message
 * with the specified role, message type, and unique identifier. It also
 * sets the current timestamp in the header.
 *
 * @note This function is intended to be called only from within the
 *       `ocpp_recv` function. Calling it from other contexts may lead
 *       to unexpected behavior.
 *
 * @param[in] msg Pointer to the OCPP message structure to be updated.
 * @param[in] role The role of the message.
 * @param[in] type The type of the OCPP message.
 * @param[in] id Pointer to the unique identifier for the message. If NULL,
 *               the identifier is not set.
 * @param[in] id_size The size of the unique identifier in bytes. Must not
 *                    exceed the maximum size of the message header ID field.
 *
 * @return 0 on success, or -EINVAL if the input parameters are invalid.
 */
int ocpp_set_message_header(struct ocpp_message *msg,
		ocpp_message_role_t role, ocpp_message_t type,
		const uint8_t *id, size_t id_size);

/**
 * @brief Copies a payload into an OCPP message.
 *
 * This function copies the provided payload data into the specified OCPP
 * message. The payload is dynamically allocated. If the payload already exists,
 * the function returns an error.
 *
 * @note This function is intended to be called only from within the
 *       `ocpp_recv` function. Calling it from other contexts may lead
 *       to unexpected behavior.
 *
 * @note The caller is responsible for managing the memory of the `data`
 *       pointer passed to this function. However, once the data is copied,
 *       the internal module takes ownership of the copied payload and will
 *       handle its deallocation. The caller does not need to manage the
 *       copied payload.
 *
 * @param[in] msg Pointer to the OCPP message structure to be updated.
 * @param[in] data Pointer to the payload data to be copied. If NULL, no data
 *             is copied, and the payload pointer is set to NULL.
 * @param[in] datasize The size of the payload data in bytes. If 0, no data is
 *                 copied, and the payload pointer is set to NULL.

 * @return 0 on success, -EINVAL if the input parameters are invalid,
 *         -EALREADY if the payload already exists, or -ENOMEM if memory
 *         allocation fails.
 */
int ocpp_copy_payload(struct ocpp_message *msg,
		const void *data, size_t datasize);

/**
 * @brief Reads the payload from an OCPP message.
 *
 * This function copies the payload data from the specified OCPP message
 * into the provided buffer.
 *
 * @param[in]  msg      Pointer to the OCPP message to read from.
 * @param[out] buf      Buffer to store the payload data.
 * @param[in]  bufsize  Size of the buffer in bytes.
 *
 * @return Number of bytes copied on success, or a negative error code:
 *         -EINVAL if parameters are invalid,
 *         -ENOSPC if the buffer is too small,
 *         -ENOENT if no payload exists.
 */
int ocpp_read_payload(const struct ocpp_message *msg,
		void *buf, size_t bufsize);

/**
 * @brief Retrieves an OCPP message by its ID.
 *
 * This function searches for and returns a pointer to the OCPP message
 * that matches the given message ID.
 *
 * @param id The ID of the message to retrieve.
 *
 * @return struct ocpp_message* Pointer to the OCPP message if found, otherwise
 *         NULL.
 */
struct ocpp_message *
ocpp_get_message_by_id(const char id[OCPP_MESSAGE_ID_MAXLEN]);

/**
 * @brief Counts the number of pending OCPP requests.
 *
 * This function returns the total number of OCPP (Open Charge Point Protocol)
 * requests that are currently pending and have not yet been processed.
 *
 * @return The number of pending OCPP requests.
 */
size_t ocpp_count_pending_requests(void);
size_t ocpp_count_stored_requests(void);

/**
 * @brief Get message type from ID string
 *
 * @param[in] idstr ID string
 *
 * @return Type of message. `OCPP_MSG_MAX` if no matching found.
 */
ocpp_message_t ocpp_get_type_from_idstr(const char *idstr);

/**
 * @brief Iterates over pending OCPP requests and applies a callback.
 *
 * This function traverses the list of pending OCPP requests and invokes
 * the provided callback function for each request. The callback can be
 * used to process or inspect the requests.
 *
 * @param[in] cb The callback function to apply to each pending request.
 * @param[in] ctx A user-defined context pointer passed to the callback.
 */
void ocpp_iterate_pending_requests(ocpp_iterate_cb_t cb, void *ctx);
void ocpp_iterate_stored_requests(ocpp_iterate_cb_t cb, void *ctx);

ocpp_message_t ocpp_get_message_type(const struct ocpp_message *msg);
ocpp_message_role_t ocpp_get_message_role(const struct ocpp_message *msg);
const char *ocpp_get_message_id(const struct ocpp_message *msg);
size_t ocpp_get_message_payload_size(const struct ocpp_message *msg);
void *ocpp_get_message_user_ctx(const struct ocpp_message *msg);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_OCPP_H */
