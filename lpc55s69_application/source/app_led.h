/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __APP_LED_H__
#define __APP_LED_H__

#include "stdint.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LEDFUNCDEBUG               0x01

#define LEDOSINTERVAL              1000UL

/* LED KeyPad & Ring control pin */
#define LED_DEBUG                  0u

#define LED_SPI                    SPI7                     // 74LS595 LED I2C Handler
#define LED_SPI_RST                kFC7_RST_SHIFT_RSTn
#define LED_SPI_CLKATTACH          kFRO12M_to_FLEXCOMM7
#define LED_SPI_CLKFREQ            12000000UL

/* 74LS595 I2C and SDB Pins */
#define LEDMO_PORT                 0u
#define LEDMO_PIN                  27u
#define LEDMO_FUNC                 IOCON_FUNC7
#define LEDMO_PINCFG               IOCON_MODE_PULLUP   | IOCON_DIGITAL_EN
 
#define LEDSCK_PORT                0u
#define LEDSCK_PIN                 21u
#define LEDSCK_FUNC                IOCON_FUNC7
#define LEDSCK_PINCFG              IOCON_MODE_PULLUP   | IOCON_DIGITAL_EN

#define LEDPD_PORT                 0u
#define LEDPD_PIN                  17u
#define LEDPD_FUNC                 IOCON_FUNC0
#define LEDPD_PINCFG               IOCON_MODE_PULLUP   | IOCON_DIGITAL_EN

#define LEDSTC_PORT                0u
#define LEDSTC_PIN                 22u
#define LEDSTC_FUNC                IOCON_FUNC0
#define LEDSTC_PINCFG              IOCON_MODE_PULLUP   | IOCON_DIGITAL_EN

#define LEDOE_PORT                 1u
#define LEDOE_PIN                  6u
#define LEDOE_FUNC                 IOCON_FUNC0
#define LEDOE_PINCFG               IOCON_MODE_PULLUP   | IOCON_DIGITAL_EN

#define LED_CH_ALL               0x3FFFF
#define LED_CH_01                0x00001
#define LED_CH_02                0x00002
#define LED_CH_03                0x00004
#define LED_CH_04                0x00008
#define LED_CH_05                0x00010
#define LED_CH_06                0x00020
#define LED_CH_07                0x00040
#define LED_CH_08                0x00080
#define LED_CH_09                0x00100
#define LED_CH_10                0x00200
#define LED_CH_11                0x00400
#define LED_CH_12                0x00800
#define LED_CH_13                0x01000
#define LED_CH_14                0x02000
#define LED_CH_15                0x04000
#define LED_CH_16                0x08000
#define LED_CH_17                0x10000
#define LED_CH_18                0x20000

extern volatile uint32_t g_LEDTicks;
extern volatile uint32_t g_LEDStatus;

extern void APP_LED_Init(void);
extern void APP_LED_Set(uint8_t num, uint8_t status);
extern void APP_LED_Task(void);

extern void APP_LED_AllOn(uint8_t delay);
extern void APP_LED_AllOff(void);
extern void APP_LED_AllSet(uint8_t status);

extern void APP_LED_TestTask();
extern void APP_LED_Disable(void);

#endif /* __APP_LED_H__ */
