/*
 * drivers/input/touchscreen/sitronix_i2c_st1232.h
 *
 * Touchscreen driver for Sitronix
 *
 * Copyright (C) 2011 Sitronix Technology Co., Ltd.
 *	Rudy Huang <rudy_huang@sitronix.com.tw>
 */
/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#ifndef __SITRONIX_I2C_ST1232_h
#define __SITRONIX_I2C_ST1232_h

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

#define SITRONIX_TOUCH_DRIVER_VERSION 0x01
#define SITRONIX_MAX_SUPPORTED_POINT 10
//#define SITRONIX_I2C_ST1232_NAME "st1232"
#define SITRONIX_I2C_ST1232_NAME "sitronix_ts"

#define EnableUpgradeMsg 1
#define SITRONIX_DBGMSG 1 

#ifdef EnableUpgradeMsg
#define UpgradeMsg(arg...) printk(arg)
#else
#define UpgradeMsg(arg...)
#endif

typedef enum{
	FIRMWARE_VERSION,
	STATUS_REG,
	DEVICE_CONTROL_REG,
	TIMEOUT_TO_IDLE_REG,
	XY_RESOLUTION_HIGH,
	X_RESOLUTION_LOW,
	Y_RESOLUTION_LOW,
	SENSING_COUNTER_HIGH,
	SENSING_COUNTER_LOW,
	RESERVED_0x09,
	RESERVED_0x0A,
	RESERVED_0x0B,
	FIRMWARE_REVISION_3,
	FIRMWARE_REVISION_2,
	FIRMWARE_REVISION_1,
	FIRMWARE_REVISION_0,
	FINGERS,
	KEYS_REG,
	XY0_COORD_H,
	X0_COORD_L,
	Y0_COORD_L,
	RESERVED_0x15,
	XY1_COORD,
	X1_COORD,
	Y1_COORD,
	RESERVED_0x19,
	XY2_COORD,
	X2_COORD,
	Y2_COORD,
	RESERVED_0x1D,
	XY3_COORD,
	X3_COORD,
	Y3_COORD,
	RESERVED_0x21,
	XY4_COORD,
	X4_COORD,
	Y4_COORD,
	RESERVED_0x25,
	XY5_COORD,
	X5_COORD,
	Y5_COORD,
	RESERVED_0x29,
	XY6_COORD,
	X6_COORD,
	Y6_COORD,
	RESERVED_0x2D,
	XY7_COORD,
	X7_COORD,
	Y7_COORD,
	RESERVED_0x31,
	XY8_COORD,
	X8_COORD,
	Y8_COORD,
	RESERVED_0x35,
	XY9_COORD,
	X9_COORD,
	Y9_COORD,
	RESERVED_0x39,
	RESERVED_0x3A,
	RESERVED_0x3B,
	RESERVED_0x3C,
	RESERVED_0x3D,
	RESERVED_0x3E,
	MAX_NUM_TOUCHES,
	DATA_0_HIGH,
	DATA_0_LOW,

	PAGE_REG = 0xff,
}RegisterOffset;

#define SITRONIX_AUTOTUNE_CODE_BUFFER_SIZE 15*1024
#define SITRONIX_AUTOTUNE_TABLE_BUFFER_SIZE 512
#define SITRONIX_TOUCH_PARAM_BUFFER_SIZE 512
#define SITRONIX_TOUCH_FW_BUFFER_SIZE 16*1024
#define SITRONIX_JNI_BUFFER_SIZE 1024
#define SITRONIX_TS_CHANGE_MODE_DELAY 150

#define DEV_FLASH_SIZE						0x4000
#define	DEV_FLAHS_PAGE_SIZE					512
#define TOUCH_TABLE_SIZE					0x600
#define AUTO_TUNE_CONFIG_TABLE_SIZE 	512
#define AUTO_TUNE_CONFIG_TABLE_ADDR	0x3800
#define PANEL_PARAM_TABLE_SIZE		1535
#define PANEL_PARAM_TABLE_ADDR		0x3A00
#define ST1232_ROM_PARAM_ADR 		0x3E00


#define ISP_PACKET_SIZE			8

#define ISP_CMD_ERASE			0x80
#define ISP_CMD_SEND_DATA		0x81
#define	ISP_CMD_WRITE_FLASH		0x82
#define	ISP_CMD_READ_FLASH		0x83
#define	ISP_CMD_RESET			0x84
#define	ISP_CMD_READY			0x8F

#define	I2C_REG_FW_VERSION_ADDR				0x00
#define	I2C_REG_DEV_CTRL_ADDR				0x02
#define I2C_REG_FW_REVISION_ADDR			0x0C

#define	I2C_DEV_CTRL_RESET					0x01

#define ISP_PANEL_PARAM_ADDR				0x3A00
#define	ISP_ALG_PARAM_PTR_OFFSET			0x06
#define	ISP_ALG_PARAM1_SIZE					128
#define	ISP_ALG_PARAM2_SIZE					128
#define	ISP_ALG_PARAM_SIZE					(ISP_ALG_PARAM1_SIZE+ISP_ALG_PARAM2_SIZE)
#define	ISP_ALG_PARAM2_OFFSET				(ISP_ALG_PARAM1_SIZE+32)
//======= [Auto Tune Definitions] =======
#define I2C_ATPAGE_FW_VERSION_ADDR			0x00
#define I2C_ATPAGE_STATUS_ADDR				0x01
#define I2C_ATPAGE_DEVICE_CTL_ADDR			0x02
#define	I2C_ATPAGE_AP_STATUS_ADDR			0xE0
#define	I2C_ATPAGE_AP_ERR_CODE_ADDR			0xE1
#define	I2C_ATPAGE_MSG_ID_ADDR				0xE2
#define	I2C_ATPAGE_MSG_DATA0_ADDR			0xE3
#define	I2C_ATPAGE_MSG_DATA1_ADDR			0xE4
#define	I2C_ATPAGE_MSG_DATA2_ADDR			0xE5
#define	I2C_ATPAGE_MSG_DATA3_ADDR			0xE6
#define I2C_ATPAGE_AP_PC_ADDR				0xE7
#define	I2C_ATPAGE_CURR_STEP_ADDR			0xE8
#define	I2C_ATPAGE_TOTAL_STEP_ADDR			0xE9

#define DEVICE_CTL_AUTO_TUNE				0x80

#define DEVICE_STATUS_MASK					0x0F
#define DEVICE_ERR_CODE_MASK				0xF0
#define DEVICE_ERR_CODE(X)					((X&DEVICE_ERR_CODE_MASK)>>4))

#define	DEVICE_STATUS_NORMAL				0x00
#define	DEVICE_STATUS_INIT					0x01
#define	DEVICE_STATUS_ERR					0x02

#define AT_AP_STATUS_AUTO_TUNING			0x00
#define AT_AP_STATUS_INIT					0x01
#define AT_AP_STATUS_READY					0x02
#define AT_AP_STATUS_ATUNE_FINISH			0x03
#define AT_AP_STATUS_ERROR					0xFF

typedef enum{
	XY_COORD_H,
	X_COORD_L,
	Y_COORD_L,
	RESERVED,
	PIXEL_DATA_LENGTH,
}PIXEL_DATA_FORMAT;

#define X_RES_H_SHFT 4
#define X_RES_H_BMSK 0xf
#define Y_RES_H_SHFT 0
#define Y_RES_H_BMSK 0xf
#define FINGERS_SHFT 0
#define FINGERS_BMSK 0xf
#define X_COORD_VALID_SHFT 7
#define X_COORD_VALID_BMSK 0x1
#define X_COORD_H_SHFT 4
#define X_COORD_H_BMSK 0x7
#define Y_COORD_H_SHFT 0
#define Y_COORD_H_BMSK 0x7

#define SMT_IOC_MAGIC   0xf1

typedef enum{
	SMT_GET_DRIVER_REVISION = 1,
	SMT_GET_FW_REVISION,
	SMT_WRITE_AUTO_TUNE_CODE_OFFSET,
	SMT_WRITE_AUTO_TUNE_CODE_SIZE,
	SMT_WRITE_AUTO_TUNE_CODE_DATA,
	SMT_WRITE_AUTO_TUNE_TABLE_OFFSET,
	SMT_WRITE_AUTO_TUNE_TABLE_SIZE,
	SMT_WRITE_AUTO_TUNE_TABLE_DATA,
	SMT_WRITE_TOUCH_PARAM_FILE_OFFSET,
	SMT_WRITE_TOUCH_PARAM_FILE_SIZE,
	SMT_WRITE_TOUCH_PARAM_FILE_DATA,
	SMT_WRITE_TOUCH_FW_FILE_OFFSET,
	SMT_WRITE_TOUCH_FW_FILE_SIZE,
	SMT_WRITE_TOUCH_FW_FILE_DATA,
	SMT_FW_UPGRADE_WITH_AUTO_TUNE,
	SMT_UPGRADE_TOUCH_FW_AND_PARAM,
	SMT_UPGRADE_TOUCH_FW,
	SMT_DO_AUTOTUNE,
	SMT_IOC_MAXNR,
}SITRONIX_SMT;

#define IOCTL_SMT_GET_DRIVER_REVISION				_IOC(_IOC_READ,  SMT_IOC_MAGIC, SMT_GET_DRIVER_REVISION, 			1)
#define IOCTL_SMT_GET_FW_REVISION					_IOC(_IOC_READ,  SMT_IOC_MAGIC, SMT_GET_FW_REVISION, 				4)
#define IOCTL_SMT_WRITE_AUTO_TUNE_CODE_OFFSET		_IOC(_IOC_WRITE, SMT_IOC_MAGIC, SMT_WRITE_AUTO_TUNE_CODE_OFFSET, 	2)
#define IOCTL_SMT_WRITE_AUTO_TUNE_CODE_SIZE			_IOC(_IOC_WRITE, SMT_IOC_MAGIC, SMT_WRITE_AUTO_TUNE_CODE_SIZE, 		2)
#define IOCTL_SMT_WRITE_AUTO_TUNE_CODE_DATA			_IOC(_IOC_WRITE, SMT_IOC_MAGIC, SMT_WRITE_AUTO_TUNE_CODE_DATA, 		1024)
#define IOCTL_SMT_WRITE_AUTO_TUNE_TABLE_OFFSET		_IOC(_IOC_WRITE, SMT_IOC_MAGIC, SMT_WRITE_AUTO_TUNE_TABLE_OFFSET, 	2)
#define IOCTL_SMT_WRITE_AUTO_TUNE_TABLE_SIZE		_IOC(_IOC_WRITE, SMT_IOC_MAGIC, SMT_WRITE_AUTO_TUNE_TABLE_SIZE, 	2)
#define IOCTL_SMT_WRITE_AUTO_TUNE_TABLE_DATA		_IOC(_IOC_WRITE, SMT_IOC_MAGIC, SMT_WRITE_AUTO_TUNE_TABLE_DATA, 	1024)
#define IOCTL_SMT_WRITE_TOUCH_PARAM_FILE_OFFSET		_IOC(_IOC_WRITE, SMT_IOC_MAGIC, SMT_WRITE_TOUCH_PARAM_FILE_OFFSET, 	2)
#define IOCTL_SMT_WRITE_TOUCH_PARAM_FILE_SIZE		_IOC(_IOC_WRITE, SMT_IOC_MAGIC, SMT_WRITE_TOUCH_PARAM_FILE_SIZE, 	2)
#define IOCTL_SMT_WRITE_TOUCH_PARAM_FILE_DATA		_IOC(_IOC_WRITE, SMT_IOC_MAGIC, SMT_WRITE_TOUCH_PARAM_FILE_DATA, 	1024)
#define IOCTL_SMT_WRITE_TOUCH_FW_FILE_OFFSET		_IOC(_IOC_WRITE, SMT_IOC_MAGIC, SMT_WRITE_TOUCH_FW_FILE_OFFSET, 	2)
#define IOCTL_SMT_WRITE_TOUCH_FW_FILE_SIZE			_IOC(_IOC_WRITE, SMT_IOC_MAGIC, SMT_WRITE_TOUCH_FW_FILE_SIZE, 		2)
#define IOCTL_SMT_WRITE_TOUCH_FW_FILE_DATA			_IOC(_IOC_WRITE, SMT_IOC_MAGIC, SMT_WRITE_TOUCH_FW_FILE_DATA, 		1024)
#define IOCTL_SMT_FW_UPGRADE_WITH_AUTO_TUNE			_IOC(_IOC_WRITE, SMT_IOC_MAGIC, SMT_FW_UPGRADE_WITH_AUTO_TUNE, 		15)
#define IOCTL_SMT_UPGRADE_TOUCH_FW_AND_PARAM		_IOC(_IOC_WRITE, SMT_IOC_MAGIC, SMT_UPGRADE_TOUCH_FW_AND_PARAM, 	10)
#define IOCTL_SMT_UPGRADE_TOUCH_FW			_IOC(_IOC_WRITE, SMT_IOC_MAGIC, SMT_UPGRADE_TOUCH_FW, 			5)
#define IOCTL_SMT_DO_AUTOTUNE				_IOC(_IOC_NONE,  SMT_IOC_MAGIC, SMT_DO_AUTOTUNE, 			0)

typedef struct _tag_MTD_STRUCTURE{
	unsigned char EventType;
	unsigned char PixelID;
	u16 Pixel_X;
	u16 Pixel_Y;
}MTD_STRUCTURE, *PMTD_STRUCTURE;

struct sitronix_i2c_st1232_platform_data {
	uint32_t version;	/* Use this entry for panels with */
				/* (major << 8 | minor) version or above. */
				/* If non-zero another array entry follows */
	int (*power)(int on);	/* Only valid in first array entry */
	int	(*init_irq)(void);
	int (*get_int_status)(void);
	void (*reset_ic)(void);
};

#endif // __SITRONIX_I2C_ST1232_h
