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
#define DEBUG

#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/bootmem.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <mach/msm_bus_board.h>
#include <mach/msm_memtypes.h>
#include <mach/board.h>
#include <mach/gpiomux.h>
#include <mach/socinfo.h>
#include <mach/rpc_pmapp.h>
#include "../devices.h"
#include "board-cellon-C8680.h"
#include <mach/msm_iomap.h>
#include <mach/msm_smsm.h>

#ifdef CONFIG_FB_MSM_TRIPLE_BUFFER
#define MSM_FB_SIZE		0x4BF000
#define MSM7x25A_MSM_FB_SIZE    0x1C2000
#define MSM8x25_MSM_FB_SIZE	0x5FA000
#else
#define MSM_FB_SIZE		0x32A000
#define MSM7x25A_MSM_FB_SIZE	0x12C000
#define MSM8x25_MSM_FB_SIZE	0x3FC000
#endif

#define GPIO_C8680_LCD_RESET_N	 85
#define GPIO_C8680_LCD_BACKLIGHT_EN 96
#define GPIO_C8680_LCD_ID 118

static struct regulator_bulk_data regs_lcdc[] = {
	{ .supply = "gp2",   .min_uV = 2850000, .max_uV = 2850000 },
	{ .supply = "msme1", .min_uV = 1800000, .max_uV = 1800000 },
};

static uint32_t lcdc_gpio_initialized;

static int msm_fb_lcdc_power_save(int on)
{
	int rc = 0;
	/* Doing the init of the LCDC GPIOs very late as they are from
		an I2C-controlled IO Expander */

	if (lcdc_gpio_initialized) {
		gpio_set_value_cansleep(GPIO_DISPLAY_PWR_EN, on);
		gpio_set_value_cansleep(GPIO_BACKLIGHT_EN, on);

		rc = on ? regulator_bulk_enable(
				ARRAY_SIZE(regs_lcdc), regs_lcdc) :
			  regulator_bulk_disable(
				ARRAY_SIZE(regs_lcdc), regs_lcdc);

		if (rc)
			pr_err("%s: could not %sable regulators: %d\n",
					__func__, on ? "en" : "dis", rc);
	}

	return rc;
}

static int msm_lcdc_power_save(int on)
{
	int rc = 0;
	rc = msm_fb_lcdc_power_save(on);

	return rc;
}

static struct lcdc_platform_data lcdc_pdata = {
	.lcdc_gpio_config = NULL,
	.lcdc_power_save   = msm_lcdc_power_save,
};


static struct resource msm_fb_resources[] = {
	{
		.flags  = IORESOURCE_DMA,
	}
};

#ifdef CONFIG_MSM_V4L2_VIDEO_OVERLAY_DEVICE
static struct resource msm_v4l2_video_overlay_resources[] = {
	{
		.flags = IORESOURCE_DMA,
	}
};
#endif

typedef enum {
	MANUFACTURER_TIANMA,
	MANUFACTURER_BYD,
	MANUFACTURER_BOE,
	MANUFACTURER_CHIMEI,
	MANUFACTURER_TRULY,
	MANUFACTURER_SHARP,
	MANUFACTURER_UNKOWN,
} PANEL_MANUFACTURER;

typedef enum {
	C8680,
} CELLON_PROJECT_NAME;

PANEL_MANUFACTURER c8680_panel_manufacturer;

static int c8680_detect_lcd_panel(void)
{
	int rc = 0;
	int lcd_id = 0;

	rc = gpio_request(GPIO_C8680_LCD_ID, "c8680_lcd_id");
	if (rc < 0) {
		pr_err("%s: gpio_request c8680_lcd_id failed!",
				__func__);
		return rc;
	}
	rc = gpio_tlmm_config(GPIO_CFG(GPIO_C8680_LCD_ID, 0,
				GPIO_CFG_INPUT, GPIO_CFG_NO_PULL,
				GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	if (rc < 0) {
		pr_err("%s: unable to config c8680_lcd_id\n", __func__);
		gpio_free(GPIO_C8680_LCD_ID);
		return rc;
	}
	lcd_id = gpio_get_value(GPIO_C8680_LCD_ID);
	gpio_free(GPIO_C8680_LCD_ID);

	if (lcd_id == 1)
		c8680_panel_manufacturer = MANUFACTURER_TRULY;
	else if (lcd_id == 0)
		c8680_panel_manufacturer = MANUFACTURER_SHARP;
	else
		c8680_panel_manufacturer = MANUFACTURER_UNKOWN;

	printk("c8680 panel_manufacturer = %d\n", c8680_panel_manufacturer);

	return rc;

}

char Lcd_Panel_manufacturer;
char cellon_project_name;

static int cellon_auto_detect_lcd_panel(const char *name)
{
	int ret = -ENODEV;

	if (c8680_panel_manufacturer == MANUFACTURER_TRULY) {
		if (!strcmp(name, "mipi_cmd_otm9608a_qhd"))
			ret = 0;
	} else if (c8680_panel_manufacturer == MANUFACTURER_SHARP) {
		if (!strcmp(name, "mipi_cmd_nt35565_qhd"))
			ret = 0;
	}

	Lcd_Panel_manufacturer = c8680_panel_manufacturer;
	cellon_project_name = C8680;

	return ret;
}

static int msm_fb_detect_panel(const char *name)
{
	int ret = -ENODEV;

	printk("%s: %d\n", __func__, __LINE__);

		ret = cellon_auto_detect_lcd_panel(name);
	return ret;
}

static struct msm_fb_platform_data msm_fb_pdata = {
	.detect_client = msm_fb_detect_panel,
};

static struct platform_device msm_fb_device = {
	.name   = "msm_fb",
	.id     = 0,
	.num_resources  = ARRAY_SIZE(msm_fb_resources),
	.resource       = msm_fb_resources,
	.dev    = {
		.platform_data = &msm_fb_pdata,
	}
};

#ifdef CONFIG_MSM_V4L2_VIDEO_OVERLAY_DEVICE
static struct platform_device msm_v4l2_video_overlay_device = {
		.name   = "msm_v4l2_overlay_pd",
		.id     = 0,
		.num_resources  = ARRAY_SIZE(msm_v4l2_video_overlay_resources),
		.resource       = msm_v4l2_video_overlay_resources,
	};
#endif

static struct msm_panel_common_pdata mipi_NT35516_pdata = {
	.pmic_backlight = NULL,
};

static struct platform_device mipi_dsi_NT35516_panel_device = {
	.name   = "mipi_NT35516",
	.id     = 0,
	.dev    = {
		.platform_data = &mipi_NT35516_pdata,
	}
};

static struct msm_panel_common_pdata mipi_NT35565_pdata = {
	.backlight_level = NULL,
};

static struct platform_device mipi_dsi_NT35565_panel_device = {
	.name = "mipi_NT35565",
	.id = 0,
	.dev = {
		.platform_data = &mipi_NT35565_pdata,
	}
};
static struct platform_device *c8680_fb_devices[] __initdata = {
	&msm_fb_device,
	&mipi_dsi_NT35565_panel_device,
	&mipi_dsi_NT35516_panel_device,
};

void __init msm_msm7627a_allocate_memory_regions(void)
{
	void *addr;
	unsigned long fb_size;

	fb_size = MSM8x25_MSM_FB_SIZE;

	addr = alloc_bootmem_align(fb_size, 0x1000);
	msm_fb_resources[0].start = __pa(addr);
	msm_fb_resources[0].end = msm_fb_resources[0].start + fb_size - 1;
	pr_info("allocating %lu bytes at %p (%lx physical) for fb\n", fb_size,
						addr, __pa(addr));

#ifdef CONFIG_MSM_V4L2_VIDEO_OVERLAY_DEVICE
	fb_size = MSM_V4L2_VIDEO_OVERLAY_BUF_SIZE;
	addr = alloc_bootmem_align(fb_size, 0x1000);
	msm_v4l2_video_overlay_resources[0].start = __pa(addr);
	msm_v4l2_video_overlay_resources[0].end =
		msm_v4l2_video_overlay_resources[0].start + fb_size - 1;
	pr_debug("allocating %lu bytes at %p (%lx physical) for v4l2\n",
		fb_size, addr, __pa(addr));
#endif

}

static struct msm_panel_common_pdata mdp_pdata = {
	.gpio = 97,
	.mdp_rev = MDP_REV_303,
	.cont_splash_enabled = 0,
};

#define GPIO_LCDC_BRDG_PD	128
#define GPIO_LCDC_BRDG_RESET_N	129
#define GPIO_LCD_DSI_SEL	125
#define LCDC_RESET_PHYS		0x90008014
#ifdef CONFIG_FB_MSM_MIPI_DSI

#endif

enum {
	DSI_SINGLE_LANE = 1,
	DSI_TWO_LANES,
};

#ifdef CONFIG_FB_MSM_MIPI_DSI
static int msm_fb_get_lane_config(void)
{
	/* For MSM7627A SURF/FFA and QRD */
	int rc = DSI_TWO_LANES;
	pr_info("DSI_TWO_LANES\n");

	return rc;
}

static int msm_fb_dsi_client_c8680_reset(void)
{
	int rc = 0;
	unsigned smem_size;
	unsigned int boot_reason=0;
	int lcd_en=1;
	printk("%s: %d\n", __func__, __LINE__);

	rc = gpio_request(GPIO_C8680_LCD_BACKLIGHT_EN, "gpio_bkl_en");
	if (rc < 0)
		return rc;

	rc = gpio_tlmm_config(GPIO_CFG(GPIO_C8680_LCD_BACKLIGHT_EN, 0,
		GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
		GPIO_CFG_ENABLE);
	if (rc < 0) {
		pr_err("failed GPIO_BACKLIGHT_EN tlmm config\n");
		return rc;
	}

	 //add by fengxiaoli to turn off the lcd when power off rtc
	boot_reason= *(unsigned int *)
                 (smem_get_entry(SMEM_POWER_ON_STATUS_INFO, &smem_size));
	printk(KERN_NOTICE "Boot Reason = %d\n",boot_reason);
	if (boot_reason==2)
	     lcd_en=0;
	 //end add
	 rc = gpio_direction_output(GPIO_C8680_LCD_BACKLIGHT_EN, lcd_en);
	if (rc < 0) {
		pr_err("failed to enable backlight\n");
		gpio_free(QRD_GPIO_BACKLIGHT_EN);
		return rc;
	}

	c8680_detect_lcd_panel();

	return rc;
}

static int msm_fb_dsi_client_cellon_reset(void)
{
	int rc = 0;

	rc = msm_fb_dsi_client_c8680_reset();

	return rc;
}

static int msm_fb_dsi_client_reset(void)
{
	int rc = 0;

	rc = msm_fb_dsi_client_cellon_reset();

	return rc;
}

bool kernel_booted = 0;

static int mipi_dsi_panel_c8680_power(int on)
{
	int rc = 0;

	printk("%s: on = %d\n", __func__, on);

	rc = gpio_request(GPIO_C8680_LCD_RESET_N, "c8680_lcdc_reset_n");
	if (rc < 0) {
		pr_err("failed to request c8680 lcd reset_n\n");
		return rc;
	}

	rc = gpio_tlmm_config(GPIO_CFG(GPIO_C8680_LCD_RESET_N, 0,
			GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA),
			GPIO_CFG_ENABLE);
	if (rc < 0) {
		pr_err("Failed to enable LCD reset\n");
		gpio_free(GPIO_LCDC_BRDG_RESET_N);
		return rc;
	}

	rc = gpio_direction_output(GPIO_C8680_LCD_RESET_N, 1);
	if (rc < 0) {
		pr_err("Failed to set reset invalid\n");
		return rc;
	}

	if (!kernel_booted) {
		gpio_set_value(GPIO_C8680_LCD_BACKLIGHT_EN, 0);
		printk("%s: %d\n", __func__, __LINE__);
	}

	if (on) {
		//both sharp and truly have the same reset timing
		gpio_set_value_cansleep(GPIO_C8680_LCD_RESET_N, 0);
		udelay(50);
		gpio_set_value_cansleep(GPIO_C8680_LCD_RESET_N, 1);
		msleep(20);
	}
	else
	{	
		//must reset IC OTM9608A, otherwise, the phone will waste 3mA when sleep.
		rc = gpio_direction_output(GPIO_C8680_LCD_RESET_N, 0);
		if (rc < 0) {
			pr_err("Failed to set reset invalid\n");
			return rc;
		}
	}

	gpio_free(GPIO_C8680_LCD_RESET_N);

	return rc;
}

static int mipi_dsi_panel_cellon_power(int on)
{
	int rc = 0;
	rc = mipi_dsi_panel_c8680_power(on);
	return rc;
}

static int mipi_dsi_panel_power(int on)
{
	int rc = 0;
	rc = mipi_dsi_panel_cellon_power(on);
	return rc;
}
#endif

#define MDP_303_VSYNC_GPIO 97

static char mipi_dsi_splash_is_enabled(void) {
    return mdp_pdata.cont_splash_enabled;
}

#ifdef CONFIG_FB_MSM_MIPI_DSI
static struct mipi_dsi_platform_data mipi_dsi_pdata = {
	.vsync_gpio		= MDP_303_VSYNC_GPIO,
	.dsi_power_save		= mipi_dsi_panel_power,
	.dsi_client_reset       = msm_fb_dsi_client_reset,
	.get_lane_config	= msm_fb_get_lane_config,
    .splash_is_enabled  = mipi_dsi_splash_is_enabled,
};
#endif

static char prim_panel_name[PANEL_NAME_MAX_LEN];
static int __init prim_display_setup(char *param)
{
	if (strnlen(param, PANEL_NAME_MAX_LEN))
		strlcpy(prim_panel_name, param, PANEL_NAME_MAX_LEN);
	return 0;
}
early_param("prim_display", prim_display_setup);

void msm7x27a_set_display_params(char *prim_panel)
{
	if (strnlen(prim_panel, PANEL_NAME_MAX_LEN)) {
		strlcpy(msm_fb_pdata.prim_panel_name, prim_panel,
			PANEL_NAME_MAX_LEN);
		pr_debug("msm_fb_pdata.prim_panel_name %s\n",
			msm_fb_pdata.prim_panel_name);
	}
}

void __init msm_fb_add_devices(void)
{
	msm7x27a_set_display_params(prim_panel_name);

	platform_add_devices(c8680_fb_devices,
				ARRAY_SIZE(c8680_fb_devices));

	msm_fb_register_device("mdp", &mdp_pdata);
	msm_fb_register_device("lcdc", &lcdc_pdata);
#ifdef CONFIG_FB_MSM_MIPI_DSI
	msm_fb_register_device("mipi_dsi", &mipi_dsi_pdata);
#endif
}
