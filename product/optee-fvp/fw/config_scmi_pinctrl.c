/*
 * Arm SCP/MCP Software - SCMI pin control module for OPTEE-FVP
 * Copyright (c) 2024, Linaro Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_module.h>
#include <mock_scmi_pinctrl.h>
#include <mod_scmi_pinctrl.h>
#include <stdint.h>

/*
 * Example Pin Layout
 *      Controller-0                   Controller-1      
 *      ----                           ----
 *  0:  gpio0                          gpio10    uart1(TX)
 *  1:  gpio1                          gpio11    uart1(RX)
 *  2:  gpio2                          gpio12    uart1(CTS)
 *  3:  gpio3                          gpio13    uart1(RTS)
 *  4:  gpio4    i2c0(SCL)  uart0(TX)  gpio14    spi0(nCS)   uart2(TX)
 *  5:  gpio5    i2c0(SDA)  uart0(RX)  gpio15    spi0(SCLK)  uart2(RX)
 *  6:  gpio6    i2c1(SCL)  uart0(CTS) gpio16    spi0(SDI)   uart2(CTS)
 *  7:  gpio7    i2c1(SDA)  uart0(RTS) gpio17    spi0(SDO)   uart2(RTS)
 *  8:  gpio8
 *  9:  gpio9
 *
 * gpio8, gpio9, gpio14-gpio17 are invisible from OSPM agent.
 */

/*
 * SCMI pinctrl pin definitions
 */

static const
struct scmi_pinctrl_pin_data pin_table[] = {
    [0] = SCMI_PINCTRL_PIN(pin_x0, 0, 0x02),
    [1] = SCMI_PINCTRL_PIN(pin_x1, 0, 0x02),
    [2] = SCMI_PINCTRL_PIN(pin_x2, 0, 0x02),
    [3] = SCMI_PINCTRL_PIN(pin_x3, 0, 0x02),
    [4] = SCMI_PINCTRL_PIN(pin_x4, 0, 0x02),
    [5] = SCMI_PINCTRL_PIN(pin_x5, 0, 0x02),
    [6] = SCMI_PINCTRL_PIN(pin_x6, 0, 0x02),
    [7] = SCMI_PINCTRL_PIN(pin_x7, 0, 0x02),
    [8] = SCMI_PINCTRL_PIN(pin_x8, 0, 0x00),
    [9] = SCMI_PINCTRL_PIN(pin_x9, 0, 0x00),
    [10] = SCMI_PINCTRL_PIN(pin_x10, 1, 0x02),
    [11] = SCMI_PINCTRL_PIN(pin_x11, 1, 0x02),
    [12] = SCMI_PINCTRL_PIN(pin_x12, 1, 0x02),
    [13] = SCMI_PINCTRL_PIN(pin_x13, 1, 0x02),
    [14] = SCMI_PINCTRL_PIN(pin_x14, 1, 0x00),
    [15] = SCMI_PINCTRL_PIN(pin_x15, 1, 0x00),
    [16] = SCMI_PINCTRL_PIN(pin_x16, 1, 0x00),
    [17] = SCMI_PINCTRL_PIN(pin_x17, 1, 0x00),
};

/*
 * SCMI pinctrl group definitions
 */

static const uint16_t grp_gpio0_pins[] = {0, 1, 2, 3};
static const uint16_t grp_gpio_i2c0_pins[] = {4, 5};
static const uint16_t grp_gpio_i2c1_pins[] = {6, 7};
static const uint16_t grp_gpio_uart0_pins[] = {4, 5, 6, 7};
static const uint16_t grp_gpio1_pins[] = {8, 9};
static const uint16_t grp_gpio_uart1_pins[] = {10, 11, 12, 13};
static const uint16_t grp_gpio_spi_uart2_pins[] = {14, 15, 16, 17};

static const
struct scmi_pinctrl_group_data group_table[] = {
    /* Pin control device 0 */
    [0] = SCMI_PINCTRL_GROUP(grp_gpio0, 0x02),
    [1] = SCMI_PINCTRL_GROUP(grp_gpio_i2c0, 0x02),
    [2] = SCMI_PINCTRL_GROUP(grp_gpio_i2c1, 0x02),
    [3] = SCMI_PINCTRL_GROUP(grp_gpio_uart0, 0x02),
    [4] = SCMI_PINCTRL_GROUP(grp_gpio1, 0x00),

    /* Pin control device 1 */
    [5] = SCMI_PINCTRL_GROUP(grp_gpio_uart1, 0x02),
    [6] = SCMI_PINCTRL_GROUP(grp_gpio_spi_uart2, 0x00),
};

/*
 * SCMI pinctrl function definitions
 */

static const uint16_t f_gpio[] = {0, 3, 4, 5, 6};
static const uint16_t f_i2c[] = {1, 2};
static const uint16_t f_uart[] = {3, 5, 6};
static const uint16_t f_spi[] = {6};

static const
struct scmi_pinctrl_function_data function_table[] = {
    [0] = SCMI_PINCTRL_FUNC(f_gpio, 0x02),
    [1] = SCMI_PINCTRL_FUNC(f_i2c, 0x02),
    [2] = SCMI_PINCTRL_FUNC(f_uart, 0x02),
    [3] = SCMI_PINCTRL_FUNC(f_spi, 0x00),
};

/*
 * SCMI pinctrl driver definitions
 */

static const
struct scmi_pinctrl_driver_data driver_table[] = {
    { .api = &pinctrl_drv0_api, },
    { .api = &pinctrl_drv1_api, },
};

/*
 * SCMI pinctrl module configuration
 */

const
struct fwk_module_config config_scmi_pinctrl = {
    .data = &(struct mod_scmi_pinctrl_config) {
	.pin_table = pin_table,
	.pin_count = FWK_ARRAY_SIZE(pin_table),
	.group_table = group_table,
	.group_count = FWK_ARRAY_SIZE(group_table),
	.function_table = function_table,
	.function_count = FWK_ARRAY_SIZE(function_table),

	.config_count = SCMI_PINCTRL_CONFIG_COUNT,

	.driver_table = driver_table,
	.driver_count = FWK_ARRAY_SIZE(driver_table),
    },

    /*
     * Note: Currently no elements.
     * Pins or drivers (pin controllers) may be candidates
     * for an element type.
     * .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_dev_desc_table),
     */
};

