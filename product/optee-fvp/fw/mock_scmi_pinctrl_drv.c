/*
 * Arm SCP/MCP Software - SCMI pin control mock driver
 * Copyright (c) 2024, Linaro Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_scmi_pinctrl.h>
#include <mod_scmi_std.h>
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
 */

/*
 * Primary mock device
 */
uint32_t pinctrl_drv0_init(unsigned int drv_id)
{
    return SCMI_SUCCESS;
}

uint32_t pinctrl_drv0_fini(unsigned int drv_id)
{
    return SCMI_SUCCESS;
}

uint32_t pinctrl_drv0_set_function(void)
{
    return SCMI_SUCCESS;
}

uint32_t pinctrl_drv0_set_config(void)
{
    return SCMI_SUCCESS;
}

uint32_t pinctrl_drv0_set_direction(void)
{
    return SCMI_SUCCESS;
}

uint32_t pinctrl_drv0_set_value(void)
{
    return SCMI_SUCCESS;
}

uint32_t pinctrl_drv0_get_value(void)
{
    return SCMI_SUCCESS;
}

/*
 * Secondary mock device
 */
uint32_t pinctrl_drv1_init(unsigned int drv_id)
{
    return SCMI_SUCCESS;
}

uint32_t pinctrl_drv1_fini(unsigned int drv_id)
{
    return SCMI_SUCCESS;
}

uint32_t pinctrl_drv1_set_function(void)
{
    return SCMI_SUCCESS;
}

uint32_t pinctrl_drv1_set_config(void)
{
    return SCMI_SUCCESS;
}

uint32_t pinctrl_drv1_set_direction(void)
{
    return SCMI_SUCCESS;
}

uint32_t pinctrl_drv1_set_value(void)
{
    return SCMI_SUCCESS;
}

uint32_t pinctrl_drv1_get_value(void)
{
    return SCMI_SUCCESS;
}

struct scmi_pinctrl_driver_api pinctrl_drv0_api = {
    .init = pinctrl_drv0_init,
    .fini = pinctrl_drv0_fini,
    .set_function = pinctrl_drv0_set_function,
    .set_config = pinctrl_drv0_set_config,
    .set_direction = pinctrl_drv0_set_direction,
    .set_value = pinctrl_drv0_set_value,
    .get_value = pinctrl_drv0_get_value,
};

struct scmi_pinctrl_driver_api pinctrl_drv1_api = {
    .init = pinctrl_drv1_init,
    .fini = pinctrl_drv1_fini,
    .set_function = pinctrl_drv1_set_function,
    .set_config = pinctrl_drv1_set_config,
    .set_direction = pinctrl_drv1_set_direction,
    .set_value = pinctrl_drv1_set_value,
    .get_value = pinctrl_drv1_get_value,
};
