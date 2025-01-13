# SPDX-License-Identifier: MIT

ifneq ($(OCPP_ROOT),)
ocpp-basedir := $(OCPP_ROOT)/
endif

OCPP_SRCS := \
	$(ocpp-basedir)src/ocpp.c \
	$(ocpp-basedir)src/core/configuration.c \
	$(ocpp-basedir)src/stringify.c \

OCPP_INCS := $(ocpp-basedir)include
