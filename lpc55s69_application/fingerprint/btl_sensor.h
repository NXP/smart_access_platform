/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __BTL_SENSOR_H__
#define __BTL_SENSOR_H__

#include "stdbool.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdint.h"

#include "fsl_common.h"
#include "fsl_clock.h"
#include "fsl_gpio.h"
#include "fsl_spi.h"
#include "fsl_mrt.h"
#include "fsl_inputmux.h"
#include "fsl_pint.h"
#include "fsl_iocon.h"
#include "fsl_iap.h"
#include "fsl_iap_ffr.h"

#include "btl_bep.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define COMMAND

//-------------------Factory-----------------------
#define  READ_VERSION_CMD       0X01
#define  POWER_CONTRAL_CMD      0X03
#define  BL_WRITE_REG_CMD       0X04
#define  BL_READ_REG_CMD        0X05
#define  READ_CURRENT_CMD       0X06
#define  READ_BMP_CMD           0X07
#define  SET_RESET_PIN_CMD      0X08
#define  SET_BL_SPI_SPEED_CMD   0X09
#define  BEEP_CONTRAL_CMD       0x0a
#define  SET_INTERRUPT_CMD      0X0b
#define  START_IAP_CMD          0X0c
#define  CHECK_IAP_CMD          0X0d

#define  SET_IIC_SPEED_CMD      0X10
#define  SET_IIC_INT_FUN_CMD    0X11
#define  SET_IIC_PIN_LEVEL_CMD  0X12
#define  IIC_READ_CMD           0X13
#define  IIC_WRITE_CMD          0X14
#define  IIC_INT_REPORT_CMD     0X15
#define  IIC_INT_FLAG_CMD       0X16
#define  FIRMWARE_UPDATE_CMD    0x17
#define  FIRMWARE_HEX_CMD       0x18
#define  FIRMWARE_END_CMD       0x19
#define  VOLTAGE_CHECK_CMD      0x1A
#define  SWITCH_ENABLE_CMD      0X1B
#define  SWITCH_REPORT_CMD      0x1c
#define  TEST_MODE              0x2a


typedef struct BTL_PROCESS_PRE{  
	int NumFingers;
	int	AcceptedNum;
	int MatchIndex;
	btl_mode_t mode;
	unsigned char *pimage;	
	PBL_TEMPLATE pMulTemplates;
	PBL_TEMPLATE *MulTemplatesArray;	
} WORK_STRUCTURE;

typedef struct SENSOR_OPT
{
	unsigned char EnrollTimes;
	unsigned char MaxEnrollTimes;
	uint16_t StoreFingers;   	     // �洢��ָ����
	uint32_t FingerFlashSize;        // ָ��ģ��洢FLASH�ռ�(�ֽ�)  
    uint32_t StoreAddress;
    uint32_t StoreSize;
} FP_OPT;

extern FP_OPT g_OptGlobal;
extern WORK_STRUCTURE work;

#endif
