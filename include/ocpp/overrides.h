/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_OCPP_OVERRIDES_H
#define LIBMCU_OCPP_OVERRIDES_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>

struct ocpp_message;

/**
 * @brief Sends an OCPP message.
 *
 * @param[in] msg A pointer to the OCPP message to be sent.
 * @return An integer indicating the status of the operation. A return value
 *         of 0 indicates success, while any other value indicates an error.
 */
int ocpp_send(const struct ocpp_message *msg);

/**
 * @brief Receives an OCPP message.
 *
 * This function receives an OCPP message from the server. The received message
 * is stored in the provided ocpp_message structure.
 *
 * If the function returns -ENOTSUP, it sends an error response message. In the
 * case of an ENOTSUP error, the last received message time is recorded. For
 * other errors, the time is not recorded.
 *
 * @param[out] msg A pointer to the ocpp_message structure where the received
 *             message will be stored.
 * @return An integer indicating the status of the operation. A return value
 *         of 0 indicates success, while any other value indicates an error.
 */
int ocpp_recv(struct ocpp_message *msg);

/**
 * @brief Generates a unique message ID.
 *
 * This function generates a unique message ID for an OCPP message. The
 * generated ID is stored in the provided buffer.
 *
 * @param[in] buf A pointer to the buffer where the generated ID will be stored.
 * @param[in] bufsize The size of the buffer.
 */
void ocpp_generate_message_id(void *buf, size_t bufsize);

/**
 * @brief Acquires a lock for OCPP operations.
 *
 * @return An integer indicating the status of the operation. A return value
 *         of 0 indicates that the lock was successfully acquired, while any
 *         other value indicates an error.
 */
int ocpp_lock(void);
int ocpp_unlock(void);

/**
 * @brief Acquires a lock for OCPP configuration operations.
 *
 * @return An integer indicating the status of the operation. A return value
 *         of 0 indicates that the lock was successfully acquired, while any
 *         other value indicates an error.
 */
int ocpp_configuration_lock(void);
int ocpp_configuration_unlock(void);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_OCPP_OVERRIDES_H */
