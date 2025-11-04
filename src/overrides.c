/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#include "ocpp/overrides.h"
#include <time.h>
#include <stdio.h>

void __attribute__((weak)) ocpp_generate_message_id(void *buf, size_t bufsize)
{
	snprintf(buf, bufsize, "%lu", time(NULL));
}

bool __attribute__((weak))
ocpp_is_message_droppable(const struct ocpp_message *msg)
{
	(void)msg;
	return true;
}
