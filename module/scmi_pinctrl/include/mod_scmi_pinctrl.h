/*
 * Arm SCP/MCP Software - SCMI pin control module
 * Copyright (c) 2024, Linaro Limited. All rights reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI Pin Control Protocol Support.
 */

#ifndef MOD_SCMI_PINCTRL_H
#define MOD_SCMI_PINCTRL_H

#include <fwk_id.h>
#include <stddef.h>

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
    const uint16_t *pins;
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
    const uint16_t *groups;
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

struct scmi_pinctrl_driver_api {
    /* init */
    uint32_t (*init)(unsigned int drv_id);
    uint32_t (*fini)(unsigned int drv_id);

    /* pin multiplexing */
    uint32_t (*set_function)(void);

    /* pin configuration */
    uint32_t (*set_config)(void);

    /* gpio specific */
    uint32_t (*set_direction)(void);
    uint32_t (*set_value)(void);
    uint32_t (*get_value)(void);
};

struct scmi_pinctrl_driver_data {
    struct scmi_pinctrl_driver_api *api;
};

struct mod_scmi_pinctrl_config {
    const struct scmi_pinctrl_pin_data *pin_table;
    unsigned int pin_count;
    const struct scmi_pinctrl_group_data *group_table;
    unsigned int group_count;
    const struct scmi_pinctrl_function_data *function_table;
    unsigned int function_count;

    const struct scmi_pinctrl_driver_data *driver_table;
    unsigned int driver_count;

    unsigned int config_count;
};

#endif /* MOD_SCMI_PINCTRL_H */
