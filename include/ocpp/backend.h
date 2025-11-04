/*
 * SPDX-FileCopyrightText: 2025 권경환 Kyunghwan Kwon <k@pazzk.net>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef PAZZK_OCPP_BACKEND_H
#define PAZZK_OCPP_BACKEND_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "ocpp/type.h"

struct ocpp_backend_message_header {
	char id[OCPP_MESSAGE_ID_MAXLEN];
	uint8_t role; /* ocpp_message_role_t */
	uint8_t type; /* ocpp_message_t */
	uint8_t padding;
	int64_t timestamp;
#if __SIZEOF_POINTER__ == 8
	uint64_t custom; /* user custom data */
#elif __SIZEOF_POINTER__ == 4
	uint32_t custom; /* user custom data */
#else
#error "Unsupported pointer size"
#endif
} __attribute__((packed));

struct ocpp_backend_message {
	struct ocpp_backend_message_header header;
	uint32_t payload_size;
	uint8_t payload[];
} __attribute__((packed));

/**
 * @brief Callback function type for iterating over backend messages.
 *
 * This callback is used to process each backend message during iteration.
 * The function should return `true` to continue the iteration or `false`
 * to stop it.
 *
 * @param[in] msg Pointer to the current backend message being processed.
 * @param[in] ctx User-defined context passed to the callback.
 * @return `true` to continue iteration, `false` to stop.
 */
typedef bool (*ocpp_backend_foreach_cb_t)
		(const struct ocpp_backend_message *msg, void *ctx);

struct ocpp_backend;
struct ocpp_backend_api {
	int (*push)(struct ocpp_backend *self,
			const struct ocpp_backend_message *msg);
	int (*push_front)(struct ocpp_backend *self,
			const struct ocpp_backend_message *msg);
	int (*pop)(struct ocpp_backend *self,
			struct ocpp_backend_message *buf, size_t bufsize);
	int (*peek)(struct ocpp_backend *self,
			struct ocpp_backend_message *buf, size_t bufsize);
	int (*peek_payload)(struct ocpp_backend *self,
			uint8_t *buf, size_t bufsize);
	int (*drop)(struct ocpp_backend *self, size_t nr_msgs);
	int (*clear)(struct ocpp_backend *self);
	size_t (*count)(const struct ocpp_backend *self);
	int (*foreach)(struct ocpp_backend *self,
			ocpp_backend_foreach_cb_t cb, void *cb_ctx);
};

#if defined(__cplusplus)
}
#endif

#endif /* PAZZK_OCPP_BACKEND_H */
