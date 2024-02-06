# SPDX-License-Identifier: MIT

COMPONENT_NAME = Configuration

SRC_FILES = \
	../src/core/configuration.c \

TEST_SRC_FILES = \
	src/configuration_test.cpp \
	src/test_all.cpp \

INCLUDE_DIRS = \
	$(CPPUTEST_HOME)/include \
	../include \

MOCKS_SRC_DIRS =
CPPUTEST_CPPFLAGS =

include runners/MakefileRunner
