# SPDX-License-Identifier: MIT

PROJECT := ocpp
BASEDIR := $(shell pwd)
BUILDIR := build

VERBOSE ?= 0
V ?= $(VERBOSE)
ifeq ($(V), 0)
	Q = @
else
	Q =
endif
export PROJECT
export BASEDIR
export BUILDIR
export Q

.PHONY: confirm
confirm:
	@echo 'Are you sure? [y/N] ' && read ans && [ $${ans:-N} = y ]

## help: print this help message
.PHONY: help
help:
	@echo 'Usage:'
	@sed -n 's/^##//p' ${MAKEFILE_LIST} | column -t -s ':' |  sed -e 's/^/ /'

## test
.PHONY: test
test:
	$(Q)$(MAKE) -C tests
## coverage
.PHONY: coverage
coverage:
	$(Q)$(MAKE) -C tests $@
## clean
.PHONY: clean
clean:
	$(Q)rm -fr $(BUILDIR)
	$(Q)$(MAKE) -C tests $@
## version: print firmware version
.PHONY: version
version:
	$(info $(VERSION_TAG), $(VERSION))
