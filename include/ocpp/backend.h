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
	uint32_t custom; /* user custom data */
} __attribute__((packed));

struct ocpp_backend_message {
	struct ocpp_backend_message_header header;
	uint32_t payload_size;
	uint8_t payload[];
} __attribute__((packed));

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
