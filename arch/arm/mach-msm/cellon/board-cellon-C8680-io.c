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
 *
 */

#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio_event.h>
#include <linux/leds.h>
#include <linux/i2c/atmel_mxt_ts.h>
#include <linux/i2c.h>
#include <linux/input/rmi_platformdata.h>
#include <linux/input/rmi_i2c.h>
#include <linux/delay.h>
#include <linux/atmel_maxtouch.h>

#include <asm/gpio.h>
#include <asm/mach-types.h>
#include <mach/rpc_server_handset.h>
#include <mach/pmic.h>

#include "../devices.h"
#include "board-cellon-C8680.h"
#include "../devices-msm7x2xa.h"

#ifdef CONFIG_RMI4_SPI
#include <linux/interrupt.h>
#include <plat/mcspi.h>
#include <linux/spi/spi.h>
#include <linux/rmi.h>
#endif


#ifdef CONFIG_RMI4_I2C
#include <linux/interrupt.h>
#include <linux/rmi.h>
#define S2202_ADDR	0x20
#define S2202_ATTN	48
#define S2202_RST   26
#endif /* CONFIG_RMI4_I2C */


#ifdef CONFIG_TOUCHSCREEN_HX8526A
#include <linux/himax_ts.h>
#define HX8526A_ADDR     0x4a
#define HX8526A_RST      26
#define HX8526A_INT      48
#define HX8526A_ID       -1
#endif /*CONFIG_TOUCHSCREEN_HX8526A*/

#define C8680_CTP_ID	115

#define ATMEL_TS_I2C_NAME "maXTouch"
#define ATMEL_X_OFFSET 13
#define ATMEL_Y_OFFSET 0

#ifndef CLEARPAD3000_ATTEN_GPIO
#define CLEARPAD3000_ATTEN_GPIO (48)
#endif

#ifndef CLEARPAD3000_RESET_GPIO
#define CLEARPAD3000_RESET_GPIO (26)
#endif

/* 8625 keypad device information */

static unsigned int kp_row_gpios_8625[] = {31, 33};

static unsigned int kp_col_gpios_8625[] = {36};


static const unsigned short keymap_8625[] = {
	KEY_VOLUMEUP,
	KEY_VOLUMEDOWN,
};

static struct gpio_event_matrix_info kp_matrix_info_8625 = {
	.info.func      = gpio_event_matrix_func,
	.keymap         = keymap_8625,
	.output_gpios   = kp_row_gpios_8625,
	.input_gpios    = kp_col_gpios_8625,
	.noutputs       = ARRAY_SIZE(kp_row_gpios_8625),
	.ninputs        = ARRAY_SIZE(kp_col_gpios_8625),
	.settle_time.tv_nsec = 40 * NSEC_PER_USEC,
	.poll_time.tv_nsec = 20 * NSEC_PER_MSEC,
	.flags          = GPIOKPF_LEVEL_TRIGGERED_IRQ | //GPIOKPF_DRIVE_INACTIVE |
			  GPIOKPF_PRINT_UNMAPPED_KEYS,
};

static struct gpio_event_info *kp_info_8625[] = {
	&kp_matrix_info_8625.info,
};

static struct gpio_event_platform_data kp_pdata_8625 = {
	.name           = "7x27a_kp",
	.info           = kp_info_8625,
	.info_count     = ARRAY_SIZE(kp_info_8625)
};

static struct platform_device kp_pdev_8625 = {
	.name   = GPIO_EVENT_DEV_NAME,
	.id     = -1,
	.dev    = {
		.platform_data  = &kp_pdata_8625,
	},
};

#define LED_RED_GPIO_8625 49
#define LED_GREEN_GPIO_8625 34

#define MXT_TS_IRQ_GPIO         48
#define MXT_TS_RESET_GPIO       26

static const u8 mxt_config_data[] = {
	/* T6 Object */
	0, 0, 0, 0, 0, 0,
	/* T38 Object */
	16, 1, 0, 0, 0, 0, 0, 0,
	/* T7 Object */
	32, 16, 50,
	/* T8 Object */
	30, 0, 20, 20, 0, 0, 20, 0, 50, 0,
	/* T9 Object */
	3, 0, 0, 18, 11, 0, 32, 75, 3, 3,
	0, 1, 1, 0, 10, 10, 10, 10, 31, 3,
	223, 1, 11, 11, 15, 15, 151, 43, 145, 80,
	100, 15, 0, 0, 0,
	/* T15 Object */
	131, 0, 11, 11, 1, 1, 0, 45, 3, 0,
	0,
	/* T18 Object */
	0, 0,
	/* T19 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	/* T23 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	/* T25 Object */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0,
	/* T40 Object */
	0, 0, 0, 0, 0,
	/* T42 Object */
	0, 0, 0, 0, 0, 0, 0, 0,
	/* T46 Object */
	0, 2, 32, 48, 0, 0, 0, 0, 0,
	/* T47 Object */
	1, 20, 60, 5, 2, 50, 40, 0, 0, 40,
	/* T48 Object */
	1, 12, 80, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 6, 6, 0, 0, 100, 4, 64,
	10, 0, 20, 5, 0, 38, 0, 20, 0, 0,
	0, 0, 0, 0, 16, 65, 3, 1, 1, 0,
	10, 10, 10, 0, 0, 15, 15, 154, 58, 145,
	80, 100, 15, 3,
};

#define MAX_LEN		100

static ssize_t mxt_virtual_keys_register(struct kobject *kobj,
                     struct kobj_attribute *attr, char *buf)
{
	char *virtual_keys ;

		/*four virtual keys in Android*/
		virtual_keys = __stringify(EV_KEY) ":" __stringify(KEY_MENU) \
				":60:860:120:80" ":" __stringify(EV_KEY) \
				":" __stringify(KEY_HOME)   ":180:860:120:80" \
				":" __stringify(EV_KEY) ":" \
				__stringify(KEY_BACK) ":300:860:120:80" \
				":" __stringify(EV_KEY) ":" \
				__stringify(KEY_SEARCH)   ":420:860:120:80" "\n";

	return snprintf(buf, strnlen(virtual_keys, MAX_LEN) + 1 , "%s",
                        virtual_keys);
}

static struct kobj_attribute mxt_virtual_keys_attr = {
        .attr = {
                .name = "virtualkeys.atmel_mxt_ts",
                .mode = S_IRUGO,
        },
        .show = &mxt_virtual_keys_register,
};

static struct attribute *mxt_virtual_key_properties_attrs[] = {
        &mxt_virtual_keys_attr.attr,
        NULL
};

static struct attribute_group mxt_virtual_key_properties_attr_group = {
        .attrs = mxt_virtual_key_properties_attrs,
};

struct kobject *mxt_virtual_key_properties_kobj;

static int mxt_touchpad_setup(void)
{
	int retval = 0;

        mxt_virtual_key_properties_kobj =
                kobject_create_and_add("board_properties", NULL);
        if (mxt_virtual_key_properties_kobj)
                retval = sysfs_create_group(mxt_virtual_key_properties_kobj,
                                &mxt_virtual_key_properties_attr_group);
        if (!mxt_virtual_key_properties_kobj || retval)
                pr_err("failed to create mxt board_properties\n");

	return retval;
}

static struct mxt_config_info mxt_config_array[] = {
	{
		.config		= mxt_config_data,
		.config_length	= ARRAY_SIZE(mxt_config_data),
		.family_id	= 0x81,
		.variant_id	= 0x01,
		.version	= 0x10,
		.build		= 0xAA,
	},
};

static int mxt_key_codes[MXT_KEYARRAY_MAX_KEYS] = {
	[0] = KEY_HOME,
	[1] = KEY_MENU,
	[9] = KEY_BACK,
	[10] = KEY_SEARCH,
};

static struct mxt_platform_data mxt_platform_data = {
	.config_array		= mxt_config_array,
	.config_array_size	= ARRAY_SIZE(mxt_config_array),
	.panel_minx		= 0,
	.panel_maxx		= 479,
	.panel_miny		= 0,
	.panel_maxy		= 879,
	.disp_minx		= 0,
	.disp_maxx		= 479,
	.disp_miny		= 0,
	.disp_maxy		= 799,
	.irqflags		= IRQF_TRIGGER_LOW,
	.i2c_pull_up		= true,
//	.reset_gpio		= MXT_TS_RESET_GPIO,
//	.irq_gpio		= MXT_TS_IRQ_GPIO,
	.key_codes		= mxt_key_codes,
};

static struct i2c_board_info mxt_device_info[] __initdata = {
	{
		I2C_BOARD_INFO("atmel_mxt_ts", 0),
		.platform_data = &mxt_platform_data,
		.irq = MSM_GPIO_TO_INT(MXT_TS_IRQ_GPIO),
	},
};

#define _ws(_w)  __stringify(_w)
#define _hs(_h)  __stringify(_h)
#define _ps(_p)  __stringify(_p)
#define _pwhs(_p,_w,_h) _ps(_p)":"_ws(_w)":"_hs(_h)
/* Synaptics change for beagle board */
#if defined(CONFIG_RMI4_I2C) || defined(CONFIG_RMI4_SPI)

struct syna_gpio_data {
	u16 gpio_number;
	char* gpio_name;
};

static int s2202_touchpad_gpio_unconfig(void *gpio_data)
{
    struct syna_gpio_data *data = gpio_data ;
    gpio_free(S2202_RST);
    gpio_free(data->gpio_number);
    return 0;

}
static int s2202_touchpad_gpio_setup(void *gpio_data, bool configure)
{
	int retval=0;
	struct syna_gpio_data *data = gpio_data;


	if (configure) {

		retval = gpio_request(S2202_RST, "rmi4_rst");
		if (retval) {
			pr_err("%s: Failed to get attn gpio %d. Code: %d.",
			       __func__, data->gpio_number, retval);
			return retval;
		}

		retval = gpio_tlmm_config(GPIO_CFG(S2202_RST, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP,
					GPIO_CFG_8MA), GPIO_CFG_ENABLE);
		if (retval) {
			pr_err("%s: gpio_tlmm_config for %d failed\n",
				__func__, data->gpio_number);
			return retval;
		}

		gpio_set_value(S2202_RST, 1);

		retval = gpio_request(data->gpio_number, "rmi4_attn");
		if (retval) {
			pr_err("%s: Failed to get attn gpio %d. Code: %d.",
			       __func__, data->gpio_number, retval);
			return retval;
		}

		retval = gpio_tlmm_config(GPIO_CFG(data->gpio_number, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,
					GPIO_CFG_8MA), GPIO_CFG_ENABLE);
		if (retval) {
			pr_err("%s: gpio_tlmm_config for %d failed\n",
				__func__, data->gpio_number);
			return retval;
		}

		gpio_set_value(data->gpio_number, 1);
//		retval = gpio_direction_input(data->gpio_number);
//		omap_mux_init_signal(data->gpio_name, OMAP_PIN_INPUT_PULLUP);
//		retval = gpio_direction_input(data->gpio_number);
#if 0
		if (retval) {
			pr_err("%s: Failed to setup attn gpio %d. Code: %d.",
			       __func__, data->gpio_number, retval);
			gpio_free(data->gpio_number);
		}
#endif
	} else {
		pr_warn("%s: No way to deconfigure gpio %d.",
		       __func__, data->gpio_number);
	}

	return retval;
}

static ssize_t s2202_virtual_keys_show(struct kobject *kobj,
		     struct kobj_attribute *attr, char *buf)
{
       char *virtual_keys = __stringify(EV_KEY) ":" __stringify(KEY_BACK)  ":80:890:120:80"  \
                     ":" __stringify(EV_KEY) ":" __stringify(KEY_HOME)  ":240:890:120:80" \
                     ":" __stringify(EV_KEY) ":" __stringify(KEY_MENU)  ":400:890:120:80" "\n";
	return snprintf(buf, strnlen(virtual_keys, MAX_LEN) + 1 , "%s",
			virtual_keys);

}

static struct kobj_attribute s2202_virtual_keys_attr = {
	.attr = {
		.name = "virtualkeys.sensor00fn11",
		.mode = S_IRUGO,
	},
	.show = &s2202_virtual_keys_show,
};


static struct attribute *s2202_key_properties_attrs[] = {
	&s2202_virtual_keys_attr.attr,
	NULL
};

static struct attribute_group s2202_key_properties_attr_group = {
	.attrs = s2202_key_properties_attrs,
};

static int s2202_virtual_key_properties(struct kobject * kobj)
{
	int retval = 0;

	if (kobj)
		retval = sysfs_create_group(kobj,
				&s2202_key_properties_attr_group);
	if (!kobj || retval)
		pr_err("failed to create ft5202 board_properties\n");

	return retval;
}

static struct syna_gpio_data s2202_gpiodata = {
	.gpio_number = S2202_ATTN,
	.gpio_name = "s2202_irq",
};

static struct rmi_device_platform_data s2202_platformdata = {
	.sensor_name = "s2202_ts",
	.driver_name = "rmi_generic",
	.attn_gpio = S2202_ATTN,
	.attn_polarity = RMI_ATTN_ACTIVE_LOW,
	.gpio_data = &s2202_gpiodata,
	.gpio_config = s2202_touchpad_gpio_setup,
	.gpio_unconfig = s2202_touchpad_gpio_unconfig,
//	.f1a_button_map = &s2202_f1a_button_map,
//	.virtualbutton_map = &s2202_virtualbutton_map,
};
static struct i2c_board_info s2202_ts_devices_info[] __initdata = {
    {
        I2C_BOARD_INFO("rmi_i2c", S2202_ADDR),
        .platform_data = &s2202_platformdata,
    },
};
#endif

#ifdef CONFIG_TOUCHSCREEN_ATMEL_MXT154
#define MXT_GPIO_INT 48
#define MXT_GPIO_RST 26

#define __pwhs  _pwhs(1008,100,40)
#define _ms_     __stringify(55) // 5-105
#define _hs_     __stringify(200) //150-250
#define _bs_     __stringify(360) //
#define _ss_     __stringify(500)
static ssize_t atmel_virtual_keys_show(struct kobject *kobj,
		     struct kobj_attribute *attr, char *buf)
{

    char *virtual_keys = __stringify(EV_KEY) ":" __stringify(KEY_MENU)  ":"_ms_":"  __pwhs  \
                     ":" __stringify(EV_KEY) ":" __stringify(KEY_HOME)  ":"_hs_":" __pwhs \
                     ":" __stringify(EV_KEY) ":" __stringify(KEY_BACK)  ":"_bs_":" __pwhs \
                     ":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH)":"_ss_":" _pwhs(1008,80,40) "\n";

	return snprintf(buf, strnlen(virtual_keys, MAX_LEN) + 1 , "%s",
			virtual_keys);

}

static struct kobj_attribute atmel_virtual_keys_attr = {
	.attr = {
		.name = "virtualkeys.maxtouch-ts154",
		.mode = S_IRUGO,
	},
	.show = &atmel_virtual_keys_show,
};


static struct attribute *atmel_key_properties_attrs[] = {
	&atmel_virtual_keys_attr.attr,
	NULL
};

static struct attribute_group atmel_key_properties_attr_group = {
	.attrs = atmel_key_properties_attrs,
};

static int atmel_virtual_key_properties(struct kobject * kobject)
{
	int retval = 0;

	if (kobject)
		retval = sysfs_create_group(kobject,
				&atmel_key_properties_attr_group);
	if (retval)
		pr_err("atmel:failed to create  board_properties\n");

	return retval;
}

void atmel_gpio_uninit(void)
{
    gpio_free(MXT_GPIO_INT) ;
    gpio_free(MXT_GPIO_RST) ;
}
int atmel_gpio_init(void)
{
    int ret ;

    ret = gpio_request(MXT_GPIO_RST, "rmi4_rst");
    if (ret) {
    pr_err("%s: Failed to get attn gpio %d. Code: %d.",
           __func__, MXT_GPIO_INT, ret);
    return ret;
    }

    ret = gpio_tlmm_config(GPIO_CFG(MXT_GPIO_RST, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP,
    		GPIO_CFG_8MA), GPIO_CFG_ENABLE);
    if (ret) {
    pr_err("%s: gpio_tlmm_config for %d failed\n",
    	__func__, MXT_GPIO_RST);
    return ret;
    }

    gpio_set_value(MXT_GPIO_RST, 1);

    ret = gpio_request(MXT_GPIO_INT, "rmi4_attn");
    if (ret) {
    pr_err("%s: Failed to get attn gpio %d. Code: %d.",
           __func__, MXT_GPIO_INT, ret);
    return ret;
    }

    ret = gpio_tlmm_config(GPIO_CFG(MXT_GPIO_INT, 0, GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,
    		GPIO_CFG_8MA), GPIO_CFG_ENABLE);
    if(ret) {
    pr_err("%s: gpio_tlmm_config for %d failed\n",
    	__func__, MXT_GPIO_INT);
    return ret;
    }

    gpio_set_value(MXT_GPIO_INT, 1);

  return 0 ;
}

static int mxt154_key_codes[MXT_KEYARRAY_MAX_KEYS] = {
	[0] = KEY_HOME,
	[1] = KEY_MENU,
	[9] = KEY_BACK,
	[10] = KEY_SEARCH,
};

static struct mxt_platform_data mxt154_platform_data = {
	//.config_array		= mxt_config_array,
	//.config_array_size	= ARRAY_SIZE(mxt_config_array),
	.panel_minx		= 0,
	.panel_maxx		= 540,
	.panel_miny		= 0,
	.panel_maxy		= 1023,
	.disp_minx		= 0,
	.disp_maxx		= 540,
	.disp_miny		= 0,
	.disp_maxy		= 960,
	.gpio_rst		= MXT_GPIO_RST,
	.gpio_irq		= MXT_GPIO_INT,
	.gpio_init      = atmel_gpio_init,
	.gpio_uninit    = atmel_gpio_uninit,
	.key_codes		= mxt154_key_codes,
};

static struct i2c_board_info atmel_ts_devices_info[] __initdata = {
	{
		I2C_BOARD_INFO(ATMEL_MXT154_NAME, 0x4b),
		.platform_data = &mxt154_platform_data,
		.irq = MSM_GPIO_TO_INT(MXT_GPIO_INT),
	},
};

#endif


#ifdef CONFIG_TOUCHSCREEN_HX8526A
#define __pwhs  _pwhs(1008,100,40)
#define _ms_     __stringify(55) // 5-105
#define _hs_     __stringify(200) //150-250
#define _bs_     __stringify(360) //
#define _ss_     __stringify(500)
static ssize_t himax_virtual_keys_show(struct kobject *kobj,
		     struct kobj_attribute *attr, char *buf)
{

    char *virtual_keys = __stringify(EV_KEY) ":" __stringify(KEY_MENU)  ":"_ms_":"  __pwhs  \
                     ":" __stringify(EV_KEY) ":" __stringify(KEY_HOME)  ":"_hs_":" __pwhs \
                     ":" __stringify(EV_KEY) ":" __stringify(KEY_BACK)  ":"_bs_":" __pwhs \
                     ":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH)":"_ss_":" _pwhs(1008,80,40) "\n";

	return snprintf(buf, strnlen(virtual_keys, MAX_LEN) + 1 , "%s",
			virtual_keys);

}

static struct kobj_attribute himax_virtual_keys_attr = {
	.attr = {
		.name = "virtualkeys.himax-touchscreen",
		.mode = S_IRUGO,
	},
	.show = &himax_virtual_keys_show,
};


static struct attribute *himax_key_properties_attrs[] = {
	&himax_virtual_keys_attr.attr,
	NULL
};

static struct attribute_group himax_key_properties_attr_group = {
	.attrs = himax_key_properties_attrs,
};

static int himax_virtual_key_properties(struct kobject * kobject)
{
	int retval = 0;

	if (kobject)
		retval = sysfs_create_group(kobject,
				&himax_key_properties_attr_group);
	if (retval)
		pr_err("Himax:failed to create  board_properties\n");

	return retval;
}

static struct himax_platform_data himax_platformdata = {
    .gpio_int = HX8526A_INT,
    .gpio_id  = HX8526A_ID ,
    .gpio_rst = HX8526A_RST,
};
static struct i2c_board_info himax_ts_devices_info[] __initdata = {
    {
        I2C_BOARD_INFO("himax_ts", HX8526A_ADDR),
        .platform_data = &himax_platformdata,
    },
};
#endif /* HIMAX */

#if defined(CONFIG_TOUCHSCREEN_SYNAPTICS_RMI4_I2C) || \
defined(CONFIG_TOUCHSCREEN_SYNAPTICS_RMI4_I2C_MODULE)
static int synaptics_touchpad_setup(void);
struct kobject *virtual_key_properties_kobj;

static struct msm_gpio clearpad3000_cfg_data[] = {
	{GPIO_CFG(CLEARPAD3000_ATTEN_GPIO, 0, GPIO_CFG_INPUT,
			GPIO_CFG_NO_PULL, GPIO_CFG_6MA), "rmi4_attn"},
	{GPIO_CFG(CLEARPAD3000_RESET_GPIO, 0, GPIO_CFG_OUTPUT,
			GPIO_CFG_PULL_DOWN, GPIO_CFG_8MA), "rmi4_reset"},
};

static struct rmi_XY_pair rmi_offset = {.x = 0, .y = 0};
static struct rmi_range rmi_clipx = {.min = 48, .max = 980};
static struct rmi_range rmi_clipy = {.min = 7, .max = 1647};
static struct rmi_f11_functiondata synaptics_f11_data = {
	.swap_axes = false,
	.flipX = false,
	.flipY = false,
	.offset = &rmi_offset,
	.button_height = 113,
	.clipX = &rmi_clipx,
	.clipY = &rmi_clipy,
};

static ssize_t clearpad3000_virtual_keys_register(struct kobject *kobj,
		     struct kobj_attribute *attr, char *buf)
{
	char *virtual_keys = __stringify(EV_KEY) ":" __stringify(KEY_MENU) \
			     ":60:830:120:60" ":" __stringify(EV_KEY) \
			     ":" __stringify(KEY_HOME)   ":180:830:120:60" \
				":" __stringify(EV_KEY) ":" \
				__stringify(KEY_SEARCH) ":300:830:120:60" \
				":" __stringify(EV_KEY) ":" \
			__stringify(KEY_BACK)   ":420:830:120:60" "\n";

	return snprintf(buf, strnlen(virtual_keys, MAX_LEN) + 1 , "%s",
			virtual_keys);
}

static struct kobj_attribute clearpad3000_virtual_keys_attr = {
	.attr = {
		.name = "virtualkeys.sensor00fn11",
		.mode = S_IRUGO,
	},
	.show = &clearpad3000_virtual_keys_register,
};

static struct attribute *virtual_key_properties_attrs[] = {
	&clearpad3000_virtual_keys_attr.attr,
	NULL
};

static struct attribute_group virtual_key_properties_attr_group = {
	.attrs = virtual_key_properties_attrs,
};

static struct rmi_functiondata synaptics_functiondata[] = {
	{
		.function_index = RMI_F11_INDEX,
		.data = &synaptics_f11_data,
	},
};

static struct rmi_functiondata_list synaptics_perfunctiondata = {
	.count = ARRAY_SIZE(synaptics_functiondata),
	.functiondata = synaptics_functiondata,
};

static struct rmi_sensordata synaptics_sensordata = {
	.perfunctiondata = &synaptics_perfunctiondata,
	.rmi_sensor_setup	= synaptics_touchpad_setup,
};

static struct rmi_i2c_platformdata synaptics_platformdata = {
	.i2c_address = 0x2c,
	.irq_type = IORESOURCE_IRQ_LOWLEVEL,
	.sensordata = &synaptics_sensordata,
};

static struct i2c_board_info synaptic_i2c_clearpad3k[] = {
	{
	I2C_BOARD_INFO("rmi4_ts", 0x2c),
	.platform_data = &synaptics_platformdata,
	},
};

static int synaptics_touchpad_setup(void)
{
	int retval = 0;

	virtual_key_properties_kobj =
		kobject_create_and_add("board_properties", NULL);
	if (virtual_key_properties_kobj)
		retval = sysfs_create_group(virtual_key_properties_kobj,
				&virtual_key_properties_attr_group);
	if (!virtual_key_properties_kobj || retval)
		pr_err("failed to create ft5202 board_properties\n");

	retval = msm_gpios_request_enable(clearpad3000_cfg_data,
		    sizeof(clearpad3000_cfg_data)/sizeof(struct msm_gpio));
	if (retval) {
		pr_err("%s:Failed to obtain touchpad GPIO %d. Code: %d.",
				__func__, CLEARPAD3000_ATTEN_GPIO, retval);
		retval = 0; /* ignore the err */
	}
	synaptics_platformdata.irq = gpio_to_irq(CLEARPAD3000_ATTEN_GPIO);

	gpio_set_value(CLEARPAD3000_RESET_GPIO, 0);
	usleep(10000);
	gpio_set_value(CLEARPAD3000_RESET_GPIO, 1);
	usleep(50000);

	return retval;
}
#endif

static struct regulator_bulk_data regs_atmel[] = {
	{ .supply = "ldo12", .min_uV = 2700000, .max_uV = 3300000 },
	{ .supply = "smps3", .min_uV = 1800000, .max_uV = 1800000 },
};

#define ATMEL_TS_GPIO_IRQ 82

static int atmel_ts_power_on(bool on)
{
	int rc = on ?
		regulator_bulk_enable(ARRAY_SIZE(regs_atmel), regs_atmel) :
		regulator_bulk_disable(ARRAY_SIZE(regs_atmel), regs_atmel);

	if (rc)
		pr_err("%s: could not %sable regulators: %d\n",
				__func__, on ? "en" : "dis", rc);
	else
		msleep(50);

	return rc;
}

static int atmel_ts_platform_init(struct i2c_client *client)
{
	int rc;
	struct device *dev = &client->dev;

	rc = regulator_bulk_get(dev, ARRAY_SIZE(regs_atmel), regs_atmel);
	if (rc) {
		dev_err(dev, "%s: could not get regulators: %d\n",
				__func__, rc);
		goto out;
	}

	rc = regulator_bulk_set_voltage(ARRAY_SIZE(regs_atmel), regs_atmel);
	if (rc) {
		dev_err(dev, "%s: could not set voltages: %d\n",
				__func__, rc);
		goto reg_free;
	}

	rc = gpio_tlmm_config(GPIO_CFG(ATMEL_TS_GPIO_IRQ, 0,
				GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,
				GPIO_CFG_8MA), GPIO_CFG_ENABLE);
	if (rc) {
		dev_err(dev, "%s: gpio_tlmm_config for %d failed\n",
			__func__, ATMEL_TS_GPIO_IRQ);
		goto reg_free;
	}

	/* configure touchscreen interrupt gpio */
	rc = gpio_request(ATMEL_TS_GPIO_IRQ, "atmel_maxtouch_gpio");
	if (rc) {
		dev_err(dev, "%s: unable to request gpio %d\n",
			__func__, ATMEL_TS_GPIO_IRQ);
		goto ts_gpio_tlmm_unconfig;
	}

	rc = gpio_direction_input(ATMEL_TS_GPIO_IRQ);
	if (rc < 0) {
		dev_err(dev, "%s: unable to set the direction of gpio %d\n",
			__func__, ATMEL_TS_GPIO_IRQ);
		goto free_ts_gpio;
	}
	return 0;

free_ts_gpio:
	gpio_free(ATMEL_TS_GPIO_IRQ);
ts_gpio_tlmm_unconfig:
	gpio_tlmm_config(GPIO_CFG(ATMEL_TS_GPIO_IRQ, 0,
				GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_2MA), GPIO_CFG_DISABLE);
reg_free:
	regulator_bulk_free(ARRAY_SIZE(regs_atmel), regs_atmel);
out:
	return rc;
}

static int atmel_ts_platform_exit(struct i2c_client *client)
{
	gpio_free(ATMEL_TS_GPIO_IRQ);
	gpio_tlmm_config(GPIO_CFG(ATMEL_TS_GPIO_IRQ, 0,
				GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_2MA), GPIO_CFG_DISABLE);
	regulator_bulk_free(ARRAY_SIZE(regs_atmel), regs_atmel);
	return 0;
}

static u8 atmel_ts_read_chg(void)
{
	return gpio_get_value(ATMEL_TS_GPIO_IRQ);
}

static u8 atmel_ts_valid_interrupt(void)
{
	return !atmel_ts_read_chg();
}


static struct maxtouch_platform_data atmel_ts_pdata = {
	.numtouch = 4,
	.init_platform_hw = atmel_ts_platform_init,
	.exit_platform_hw = atmel_ts_platform_exit,
	.power_on = atmel_ts_power_on,
	.display_res_x = 480,
	.display_res_y = 864,
	.min_x = ATMEL_X_OFFSET,
	.max_x = (505 - ATMEL_X_OFFSET),
	.min_y = ATMEL_Y_OFFSET,
	.max_y = (863 - ATMEL_Y_OFFSET),
	.valid_interrupt = atmel_ts_valid_interrupt,
	.read_chg = atmel_ts_read_chg,
};

static struct i2c_board_info atmel_ts_i2c_info[] __initdata = {
	{
		I2C_BOARD_INFO(ATMEL_TS_I2C_NAME, 0),
		.platform_data = &atmel_ts_pdata,
		.irq = MSM_GPIO_TO_INT(ATMEL_TS_GPIO_IRQ),
	},
};

static struct msm_handset_platform_data hs_platform_data = {
	.hs_name = "7k_handset",
	.pwr_key_delay_ms = 500, /* 0 will disable end key */
};

static struct platform_device hs_pdev = {
	.name   = "msm-handset",
	.id     = -1,
	.dev    = {
		.platform_data = &hs_platform_data,
	},
};


#ifdef CONFIG_LEDS_TRICOLOR_FLAHSLIGHT

#define LED_FLASH_EN1 13
#define QRD7_LED_FLASH_EN 96

static struct msm_gpio tricolor_leds_gpio_cfg_data[] = {
{
	GPIO_CFG(-1, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
		"flashlight"},
};

static int tricolor_leds_gpio_setup(void) {
	int ret = 0;

	ret = msm_gpios_request_enable(tricolor_leds_gpio_cfg_data,
			sizeof(tricolor_leds_gpio_cfg_data)/sizeof(struct msm_gpio));
	if( ret<0 )
		printk(KERN_ERR "%s: Failed to obtain tricolor_leds GPIO . Code: %d\n",
				__func__, ret);
	return ret;
}


static struct platform_device msm_device_tricolor_leds = {
	.name   = "tricolor leds and flashlight",
	.id = -1,
};
#endif



static struct gpio_led gpio_leds_data[] = {
	{
		.name = "button-backlight",
		.gpio = 9,
	},
};

static struct gpio_led_platform_data gpio_leds_pdata = {
	.leds = gpio_leds_data,
       .num_leds = 1,
};

static struct platform_device gpio_leds_pdev = {
	.name   = "leds-gpio",//"gpio-leds",
	.id     = -1,
	.dev    = {
		.platform_data	= &gpio_leds_pdata,
	},
};

//end

char CTP_Panel_manufacturer;
int __init register_tp_devices(void)
{
    	struct kobject *kobj=NULL;

    printk("Register touchscreen devices for Cellon Phone.\n");
	kobj = kobject_create_and_add("board_properties", NULL);
    if(!kobj){
        printk(KERN_ERR"create TP board properties error\n");
        return -ENODEV;
    }
#if defined(CONFIG_RMI4_I2C) || defined(CONFIG_RMI4_SPI)
        i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
                    s2202_ts_devices_info,
                    ARRAY_SIZE(s2202_ts_devices_info));
        s2202_virtual_key_properties(kobj);
#endif

#if defined(CONFIG_TOUCHSCREEN_HX8526A)
        i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
                    himax_ts_devices_info,
                    ARRAY_SIZE(himax_ts_devices_info));
        himax_virtual_key_properties(kobj);
#endif
#if defined(CONFIG_TOUCHSCREEN_ATMEL_MXT154)
        i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
                    atmel_ts_devices_info,
                    ARRAY_SIZE(atmel_ts_devices_info));
        atmel_virtual_key_properties(kobj);
#endif
    return 0 ;
}
void __init msm7627a_add_io_devices(void)
{
	/* touchscreen */
	atmel_ts_pdata.min_x = 0;
	atmel_ts_pdata.max_x = 480;
	atmel_ts_pdata.min_y = 0;
	atmel_ts_pdata.max_y = 320;

    register_tp_devices();
#ifdef CONFIG_NFC_DEVICES
    i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
                c8681_nfc_pn544_info,
                ARRAY_SIZE(c8681_nfc_pn544_info));
    printk("register pn544 i2c device success\n");
#endif
	i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
				atmel_ts_i2c_info,
				ARRAY_SIZE(atmel_ts_i2c_info));
	/* keypad */
	platform_device_register(&kp_pdev_8625);
	platform_device_register(&gpio_leds_pdev);

	/* Vibrator */
	msm_init_pmic_vibrator();

	/* headset */
	platform_device_register(&hs_pdev);
#ifdef CONFIG_LEDS_TRICOLOR_FLAHSLIGHT
	platform_device_register(&msm_device_tricolor_leds);
	tricolor_leds_gpio_setup();
#endif

}

void __init qrd7627a_add_io_devices(void)
{
	int rc;

	/* touchscreen */


			mxt_touchpad_setup();
		rc = gpio_tlmm_config(GPIO_CFG(MXT_TS_IRQ_GPIO, 0,
				GPIO_CFG_INPUT, GPIO_CFG_PULL_UP,
				GPIO_CFG_8MA), GPIO_CFG_ENABLE);
		if (rc) {
			pr_err("%s: gpio_tlmm_config for %d failed\n",
				__func__, MXT_TS_IRQ_GPIO);
		}

		rc = gpio_tlmm_config(GPIO_CFG(MXT_TS_RESET_GPIO, 0,
				GPIO_CFG_OUTPUT, GPIO_CFG_PULL_DOWN,
				GPIO_CFG_8MA), GPIO_CFG_ENABLE);
		if (rc) {
			pr_err("%s: gpio_tlmm_config for %d failed\n",
				__func__, MXT_TS_RESET_GPIO);
		}

		i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
					mxt_device_info,
					ARRAY_SIZE(mxt_device_info));

	/* headset */
	platform_device_register(&hs_pdev);

	/* vibrator */
#ifdef CONFIG_MSM_RPC_VIBRATOR
	msm_init_pmic_vibrator();
#endif

#ifdef CONFIG_LEDS_TRICOLOR_FLAHSLIGHT
	    /*tricolor leds init*/
	platform_device_register(&msm_device_tricolor_leds);
	tricolor_leds_gpio_setup();
#endif
}
