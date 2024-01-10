/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Linaro Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCMI Pin Control Protocol Support.
 */

#include <fwk_assert.h>
#include <fwk_attributes.h>
#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <fwk_string.h>
#include <mod_scmi.h>
#include <mod_scmi_pinctrl.h>
#include <stdlib.h>
#include <internal/scmi.h>
#include <internal/scmi_pinctrl.h>

/*
 * SCMI Pin Control Message Handlers
 */
static int scmi_pinctrl_protocol_version_handler(fwk_id_t service_id,
						 const uint32_t *payload);
static int scmi_pinctrl_negotiate_protocol_version_handler(
		fwk_id_t service_id, const uint32_t *payload);
static int scmi_pinctrl_protocol_attributes_handler(fwk_id_t service_id,
						    const uint32_t *payload);
static int scmi_pinctrl_protocol_message_attributes_handler(
	fwk_id_t service_id, const uint32_t *payload);
static int scmi_pinctrl_attributes_handler(fwk_id_t service_id,
					   const uint32_t *payload);
static int scmi_pinctrl_list_assoc_handler(fwk_id_t service_id,
					   const uint32_t *payload);
static int scmi_pinctrl_settings_get_handler(fwk_id_t service_id,
					     const uint32_t *payload);
static int scmi_pinctrl_settings_configure_handler(fwk_id_t service_id,
						   const uint32_t *payload);
static int scmi_pinctrl_request_handler(fwk_id_t service_id,
					const uint32_t *payload);
static int scmi_pinctrl_release_handler(fwk_id_t service_id,
					const uint32_t *payload);
static int scmi_pinctrl_name_get_handler(fwk_id_t service_id,
					 const uint32_t *payload);
static int scmi_pinctrl_set_permissions_handler(fwk_id_t service_id,
						const uint32_t *payload);

/*
 * Internal variables.
 */
static struct mod_scmi_from_protocol_api *scmi_api;

static struct mod_scmi_pinctrl_config scmi_pinctrl_config;
static struct scmi_pinctrl_context scmi_pinctrl_ctx;

static int (*const handler_table[MOD_SCMI_PIN_CONTROL_COMMAND_COUNT])(
		fwk_id_t, const uint32_t *) = {
    [MOD_SCMI_PROTOCOL_VERSION] = scmi_pinctrl_protocol_version_handler,
    [MOD_SCMI_NEGOTIATE_PROTOCOL_VERSION] =
	scmi_pinctrl_negotiate_protocol_version_handler,
    [MOD_SCMI_PROTOCOL_ATTRIBUTES] = scmi_pinctrl_protocol_attributes_handler,
    [MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
        scmi_pinctrl_protocol_message_attributes_handler,
    [MOD_SCMI_PIN_CONTROL_ATTRIBUTES] = scmi_pinctrl_attributes_handler,
    /* TODO: variable size due to a trailing list */
    [MOD_SCMI_PIN_CONTROL_LIST_ASSOCIATIONS] = scmi_pinctrl_list_assoc_handler,
    [MOD_SCMI_PIN_CONTROL_SETTINGS_GET] = scmi_pinctrl_settings_get_handler,
    [MOD_SCMI_PIN_CONTROL_SETTINGS_CONFIGURE] =
	scmi_pinctrl_settings_configure_handler,
    [MOD_SCMI_PIN_CONTROL_REQUEST] = scmi_pinctrl_request_handler,
    [MOD_SCMI_PIN_CONTROL_RELEASE] = scmi_pinctrl_release_handler,
    [MOD_SCMI_PIN_CONTROL_NAME_GET] = scmi_pinctrl_name_get_handler,
    [MOD_SCMI_PIN_CONTROL_SET_PERMISSIONS] = 
	scmi_pinctrl_set_permissions_handler,
};

static const
unsigned int payload_size_table[MOD_SCMI_PIN_CONTROL_COMMAND_COUNT] = {
    [MOD_SCMI_PROTOCOL_VERSION] = 0,
    [MOD_SCMI_NEGOTIATE_PROTOCOL_VERSION] =
        (unsigned int)sizeof(struct scmi_negotiate_protocol_version_a2p),
    [MOD_SCMI_PROTOCOL_ATTRIBUTES] = 0,
    [MOD_SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
        (unsigned int)sizeof(struct scmi_protocol_message_attributes_a2p),
    [MOD_SCMI_PIN_CONTROL_ATTRIBUTES] =
        (unsigned int)sizeof(struct scmi_pinctrl_attributes_a2p),
    [MOD_SCMI_PIN_CONTROL_LIST_ASSOCIATIONS] =
        (unsigned int)sizeof(struct scmi_pinctrl_list_assoc_a2p),
    [MOD_SCMI_PIN_CONTROL_SETTINGS_GET] =
        (unsigned int)sizeof(struct scmi_pinctrl_settings_get_a2p),
    [MOD_SCMI_PIN_CONTROL_SETTINGS_CONFIGURE] =
        (unsigned int)sizeof(struct scmi_pinctrl_settings_configure_a2p),
    [MOD_SCMI_PIN_CONTROL_REQUEST] =
        (unsigned int)sizeof(struct scmi_pinctrl_request_a2p),
    [MOD_SCMI_PIN_CONTROL_RELEASE] =
        (unsigned int)sizeof(struct scmi_pinctrl_release_a2p),
    [MOD_SCMI_PIN_CONTROL_NAME_GET] =
        (unsigned int)sizeof(struct scmi_pinctrl_name_get_a2p),
    [MOD_SCMI_PIN_CONTROL_SET_PERMISSIONS] =
        (unsigned int)sizeof(struct scmi_pinctrl_set_permissions_a2p),
};

/*
 * Protocol Version
 */
static int scmi_pinctrl_protocol_version_handler(fwk_id_t service_id,
						 const uint32_t *payload)
{
    struct scmi_protocol_version_p2a return_values = {
        .status = (int32_t)SCMI_SUCCESS,
        .version = SCMI_PROTOCOL_VERSION_PIN_CONTROL,
    };

    return scmi_api->respond(service_id, &return_values, sizeof(return_values));
}

/*
 * Negotiate Protocol Version
 */
static int scmi_pinctrl_negotiate_protocol_version_handler(
		fwk_id_t service_id, const uint32_t *payload)
{
    const struct scmi_negotiate_protocol_version_a2p *parameters =
		(struct scmi_negotiate_protocol_version_a2p *)payload;
    int32_t return_value;
    int respond_status;

    parameters = (const struct scmi_negotiate_protocol_version_a2p *)payload;
    if (parameters->version != SCMI_PROTOCOL_VERSION_PIN_CONTROL)
	return_value = (int32_t)SCMI_NOT_SUPPORTED;
    else
	return_value = (int32_t)SCMI_SUCCESS;

    respond_status = scmi_api->respond(service_id, &return_value,
				       sizeof(return_value));
    if (respond_status != FWK_SUCCESS) {
	FWK_LOG_DEBUG("[SCMI-PINCTRL] %s @%d", __func__, __LINE__);
    }

    return respond_status;
}

/*
 * Protocol Attributes
 */
static int scmi_pinctrl_protocol_attributes_handler(fwk_id_t service_id,
						    const uint32_t *payload)
{
    struct scmi_pinctrl_protocol_attributes_p2a return_values = {
        .status = (int32_t)SCMI_SUCCESS,
    };
    uint32_t agent_id;
    unsigned int pin_count, group_count, function_count, i;
    int status, respond_status;


    status = scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        return status;

    if (agent_id >= SCMI_AGENT_TYPE_COUNT)
        return FWK_E_PARAM;

    for (i = 0, pin_count = 0; i < scmi_pinctrl_config.pin_count; ) {
	if (scmi_pinctrl_config.pin_table[i].agent_permission &
	    (0x1 << agent_id))
	     pin_count++;
    }

    for (i = 0, group_count = 0; i < scmi_pinctrl_config.group_count; ) {
	if (scmi_pinctrl_config.group_table[i].agent_permission &
	    (0x1 << agent_id))
	     group_count++;
    }

    for (i = 0, function_count = 0; i < scmi_pinctrl_config.function_count; ) {
	if (scmi_pinctrl_config.function_table[i].agent_permission &
	    (0x1 << agent_id))
	     function_count++;
    }

    return_values.attributes_low =
	((group_count & 0xffff) << SCMI_PINCTRL_PROTOCOL_ATTRIBUTES_GROUP_POS)
	+ (pin_count & 0xffff);
    return_values.attributes_high = function_count & 0xffff;

    respond_status = scmi_api->respond(service_id, &return_values,
				       sizeof(return_values.status));
    if (respond_status != FWK_SUCCESS) {
	FWK_LOG_DEBUG("[SCMI-PINCTRL] %s @%d", __func__, __LINE__);
    }

    return respond_status;
}

/*
 * Protocol Message Attributes
 */
static int scmi_pinctrl_protocol_message_attributes_handler(
	fwk_id_t service_id, const uint32_t *payload)
{
    const struct scmi_protocol_message_attributes_a2p *parameters =
		(struct scmi_protocol_message_attributes_a2p *)payload;
    struct scmi_protocol_message_attributes_p2a return_values = {
        .status = (int32_t)SCMI_SUCCESS,
        .attributes = 0,
    };
    uint32_t message_id;

    parameters = (const struct scmi_protocol_message_attributes_a2p *)payload;
    message_id = parameters->message_id;

    if ((message_id >= FWK_ARRAY_SIZE(handler_table)) ||
        (handler_table[message_id] == NULL)) {
        return_values.status = (int32_t)SCMI_NOT_FOUND;
        goto exit;
    }

exit:
    return scmi_api->respond(service_id, &return_values, sizeof(return_values));
}

/*
 * Pinctrl Attributes
 */
static int scmi_pinctrl_attributes_handler(fwk_id_t service_id,
					   const uint32_t *payload)
{
    const struct scmi_pinctrl_attributes_a2p *parameters =
		(struct scmi_pinctrl_attributes_a2p *)payload;
    struct scmi_pinctrl_attributes_p2a return_values = {
	.status = (int32_t)SCMI_SUCCESS,
	.attributes = 0,
    };
    uint32_t agent_id, id;
    int status;

    status = scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        return status;

    if (agent_id >= SCMI_AGENT_TYPE_COUNT)
        return FWK_E_PARAM;

    id = parameters->identifier & 0xffff;
    switch (parameters->flags & SCMI_PINCTRL_SELECTOR_MASK) {
    case SCMI_PINCTRL_SELECTOR_PIN:
	if (!(scmi_pinctrl_config.pin_table[id].agent_permission &
	     (0x1 << agent_id))) {
		return_values.status = (int32_t)SCMI_NOT_FOUND;
		break;
	}

	/* name */
	if (strlen(scmi_pinctrl_config.pin_table[id].name) >
		(SCMI_PINCTRL_NAME_LENGTH_MAX - 1))
	    return_values.attributes &= SCMI_PINCTRL_EXTENDED_NAME_MASK;
	strncpy((char *)return_values.name,
		scmi_pinctrl_config.pin_table[id].name,
		SCMI_PINCTRL_NAME_LENGTH_MAX);
	return_values.name[SCMI_PINCTRL_NAME_LENGTH_MAX - 1] = '\0';

	/* number of pins */
	return_values.attributes &= 1;

	break;
    case SCMI_PINCTRL_SELECTOR_GROUP:
	if (!(scmi_pinctrl_config.group_table[id].agent_permission &
	     (0x1 << agent_id))) {
		return_values.status = (int32_t)SCMI_NOT_FOUND;
		break;
	}

	/* name */
	if (strlen(scmi_pinctrl_config.group_table[id].name) >
		(SCMI_PINCTRL_NAME_LENGTH_MAX - 1))
	    return_values.attributes &= SCMI_PINCTRL_EXTENDED_NAME_MASK;
	strncpy((char *)return_values.name,
		scmi_pinctrl_config.group_table[id].name,
		SCMI_PINCTRL_NAME_LENGTH_MAX);
	return_values.name[SCMI_PINCTRL_NAME_LENGTH_MAX - 1] = '\0';

	/* number of pins */
	return_values.attributes &=
		scmi_pinctrl_config.group_table[id].pins_count;

	break;
    case SCMI_PINCTRL_SELECTOR_FUNCTION:
	if (!(scmi_pinctrl_config.function_table[id].agent_permission &
	     (0x1 << agent_id))) {
		return_values.status = (int32_t)SCMI_NOT_FOUND;
		break;
	}

	/* name */
	if (strlen(scmi_pinctrl_config.function_table[id].name) >
		(SCMI_PINCTRL_NAME_LENGTH_MAX - 1))
	    return_values.attributes &= SCMI_PINCTRL_EXTENDED_NAME_MASK;
	strncpy((char *)return_values.name,
		scmi_pinctrl_config.function_table[id].name,
		SCMI_PINCTRL_NAME_LENGTH_MAX);
	return_values.name[SCMI_PINCTRL_NAME_LENGTH_MAX - 1] = '\0';

	/* TODO: single-pin group flag */

	/* number of groups */
	return_values.attributes &=
		scmi_pinctrl_config.function_table[id].groups_count;

	break;
    default:
	return_values.status = (int32_t)SCMI_NOT_FOUND;
    }

    return scmi_api->respond(service_id, &return_values, sizeof(return_values));
}

/*
 * Pinctrl List Associations
 */
static int scmi_pinctrl_list_assoc_handler(fwk_id_t service_id,
					   const uint32_t *payload)
{
    const struct scmi_pinctrl_list_assoc_a2p *parameters =
		(struct scmi_pinctrl_list_assoc_a2p *)payload;
    struct scmi_pinctrl_list_assoc_p2a *return_values, return_values_data;
    uint32_t agent_id, id, index;
    int status;

    status = scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        return status;

    if (agent_id >= SCMI_AGENT_TYPE_COUNT)
        return FWK_E_PARAM;

    id = parameters->identifier & 0xffff;
    index = parameters->index;
    switch (parameters->flags & SCMI_PINCTRL_SELECTOR_MASK) {
    case SCMI_PINCTRL_SELECTOR_GROUP:
	if (id >= scmi_pinctrl_config.group_count) {
	    return_values_data.status = (int32_t)SCMI_NOT_FOUND;
	    return_values = &return_values_data;
	    break;
	}

	if (!(scmi_pinctrl_config.group_table[id].agent_permission &
	     (0x1 << agent_id))) {
	    return_values_data.status = (int32_t)SCMI_DENIED;
	    return_values = &return_values_data;
	    break;
	}

	if (index >= scmi_pinctrl_config.group_table[id].pins_count) {
	    return_values_data.status = (int32_t)SCMI_INVALID_PARAMETERS;
	    return_values = &return_values_data;
	    break;
	}

	/*
	 * TODO: take care of possible maximum size of return values,
	 * especially a number of remaining pins.
	 */
	return_values = malloc(sizeof(*return_values) +
		sizeof(uint16_t) *
			(scmi_pinctrl_config.group_table[id].pins_count -
				index));
	if (!return_values)
	    return FWK_E_NOMEM;

	return_values->status = (int32_t)SCMI_SUCCESS;
	return_values->flags = (int32_t)
		(scmi_pinctrl_config.group_table[id].pins_count - index) &
			0xfff;
	memcpy(return_values->array,
	       &scmi_pinctrl_config.group_table[id].pins[index],
	       sizeof(uint16_t) *
		((scmi_pinctrl_config.group_table[id].pins_count - index) &
			0xfff));
	break;
    case SCMI_PINCTRL_SELECTOR_FUNCTION:
	if (id >= scmi_pinctrl_config.group_count) {
	    return_values_data.status = (int32_t)SCMI_NOT_FOUND;
	    return_values = &return_values_data;
	    break;
	}

	if (!(scmi_pinctrl_config.function_table[id].agent_permission &
	     (0x1 << agent_id))) {
	    return_values_data.status = (int32_t)SCMI_DENIED;
	    return_values = &return_values_data;
	    break;
	}

	if (index >= scmi_pinctrl_config.function_table[id].groups_count) {
	    return_values_data.status = (int32_t)SCMI_INVALID_PARAMETERS;
	    return_values = &return_values_data;
	    break;
	}

	/* TODO: single-pin function or single-pin group? */
	/*
	 * TODO: take care of possible maximum size of return values,
	 * especially a number of remaining pins.
	 */
	return_values = malloc(sizeof(*return_values) +
		sizeof(uint16_t) *
			(scmi_pinctrl_config.function_table[id].groups_count -
				index));
	if (!return_values)
	    return FWK_E_NOMEM;

	return_values->status = (int32_t)SCMI_SUCCESS;
	return_values->flags = (int32_t)
		(scmi_pinctrl_config.function_table[id].groups_count - index) &
			0xfff;
	memcpy(return_values->array,
	       &scmi_pinctrl_config.function_table[id].groups[index],
	       sizeof(uint16_t) *
		((scmi_pinctrl_config.function_table[id].groups_count - index) &
			0xfff));
	break;
    default:
	return_values_data.status = (int32_t)SCMI_INVALID_PARAMETERS;
	return_values = &return_values_data;
	break;
    }

    return scmi_api->respond(service_id, return_values, sizeof(*return_values));
}

/*
 * Pinctrl Settings Get
 */
static int scmi_pinctrl_settings_get_handler(fwk_id_t service_id,
					     const uint32_t *payload)
{
    return FWK_SUCCESS;
}

/*
 * Pinctrl Settings Configure
 */
static int scmi_pinctrl_settings_configure_handler(fwk_id_t service_id,
						   const uint32_t *payload)
{
    return FWK_SUCCESS;
}

/*
 * Pinctrl Reuqest
 */
static int scmi_pinctrl_request_handler(fwk_id_t service_id,
					const uint32_t *payload)
{
    const struct scmi_pinctrl_request_a2p *parameters =
		(struct scmi_pinctrl_request_a2p *)payload;
    uint32_t return_value;
    uint32_t agent_id, id;
    int status;

    status = scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        return status;

    if (agent_id >= SCMI_AGENT_TYPE_COUNT)
        return FWK_E_PARAM;

    id = parameters->identifier & 0xffff;
    switch (parameters->flags & SCMI_PINCTRL_SELECTOR_MASK) {
    case SCMI_PINCTRL_SELECTOR_PIN:
	if (id >= scmi_pinctrl_config.pin_count) {
	    return_value = (int32_t)SCMI_NOT_FOUND;
	    break;
	}

	if (!(scmi_pinctrl_config.pin_table[id].agent_permission &
	     (0x1 << agent_id))) {
		return_value = (int32_t)SCMI_DENIED;
		break;
	}
	/* TODO: locking */
	if (scmi_pinctrl_ctx.pin_state[id].owner_agent !=
		SCMI_PINCTRL_NO_OWNER) {
	    return_value = (uint32_t)SCMI_IN_USE;
	} else {
	    scmi_pinctrl_ctx.pin_state[id].owner_agent = agent_id;
	    return_value = (uint32_t)SCMI_SUCCESS;
	}
	break;
    case SCMI_PINCTRL_SELECTOR_GROUP:
	if (id >= scmi_pinctrl_config.group_count) {
	    return_value = (int32_t)SCMI_NOT_FOUND;
	    break;
	}

	if (!(scmi_pinctrl_config.group_table[id].agent_permission &
	     (0x1 << agent_id))) {
		return_value = (int32_t)SCMI_DENIED;
		break;
	}
	/* TODO: locking */
	if (scmi_pinctrl_ctx.group_state[id].owner_agent !=
		SCMI_PINCTRL_NO_OWNER) {
	    return_value = (uint32_t)SCMI_IN_USE;
	} else {
	    scmi_pinctrl_ctx.group_state[id].owner_agent = agent_id;
	    return_value = (uint32_t)SCMI_SUCCESS;
	}
	break;
    default:
	return_value = (int32_t)SCMI_INVALID_PARAMETERS;
    }

    return scmi_api->respond(service_id, &return_value, sizeof(return_value));
}

/*
 * Pinctrl Release
 */
static int scmi_pinctrl_release_handler(fwk_id_t service_id,
					const uint32_t *payload)
{
    const struct scmi_pinctrl_release_a2p *parameters =
		(struct scmi_pinctrl_release_a2p *)payload;
    uint32_t return_value;
    uint32_t agent_id, id;
    int status;

    status = scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        return status;

    if (agent_id >= SCMI_AGENT_TYPE_COUNT)
        return FWK_E_PARAM;

    id = parameters->identifier & 0xffff;
    switch (parameters->flags & SCMI_PINCTRL_SELECTOR_MASK) {
    case SCMI_PINCTRL_SELECTOR_PIN:
	if (id >= scmi_pinctrl_config.pin_count) {
	    return_value = (int32_t)SCMI_NOT_FOUND;
	    break;
	}

	if (!(scmi_pinctrl_config.pin_table[id].agent_permission &
	     (0x1 << agent_id))) {
		return_value = (int32_t)SCMI_DENIED;
		break;
	}
	/* TODO: locking */
	if (scmi_pinctrl_ctx.pin_state[id].owner_agent == agent_id) {
	    scmi_pinctrl_ctx.group_state[id].owner_agent =
		    SCMI_PINCTRL_NO_OWNER;
	    return_value = (uint32_t)SCMI_SUCCESS;
	} else if (scmi_pinctrl_ctx.group_state[id].owner_agent ==
		SCMI_PINCTRL_NO_OWNER) {
	    return_value = (uint32_t)SCMI_SUCCESS;
	} else {
	    return_value = (uint32_t)SCMI_IN_USE;
	}
	break;
    case SCMI_PINCTRL_SELECTOR_GROUP:
	if (id >= scmi_pinctrl_config.group_count) {
	    return_value = (int32_t)SCMI_NOT_FOUND;
	    break;
	}

	if (!(scmi_pinctrl_config.group_table[id].agent_permission &
	     (0x1 << agent_id))) {
		return_value = (int32_t)SCMI_DENIED;
		break;
	}
	/* TODO: locking */
	if (scmi_pinctrl_ctx.group_state[id].owner_agent == agent_id) {
	    scmi_pinctrl_ctx.group_state[id].owner_agent =
		    SCMI_PINCTRL_NO_OWNER;
	    return_value = (uint32_t)SCMI_SUCCESS;
	} else if (scmi_pinctrl_ctx.group_state[id].owner_agent ==
		SCMI_PINCTRL_NO_OWNER) {
	    return_value = (uint32_t)SCMI_SUCCESS;
	} else {
	    return_value = (uint32_t)SCMI_IN_USE;
	}
	break;
    default:
	return_value = (int32_t)SCMI_INVALID_PARAMETERS;
    }

    return scmi_api->respond(service_id, &return_value, sizeof(return_value));
}

/*
 * Pinctrl Name Get
 */
static int scmi_pinctrl_name_get_handler(fwk_id_t service_id,
					 const uint32_t *payload)
{
    const struct scmi_pinctrl_name_get_a2p *parameters =
		(struct scmi_pinctrl_name_get_a2p *)payload;
    struct scmi_pinctrl_name_get_p2a return_values = {
	.status = (uint32_t)SCMI_SUCCESS,
	.flags = 0,
    };
    uint32_t agent_id, id;
    int status;

    status = scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        return status;

    if (agent_id >= SCMI_AGENT_TYPE_COUNT)
        return FWK_E_PARAM;

    id = parameters->identifier & 0xffff;
    switch (parameters->flags & SCMI_PINCTRL_SELECTOR_MASK) {
    case SCMI_PINCTRL_SELECTOR_PIN:
	if (id >= scmi_pinctrl_config.pin_count) {
	    return_values.status = (int32_t)SCMI_NOT_FOUND;
	    break;
	}

	if (!(scmi_pinctrl_config.pin_table[id].agent_permission &
	     (0x1 << agent_id))) {
		return_values.status = (int32_t)SCMI_NOT_FOUND;
		break;
	}

	strncpy((char *)return_values.name,
		scmi_pinctrl_config.pin_table[id].name,
		SCMI_PINCTRL_EXTENDED_NAME_LENGTH_MAX - 1);
	return_values.name[SCMI_PINCTRL_EXTENDED_NAME_LENGTH_MAX - 1] = '\0';
	break;
    case SCMI_PINCTRL_SELECTOR_GROUP:
	if (id >= scmi_pinctrl_config.group_count) {
	    return_values.status = (int32_t)SCMI_NOT_FOUND;
	    break;
	}

	if (!(scmi_pinctrl_config.group_table[id].agent_permission &
	     (0x1 << agent_id))) {
		return_values.status = (int32_t)SCMI_NOT_FOUND;
		break;
	}

	strncpy((char *)return_values.name,
		scmi_pinctrl_config.group_table[id].name,
		SCMI_PINCTRL_EXTENDED_NAME_LENGTH_MAX - 1);
	return_values.name[SCMI_PINCTRL_EXTENDED_NAME_LENGTH_MAX - 1] = '\0';
	break;
    case SCMI_PINCTRL_SELECTOR_FUNCTION:
	if (id >= scmi_pinctrl_config.function_count) {
	    return_values.status = (int32_t)SCMI_NOT_FOUND;
	    break;
	}

	if (!(scmi_pinctrl_config.function_table[id].agent_permission &
	     (0x1 << agent_id))) {
		return_values.status = (int32_t)SCMI_NOT_FOUND;
		break;
	}

	strncpy((char *)return_values.name,
		scmi_pinctrl_config.function_table[id].name,
		SCMI_PINCTRL_EXTENDED_NAME_LENGTH_MAX - 1);
	return_values.name[SCMI_PINCTRL_EXTENDED_NAME_LENGTH_MAX - 1] = '\0';
	break;
    default:
	return_values.status = (int32_t)SCMI_INVALID_PARAMETERS;
    }

    return scmi_api->respond(service_id, &return_values, sizeof(return_values));
}

/*
 * Pinctrl Set Permissions
 */
static int scmi_pinctrl_set_permissions_handler(fwk_id_t service_id,
						const uint32_t *payload)
{
    /*
     * TODO
     * We need a product-specific(?) policy against agents.
     */
    return FWK_SUCCESS;
}

/*
 * SCMI module -> SCMI pinctrl module interface
 */
static int scmi_pinctrl_get_scmi_protocol_id(fwk_id_t protocol_id,
					     uint8_t *scmi_protocol_id)
{
    *scmi_protocol_id = (uint8_t)MOD_SCMI_PROTOCOL_ID_PIN_CONTROL;

    return FWK_SUCCESS;
}

static int scmi_pinctrl_message_handler(fwk_id_t protocol_id,
					fwk_id_t service_id,
					const uint32_t *payload,
					size_t payload_size,
					unsigned int message_id)
{
    uint32_t return_value;
    uint32_t agent_id;
    int status;

    static_assert(FWK_ARRAY_SIZE(handler_table) ==
        FWK_ARRAY_SIZE(payload_size_table),
        "[SCMI] Pin control protocol table sizes not consistent");
    fwk_assert(payload != NULL);

    status = scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        return status;

    if (agent_id >= SCMI_AGENT_TYPE_COUNT)
        return FWK_E_PARAM;

    if (message_id >= FWK_ARRAY_SIZE(handler_table)) {
        return_value = (int32_t)SCMI_NOT_FOUND;
        goto error;
    }

    if (payload_size != payload_size_table[message_id]) {
        return_value = (int32_t)SCMI_PROTOCOL_ERROR;
        goto error;
    }

    return handler_table[message_id](service_id, payload);

error:
    return scmi_api->respond(service_id, &return_value, sizeof(return_value));
}

static struct mod_scmi_to_protocol_api scmi_pinctrl_mod_scmi_to_protocol_api = {
    .get_scmi_protocol_id = scmi_pinctrl_get_scmi_protocol_id,
    .message_handler = scmi_pinctrl_message_handler,
};

/*
 * Framework handlers
 */
static int scmi_pinctrl_init(fwk_id_t module_id, unsigned int element_count,
			     const void *data)
{
    unsigned int i;
    int ret;

    if (data == NULL || element_count) {
        return FWK_E_PARAM;
    }

    scmi_pinctrl_config = *(const struct mod_scmi_pinctrl_config *)data;

    scmi_pinctrl_ctx.pin_state = fwk_mm_calloc(
		sizeof(struct scmi_pinctrl_pingrp_state),
		scmi_pinctrl_config.pin_count);
    if (!scmi_pinctrl_ctx.pin_state)
	return FWK_E_NOMEM;

    scmi_pinctrl_ctx.group_state = fwk_mm_calloc(
		sizeof(struct scmi_pinctrl_pingrp_state),
		scmi_pinctrl_config.group_count);
    if (!scmi_pinctrl_ctx.group_state) {
	ret = FWK_E_NOMEM;
	goto error;
    }

    /* pins */
    for (i = 0; i < scmi_pinctrl_config.pin_count; i++) {
	/* TODO: product-specific initial state */
	scmi_pinctrl_ctx.pin_state[i].state =
		SCMI_PINCTRL_PIN_STATE_RELEASED;
	scmi_pinctrl_ctx.pin_state[i].owner_agent = SCMI_PINCTRL_NO_OWNER;
	scmi_pinctrl_ctx.pin_state[i].function = SCMI_PINCTRL_NO_FUNCTION;

	scmi_pinctrl_ctx.pin_state[i].config = fwk_mm_calloc(
				sizeof(struct scmi_pinctrl_config_state),
				scmi_pinctrl_config.config_count);
	if (!scmi_pinctrl_ctx.pin_state[i].config) {
	    ret = FWK_E_NOMEM;
	    goto error;
	}
    }

    /* groups */
    for (i = 0; i < scmi_pinctrl_config.group_count; i++) {
	/* TODO: product-specific initial state */
	scmi_pinctrl_ctx.group_state[i].state =
		SCMI_PINCTRL_PIN_STATE_RELEASED;
	scmi_pinctrl_ctx.group_state[i].owner_agent = SCMI_PINCTRL_NO_OWNER;
	scmi_pinctrl_ctx.group_state[i].function = SCMI_PINCTRL_NO_FUNCTION;

	scmi_pinctrl_ctx.group_state[i].config = fwk_mm_calloc(
				sizeof(struct scmi_pinctrl_config_state),
				scmi_pinctrl_config.config_count);
	if (!scmi_pinctrl_ctx.group_state[i].config) {
	    ret = FWK_E_NOMEM;
	    goto error;
	}
    }

    return FWK_SUCCESS;

error:
    if (scmi_pinctrl_ctx.pin_state) {
	for (i = 0; i < scmi_pinctrl_config.pin_count; i++)
	    fwk_mm_free(scmi_pinctrl_ctx.pin_state[i].config);

	fwk_mm_free(scmi_pinctrl_ctx.pin_state);
    }
    if (scmi_pinctrl_ctx.group_state) {
	for (i = 0; i < scmi_pinctrl_config.group_count; i++)
	    fwk_mm_free(scmi_pinctrl_ctx.group_state[i].config);

	fwk_mm_free(scmi_pinctrl_ctx.group_state);
    }

    return ret;
}

static int scmi_pinctrl_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round == 1) {
        return FWK_SUCCESS;
    }

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
			     FWK_ID_API(FWK_MODULE_IDX_SCMI,
					MOD_SCMI_API_IDX_PROTOCOL),
			     &scmi_api);
    if (status != FWK_SUCCESS) {
        return status;
    }

    /*
     * TODO: bind lower device driver(s)
     */
    return FWK_SUCCESS;
}

static int scmi_pinctrl_process_bind_request(fwk_id_t source_id,
					     fwk_id_t target_id,
					     fwk_id_t api_id, const void **api)
{
    if (!fwk_id_is_equal(source_id, FWK_ID_MODULE(FWK_MODULE_IDX_SCMI)))
        return FWK_E_ACCESS;

    if (api_id.api.api_idx)
        return FWK_E_ACCESS;

    *api = &scmi_pinctrl_mod_scmi_to_protocol_api;

	return FWK_SUCCESS;
}

static int scmi_pinctrl_process_event(const struct fwk_event *event,
				      struct fwk_event *resp_event)
{
    return FWK_SUCCESS;
}

/* SCMI Pin Control Protocol Definition */
const struct fwk_module module_scmi_pinctrl = {
    .api_count = 1,
    .event_count = (unsigned int)SCMI_PINCTRL_EVENT_IDX_COUNT,
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .init = scmi_pinctrl_init,
    .bind = scmi_pinctrl_bind,
    .process_bind_request = scmi_pinctrl_process_bind_request,
    .process_event = scmi_pinctrl_process_event,
};
