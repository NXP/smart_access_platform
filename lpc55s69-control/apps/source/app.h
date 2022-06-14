/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __APP_H__
#define __APP_H__

// C Standard Lib includes

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "fsl_common.h"
#include "fsl_clock.h"
#include "fsl_gpio.h"
#include "fsl_spi.h"
#include "fsl_i2c.h"
#include "fsl_usart.h"
#include "fsl_i2s.h"
#include "fsl_i2s_dma.h"
#include "fsl_mrt.h"
#include "fsl_inputmux.h"
#include "fsl_pint.h"
#include "fsl_iocon.h"
#include "fsl_gint.h"
#include "fsl_rtc.h"
#include "fsl_sysctl.h"
#include "fsl_dma.h"
#include "fsl_iap.h"
#include "fsl_iap_ffr.h"
#include "fsl_puf.h"
#include "fsl_prince.h"
#include "fsl_rng.h"

#include "fsl_common.h"
#include "fsl_iocon.h"
#include "fsl_power.h"
#include "fsl_rtc.h"

#include "clock_config.h"

// --- PRINTF ---
#include "lpc_ring_buffer.h"
#include "app_printf.h"

// --- RTC Calendar --- 
#include "app_calendar.h"

// --- LED display ---
#include "app_led.h"

// --- System Information Configure ---
#include "app_syscfg.h"

// --- CLRC630 NFC Reader ---
#include "app_nfcreader.h"
#include "drv_nfcreader663.h"

// --- finger print ---
#include "lpc5500_fpslib.h"
#include "btl_bep.h"
#include "app_fingerprint.h"

// --- K32W BLE ---
#include "app_ble.h"
#include "app_matter.h"

// --- FACEID ---
#include "app_faceid.h"

// --- PIR Sensor ---
#include "app_pir.h"

// --- interrupt service ---
#include "app_interrupt.h"

// --- Motor control API ---
#include "app_motor.h"

// --- Capacitive touch ---
#include "app_captouch.h"

// --- Flash SPI ---
#include "app_spiflash.h"

// --- MP3 ---
#include "app_mp3play.h"

// --- binary upload to flash ---

// --- Low Power API ---
//#include "app_lowpower.h"


extern volatile uint32_t g_SysTicks;

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BOARD_NAME                 "Smart Access"
#define APPCORE0_VERSION_MAIN      12u
#define APPCORE0_VERSION_MINI      9u

#define PFR_STORAGEADDR            0x0009C000   /*  16KB For PFR Address */

#define BOOT_IMG_ADDR              0x00000000   /*  32KB For Bootloader Image Address */
#define BOOT_IMG_SIZE              0x00008000

#define CORE0_IMG_ADDR             0x00008000   /* 288KB For Core0 Secure Image Address */
#define CORE0_IMG_SIZE             0x00048000

#define FPTM_FPTADDR               0x00060000   /*  96KB For FingerTemplate Storage */
#define FPTM_FPTSIZE               0x00032000

#define USER_CFGADDR               0x00098000   /*  16KB For User Application Config Information */
#define USER_CFGSIZE               0x00004000

#define OSINTERVAL                 1000
#define SLEEPTIME                  20

#define DEBUG_USART_ENABLE         1

#endif /* __APP_H__ */
