# SPDX-License-Identifier: MIT

COMPONENT_NAME = Core

SRC_FILES = \
	../src/ocpp.c \
	../src/core/configuration.c \

TEST_SRC_FILES = \
	src/core_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	$(CPPUTEST_HOME)/include \
	../include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS = -DOCPP_DEFAULT_TX_TIMEOUT_SEC=5 -DOCPP_DEFAULT_TX_RETRIES=2

include runners/MakefileRunner
