#
# Arm SCP/MCP Software, SCMI pinctrl
# Copyright (c) 2023, Linaro Limited
#
# SPDX-License-Identifier: BSD-3-Clause
#

add_library(${SCP_MODULE_TARGET} SCP_MODULE)

target_include_directories(${SCP_MODULE_TARGET}
                           PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_sources(${SCP_MODULE_TARGET}
               PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/mod_scmi_pinctrl.c")

target_link_libraries(
    module-scmi-pinctrl
    PUBLIC module-pinctrl
    PRIVATE module-scmi)

if("resource-perms" IN_LIST SCP_MODULES)
    target_link_libraries(${SCP_MODULE_TARGET} PRIVATE module-resource-perms)
endif()
