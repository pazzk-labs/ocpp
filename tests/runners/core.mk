# SPDX-License-Identifier: MIT

COMPONENT_NAME = Core

SRC_FILES = \
	../src/ocpp.c \
	../src/memory_backend.c \
	../src/strconv.c \
	../src/core/configuration.c \
	../examples/messages.c \

TEST_SRC_FILES = \
	src/core_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	$(CPPUTEST_HOME)/include \
	../include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS = -DOCPP_DEFAULT_TX_TIMEOUT_SEC=5 -DOCPP_DEFAULT_TX_RETRIES=2 \
		    -DOCPP_DEBUG=printf -DOCPP_ERROR=printf -DOCPP_INFO=printf \
		    -include stdio.h

include runners/MakefileRunner
