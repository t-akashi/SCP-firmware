
/*
 * Arm SCP/MCP Software - SCMI pin control module
 * Copyright (c) 2024, Linaro Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI Pin Control Protocol Support
 */

#ifndef INTERNAL_SCMI_PINCTRL_H
#define INTERNAL_SCMI_PINCTRL_H

#include <stdint.h>

#define SCMI_PROTOCOL_VERSION_PIN_CONTROL UINT32_C(0x10000)

struct scmi_pinctrl_config_state {
    uint32_t type;
    uint32_t value;
};

#define SCMI_PINCTRL_PIN_STATE_RELEASED 0

#define SCMI_PINCTRL_NO_OWNER		0xffff
#define SCMI_PINCTRL_NO_FUNCTION	0xffff

struct scmi_pinctrl_pingrp_state {
    unsigned int state;
    uint32_t owner_agent;
    uint32_t function;
    struct scmi_pinctrl_config_state *config;

    struct mod_scmi_pinctrl_driver_api *api;
};

struct scmi_pinctrl_context {
    struct scmi_pinctrl_pingrp_state *pin_state;
    struct scmi_pinctrl_pingrp_state *group_state;
};

/*
 * Identifiers for the type of request being processed
 */
enum scmi_pinctrl_request_type {
    SCMI_PINCTRL_REQUEST_COUNT,
};

/*
 * Identifiers of the internal events
 */
enum scmi_pinctrl_event_idx {
    SCMI_PINCTRL_EVENT_IDX_COUNT,
};

/*
 * Common for messages
 */
#define SCMI_PINCTRL_IDENTIFIER_MASK	UINT32_C(0xffff)

#define SCMI_PINCTRL_SELECTOR_MASK	UINT32_C(0x3)
#define SCMI_PINCTRL_SELECTOR_PIN	UINT32_C(0x0)
#define SCMI_PINCTRL_SELECTOR_GROUP	UINT32_C(0x1)
#define SCMI_PINCTRL_SELECTOR_FUNCTION	UINT32_C(0x2)

/* 
 * Protocol Attributes
 */
#define SCMI_PINCTRL_PROTOCOL_ATTRIBUTES_GROUP_POS 16
struct scmi_pinctrl_protocol_attributes_p2a {
    int32_t status;
    uint32_t attributes_low;
    uint32_t attributes_high;
};

/*
 * Pinctrl Attributes
 */
struct scmi_pinctrl_attributes_a2p {
    uint32_t identifier;
    uint32_t flags;
};

#define SCMI_PINCTRL_EXTENDED_NAME_POS 31
#define SCMI_PINCTRL_EXTENDED_NAME_MASK \
    (UINT32_C(0x1) << SCMI_PINCTRL_EXTENDED_NAME_POS)
#define SCMI_PINCTRL_ATTR_NUM_PINS_MASK UINT32_C(0xffff)

#define SCMI_PINCTRL_NAME_LENGTH_MAX 16

struct scmi_pinctrl_attributes_p2a {
    int32_t status;
    uint32_t attributes;
    uint8_t name[SCMI_PINCTRL_NAME_LENGTH_MAX];
};

/*
 * Pinctrl List Associations
 */
struct scmi_pinctrl_list_assoc_a2p {
    uint32_t identifier;
    uint32_t flags;
    uint32_t index;
};

#define SCMI_PINCTRL_NUM_REMAININGS_POS 16
#define SCMI_PINCTRL_NUM_REMAININGS_MASK \
    (UINT32_C(0xffff) << SCMI_PINCTRL_NUM_REMAININGS_POS)
#define SCMI_PINCTRL_LIST_NUM_PINS_MASK UINT32_C(0xfff)

struct scmi_pinctrl_list_assoc_p2a {
    int32_t status;
    uint32_t flags;
    uint16_t array[];
};

/*
 * Pinctrl Settings Get
 */
#define SCMI_PINCTRL_GET_CONFIG_FLAG_POS 18
#define SCMI_PINCTRL_GET_ALL_CONFIGS \
    (UINT32_C(0x1) << SCMI_PINCTRL_GET_CONFIG_FLAG_POS)
#define SCMI_PINCTRL_GET_FUNCTION \
    (UINT32_C(0x2) << SCMI_PINCTRL_GET_CONFIG_FLAG_POS)
#define SCMI_PINCTRL_GET_SELECTOR_POS 16
#define SCMI_PINCTRL_GET_SELECTOR_MASK \
    (UINT32_C(0x3) << SCMI_PINCTRL_GET_SELECTOR_POS)
#define SCMI_PINCTRL_GET_SKIP_CONFIGS_POS 8
#define SCMI_PINCTRL_GET_SKIP_CONFIGS_MASK \
    (UINT32_C(0xff) << SCMI_PINCTRL_GET_SKIP_CONFIGS_POS)

struct scmi_pinctrl_settings_get_a2p {
    uint32_t identifier;
    uint32_t attributes;
};

#define SCMI_PINCTRL_GET_NO_FUNCTION UINT32_C(0xffffffff)
#define SCMI_PINCTRL_GET_NUM_REM_CONFIGS_POS 24
#define SCMI_PINCTRL_GET_NUM_REM_CONFIGS_MASK \
    (UINT32_C(0xffff) << SCMI_PINCTRL_GET_NUM_REM_CONFIGS_POS)
#define SCMI_PINCTRL_GET_NUM_CONFIGS_MASK UINT32_C(0xffff)

#define SCMI_PINCTRL_GET_CONFIG_TYPE_MASK UINT32_C(0xff)

struct scmi_pinctrl_settings_get_p2a {
    int32_t status;
    uint32_t function_selected;
    uint32_t num_configs;
    uint32_t configs[][2];
};

/*
 * Pinctrl Settings Configure
 */
#define SCMI_PINCTRL_SET_FUNCTION_VALID_POS 10
#define SCMI_PINCTRL_SET_FUNCTION_VALID \
    (UINT2_C(0x1) << SCMI_PINCTRL_SET_FUNCTION_VALID)
#define SCMI_PINCTRL_SET_NUM_CONFIGS_POS 2
#define SCMI_PINCTRL_SET_NUM_CONFIGS_MASK \
    (UINT32_C(0xffff) << SCMI_PINCTRL_SET_NUM_CONFIGS_POS)

#define SCMI_PINCTRL_SET_CONFIG_TYPE_MASK UINT32_C(0xff)

struct scmi_pinctrl_settings_configure_a2p {
    uint32_t identifier;
    uint32_t function_id;
    uint32_t attributes;
    uint32_t configs[][2];
};

/*
 * Pinctrl Request
 */
struct scmi_pinctrl_request_a2p {
    uint32_t identifier;
    uint32_t flags;
};

/*
 * Pinctrl Release
 */
struct scmi_pinctrl_release_a2p {
    uint32_t identifier;
    uint32_t flags;
};

/*
 * Pinctrl Name Get
 */
struct scmi_pinctrl_name_get_a2p {
    uint32_t identifier;
    uint32_t flags;
};

#define SCMI_PINCTRL_EXTENDED_NAME_LENGTH_MAX 64

struct scmi_pinctrl_name_get_p2a {
    int32_t status;
    uint32_t flags;
    uint8_t name[SCMI_PINCTRL_EXTENDED_NAME_LENGTH_MAX];
};

/*
 * Pinctrl Set Permissions
 */

#define SCMI_PINCTRL_PERMISSION_POS 2
#define SCMI_PINCTRL_PPERMISSION_ALLOW \
    (UINT32_C(0x1) << SCMI_PINCTRL_PERMISSION_POS)

struct scmi_pinctrl_set_permissions_a2p {
    uint32_t agent_id;
    uint32_t identifier;
    uint32_t flags;
};


#endif /* INTERNAL_SCMI_PINCTRL_H */
#if 0
#define SCMI_PINCTRL_CONFIG_COUNT 0x20
#define SCMI_PINCTRL_NAME_LEN 16
#define SCMI_PINCTRL_EXTENDED_NAME_LEN 64

/*
 * External data
 */
struct scmi_pinctrl_pin_data {
    const char *name;
    unsigned int drv_id;
    uint32_t agent_permission;
};

#define SCMI_PINCTRL_PIN(pin, drv, permission)	\
    {				   		\
	.name = #pin,		   		\
	.drv_id = drv,		   		\
	.agent_permission = permission,		\
    }

struct scmi_pinctrl_group_data {
    const char *name;
    uint32_t pins;
    size_t pins_count;
    uint32_t agent_permission;
};

#define SCMI_PINCTRL_GROUP(group, permission)		\
    {							\
	.name = #group,					\
	.pins = group ## _pins,				\
	.pins_count = FWK_ARRAY_SIZE(group ## _pins),	\
	.agent_permission = permission,			\
    }

struct scmi_pinctrl_function_data {
    const char *name;
    uint32_t *groups;
    size_t groups_count;
    uint32_t agent_permission;
};

#define SCMI_PINCTRL_FUNC(function, permission)		\
    {							\
	.name = #function,				\
	.groups = function,				\
	.groups_count = FWK_ARRAY_SIZE(function),	\
	.agent_permission = permission,			\
    }

struct mod_scmi_pinctrl_driver_api {
    /* init */
    uint32_t init(unsigned int drv_id);
    uint32_t fini(unsigned int drv_id);

    /* pin multiplexing */
    uint32_t (*set_function)();

    /* pin configuration */
    uint32_t (*set_config)();

    /* gpio specific */
    uint32_t (*set_direction)();
    uint32_t (*set_value)();
    uint32_t (*get_value)();
};

struct scmi_pinctrl_driver_data {
    struct scmi_pinctrl_driver_api *api;
};

struct scmi_pinctrl_config {
    struct scmi_pinctrl_pin_data *pin_table;
    size_t pin_count;
    struct scmi_pinctrl_group_data *group_table;
    size_t group_count;
    struct scmi_pinctrl_function_data *function_table;
    size_t function_count;

    struct scmi_pinctrl_driver_data *driver_table;
    size_t driver_count;

    unsigned int config_count;
};
#endif
