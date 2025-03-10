/*
 * SPDX-FileCopyrightText: 2024 권경환 Kyunghwan Kwon <k@libmcu.org>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_OCPP_CONFIGURATION_SIZE_H
#define LIBMCU_OCPP_CONFIGURATION_SIZE_H

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(OCPP_CONFIGURATION_DEFINES)
#define OCPP_CONFIGURATION_DEFINES	"ocpp_configuration.def.template"
#endif

#define CONF_SIZE(x)			(x)
#define BOOL				CONF_SIZE(sizeof(bool))
#define INT				CONF_SIZE(sizeof(int))
#define CSL				CONF_SIZE(sizeof(int))
#define STR				CONF_SIZE

#define OCPP_CONFIG(key, accessbility, type, default_value)	+ type
enum {
	OCPP_CONFIGURATION_TOTAL_SIZE = 0
#include OCPP_CONFIGURATION_DEFINES
};
#undef OCPP_CONFIG

#undef STR
#undef CSL
#undef INT
#undef BOOL
#undef CONF_SIZE

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_OCPP_CONFIGURATION_SIZE_H */
