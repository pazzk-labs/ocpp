/*
 * SPDX-FileCopyrightText: 2025 권경환 Kyunghwan Kwon <k@pazzk.net>
 *
 * SPDX-License-Identifier: MIT
 */

#include "ocpp/memory_backend.h"
#include "ocpp/list.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

struct ocpp_backend {
	struct ocpp_backend_api api;
	struct list list;
	pthread_mutex_t mutex;
};

struct msg_container {
	struct list link;
	struct ocpp_backend_message msg;
};

static int do_push(struct ocpp_backend *self,
		const struct ocpp_backend_message *msg) {
	struct msg_container *container = (struct msg_container *)
		malloc(sizeof(struct msg_container) + msg->payload_size);
	if (!container) {
		return -ENOMEM;
	}
	memcpy(&container->msg, msg, sizeof(struct ocpp_backend_message) +
			msg->payload_size);
	pthread_mutex_lock(&self->mutex);
	list_add_tail(&container->link, &self->list);
	pthread_mutex_unlock(&self->mutex);
	return 0;
}

static int do_push_front(struct ocpp_backend *self,
		const struct ocpp_backend_message *msg) {
	struct msg_container *container = (struct msg_container *)
		malloc(sizeof(struct msg_container) + msg->payload_size);
	if (!container) {
		return -ENOMEM;
	}
	memcpy(&container->msg, msg, sizeof(struct ocpp_backend_message) +
			msg->payload_size);
	pthread_mutex_lock(&self->mutex);
	list_add(&container->link, &self->list);
	pthread_mutex_unlock(&self->mutex);
	return 0;
}

static int do_pop(struct ocpp_backend *self,
		struct ocpp_backend_message *buf, size_t bufsize) {
	pthread_mutex_lock(&self->mutex);
	if (list_empty(&self->list)) {
		pthread_mutex_unlock(&self->mutex);
		return -ENOENT;
	}
	struct msg_container *container = list_entry(list_first(&self->list),
			struct msg_container, link);
	if (bufsize < sizeof(struct ocpp_backend_message) +
			container->msg.payload_size) {
		pthread_mutex_unlock(&self->mutex);
		return -ENOSPC;
	}
	memcpy(buf, &container->msg, sizeof(struct ocpp_backend_message) +
			container->msg.payload_size);
	list_del(&container->link, &self->list);
	pthread_mutex_unlock(&self->mutex);
	free(container);
	return 0;
}

static int do_peek(struct ocpp_backend *self,
		struct ocpp_backend_message *buf, size_t bufsize) {
	pthread_mutex_lock(&self->mutex);
	if (list_empty(&self->list)) {
		pthread_mutex_unlock(&self->mutex);
		return -ENOENT;
	}
	const struct msg_container *container =
		list_entry(list_first(&self->list), struct msg_container, link);
	if (bufsize < sizeof(struct ocpp_backend_message)) {
		pthread_mutex_unlock(&self->mutex);
		return -ENOSPC;
	}
	memcpy(buf, &container->msg, sizeof(struct ocpp_backend_message));
	pthread_mutex_unlock(&self->mutex);
	return 0;
}

static int do_peek_payload(struct ocpp_backend *self,
		uint8_t *buf, size_t bufsize) {
	pthread_mutex_lock(&self->mutex);
	if (list_empty(&self->list)) {
		pthread_mutex_unlock(&self->mutex);
		return -ENOENT;
	}
	const struct msg_container *container =
		list_entry(list_first(&self->list), struct msg_container, link);
	if (bufsize < container->msg.payload_size) {
		pthread_mutex_unlock(&self->mutex);
		return -ENOSPC;
	}
	memcpy(buf, container->msg.payload, container->msg.payload_size);
	pthread_mutex_unlock(&self->mutex);
	return (int)container->msg.payload_size;
}

static int do_drop(struct ocpp_backend *self, size_t nr_msgs) {
	size_t dropped = 0;
	pthread_mutex_lock(&self->mutex);
	while (dropped < nr_msgs && !list_empty(&self->list)) {
		struct msg_container *container = list_entry(
				list_first(&self->list),
				struct msg_container, link);
		list_del(&container->link, &self->list);
		free(container);
		dropped++;
	}
	pthread_mutex_unlock(&self->mutex);
	return (int)dropped;
}

static int do_clear(struct ocpp_backend *self) {
	pthread_mutex_lock(&self->mutex);
	while (!list_empty(&self->list)) {
		struct msg_container *container = list_entry(
				list_first(&self->list),
				struct msg_container, link);
		list_del(&container->link, &self->list);
		free(container);
	}
	pthread_mutex_unlock(&self->mutex);
	return 0;
}

static size_t do_count(struct ocpp_backend *self) {
	size_t cnt = 0;
	pthread_mutex_lock(&self->mutex);
	cnt = (size_t)list_count(&self->list);
	pthread_mutex_unlock(&self->mutex);
	return cnt;
}

static int do_foreach(struct ocpp_backend *self,
		ocpp_backend_foreach_cb_t cb, void *cb_ctx) {
	int rc = 0;
	struct list *p;
	struct list *n;
	pthread_mutex_lock(&self->mutex);
	list_for_each_safe(p, n, &self->list) {
		const struct msg_container *container =
			list_entry(p, struct msg_container, link);
		pthread_mutex_unlock(&self->mutex);
		bool stop = !(*cb)(&container->msg, cb_ctx);
		pthread_mutex_lock(&self->mutex);
		if (stop) {
			rc = -EINTR;
			break;
		}
	}
	pthread_mutex_unlock(&self->mutex);
	return rc;
}

struct ocpp_backend *ocpp_memory_backend_create(void)
{
	struct ocpp_backend *backend;

	backend = (struct ocpp_backend *)calloc(1, sizeof(*backend));
	if (!backend) {
		return NULL;
	}

	backend->api = (struct ocpp_backend_api) {
		.push = do_push,
		.push_front = do_push_front,
		.pop = do_pop,
		.peek = do_peek,
		.peek_payload = do_peek_payload,
		.drop = do_drop,
		.clear = do_clear,
		.count = do_count,
		.foreach = do_foreach,
	};

	list_init(&backend->list);
	pthread_mutex_init(&backend->mutex, NULL);

	return backend;
}

void ocpp_memory_backend_destroy(struct ocpp_backend *backend)
{
	if (!backend) {
		return;
	}
	do_clear(backend);
	pthread_mutex_destroy(&backend->mutex);
	free(backend);
}
