/* Copyright (c) 2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <asm/mach-types.h>
#include <linux/i2c.h>
#include "../devices-msm7x2xa.h"

#ifdef CONFIG_SENSORS_AK8963
#include <linux/akm8963.h>
#endif

#if defined(CONFIG_I2C) && defined(CONFIG_INPUT_PS31XX)
#include <linux/stk31xx.h>
#endif

#ifdef CONFIG_GSENSOR_BMA2X2
static struct i2c_board_info bma2x2_i2c_info[] __initdata = {
	{
		I2C_BOARD_INFO("bma2x2", 0x18),
	},
};
#endif

#ifdef CONFIG_SENSORS_AK8963
#define AKM_GPIO_DRDY       27
#define AKM_GPIO_RST       0
#define AKM_LAYOUT           3
#define AKM_OUTBIT           1
static struct akm8963_platform_data akm_platform_data_8963 = {
	.gpio_DRDY        = AKM_GPIO_DRDY,
//	.gpio_RST         = AKM_GPIO_RST,        
	.layout           = AKM_LAYOUT,
	.outbit           = AKM_OUTBIT,
};
static int akm_gpio_setup(void) {
	int rc;

	rc = gpio_tlmm_config(GPIO_CFG(AKM_GPIO_DRDY, 0,
				GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_6MA), GPIO_CFG_ENABLE);
	if (rc) {
		pr_err("%s: gpio_tlmm_config for %d failed\n",
			__func__, AKM_GPIO_DRDY);
		return rc;
	}

	rc = gpio_request(AKM_GPIO_DRDY, "akm_init");
	return rc;
}

static struct i2c_board_info akm8963_i2c_info[] __initdata = {
	{	I2C_BOARD_INFO("akm8963", 0x0D),
		.flags 		= I2C_CLIENT_WAKE,
		.platform_data  = &akm_platform_data_8963,
		.irq  		= MSM_GPIO_TO_INT(AKM_GPIO_DRDY),
	},
};

#endif

#if defined(CONFIG_I2C) && defined(CONFIG_INPUT_PS31XX)
#define PS31XX_INT         17
static struct stk31xx_platform_data stk31xx_data = {
	.als_cmd = 0x4c,
	.ps_cmd =  0x21,
	.ps_gain = 0x0D,
	.transmittance = 800,
	.ps_high_thd   = 120,
	.ps_low_thd    = 100,
	.int_pin       = PS31XX_INT,
};

static struct i2c_board_info ps31xx_light_i2c_info[] __initdata = {
	{
		I2C_BOARD_INFO("stk_ps", 0x48),
		.platform_data = &stk31xx_data,
		.irq           = MSM_GPIO_TO_INT(PS31XX_INT),
	},
};

#endif


void __init msm7627a_sensor_init(void)
{




#ifdef CONFIG_GSENSOR_BMA2X2
		pr_info("i2c_register_board_info BMA2X2 ACC\n");
		i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
					bma2x2_i2c_info,
					ARRAY_SIZE(bma2x2_i2c_info));
#endif 

#ifdef CONFIG_SENSORS_AK8963
		pr_info("i2c_register_board_info AKM8963\n");
		akm_gpio_setup();
		i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
				akm8963_i2c_info,
				ARRAY_SIZE(akm8963_i2c_info));
#endif

#if defined(CONFIG_INPUT_PS31XX)
	printk("i2c_register_board_info PS31XX\n");
	i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
			ps31xx_light_i2c_info,
			ARRAY_SIZE(ps31xx_light_i2c_info));
#endif

}
