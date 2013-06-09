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
#include <linux/i2c.h>
#include <linux/delay.h>


#include <asm/gpio.h>
#include <asm/mach-types.h>
#include <mach/rpc_server_handset.h>
#include <mach/pmic.h>

#include "../devices.h"
#include "board-cellon-C8680.h"
#include "../devices-msm7x2xa.h"


#ifdef CONFIG_TOUCHSCREEN_HX8526A
#include <linux/himax_ts.h>
#define HX8526A_ADDR     0x4a
#define HX8526A_RST      26
#define HX8526A_INT      48
#define HX8526A_ID       -1
#endif /*CONFIG_TOUCHSCREEN_HX8526A*/

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

#define _ws(_w)  __stringify(_w)
#define _hs(_h)  __stringify(_h)
#define _ps(_p)  __stringify(_p)
#define _pwhs(_p,_w,_h) _ps(_p)":"_ws(_w)":"_hs(_h)
/* Synaptics change for beagle board */

#define MAX_LEN		100

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

#if defined(CONFIG_TOUCHSCREEN_HX8526A)
        i2c_register_board_info(MSM_GSBI1_QUP_I2C_BUS_ID,
                    himax_ts_devices_info,
                    ARRAY_SIZE(himax_ts_devices_info));
        himax_virtual_key_properties(kobj);
#endif
    return 0 ;
}
void __init msm7627a_add_io_devices(void)
{
	/* touchscreen */
        register_tp_devices();
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

