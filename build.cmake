# SPDX-License-Identifier: MIT

list(APPEND OCPP_SRCS
	${CMAKE_CURRENT_LIST_DIR}/src/ocpp.c
	${CMAKE_CURRENT_LIST_DIR}/src/core/configuration.c
	${CMAKE_CURRENT_LIST_DIR}/src/stringify.c
)
list(APPEND OCPP_INCS ${CMAKE_CURRENT_LIST_DIR}/include)
