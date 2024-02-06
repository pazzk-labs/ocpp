/*
 * SPDX-FileCopyrightText: 2024 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef OCPP_H
#define OCPP_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <time.h>

#include "ocpp/list.h"
#include "ocpp/type.h"
#include "ocpp/overrides.h"

#include "ocpp/core/configuration.h"

typedef void (*ocpp_event_callback_t)(const struct ocpp_message *message,
		void *ctx);

struct ocpp_message {
	struct list link;
	char id[OCPP_MESSAGE_ID_MAXLEN];
	ocpp_message_role_t role;
	ocpp_message_t type;
	void *data;
	size_t datasize;
	time_t expiry;
	int attempts; /**< The number of message sending attempts. */
};

int ocpp_init(ocpp_event_callback_t cb, void *cb_ctx);
int ocpp_step(void);
int ocpp_push_message(ocpp_message_role_t role, ocpp_message_t type,
		const void *data, size_t datasize);
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

#if defined(__cplusplus)
}
#endif

#endif /* OCPP_H */
