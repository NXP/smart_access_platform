/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __APP_CAPTOUCH_H__
#define __APP_CAPTOUCH_H__

#include "stdint.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
 
/* ----------------------------------------------------------------------------------------------------- */
/* --------------------------------------- USER SETTINGS ----------------------------------------------- */
/* ----------------------------------------------------------------------------------------------------- */
#define TOUCHIC_IS_KL16Z   0x1u                 /* Touch IC is KL16Z */

#define TOUCHIC_I2C_SHARED    1u                 /* I2C interface initialize by another chip */

#define CAPTDEBOUNCETIME     80u                 /* 80mS for debounce */
 
#define CAPTFUNCDEBUG       0x00                 /* Debug CAPT basic driver, eg. i2c read and write */
#define CAPTTASKDEBUG       0x01                 /* Debug CAPT task function */

#ifndef CAPTSTORAGENUMS
#define CAPTSTORAGENUMS     0x6                  /* Storage how many keys */
#endif

#ifndef CAPTKEYLENGTH
#define CAPTKEYLENGTH       30u                  /* key length */
#define CAPTKEYVALIDLENGTH   6u                  /* key valid length */
#endif

#define CAPTLEDENABLE       0x01                 /* Touched with LED */
#define PINPADPWDCOMEN      0x01                 /* Touch Pin Pad password compare function enable */

#define SENSITIVE           0xBB                 /* Touch Pad Sensitive */

/* ----------------------------------------------------------------------------------------------------- */
/* --------------------------------- Hardware Related Settings ----------------------------------------- */
/* ----------------------------------------------------------------------------------------------------- */
/* Captouch I2C interface */
#define CAPTOUCH_I2C               I2C1
#define CAPTOUCH_I2C_RST           kFC1_RST_SHIFT_RSTn
#define CAPTOUCH_I2C_CLKATTACH     kFRO12M_to_FLEXCOMM1
#define CAPTOUCH_I2C_CLKFREQ       12000000UL
#define CAPTOUCH_I2C_CLKSRC        kCLOCK_Flexcomm1
#define CAPTOUCH_I2C_RATE          100000UL

#define CAPTIRQ_PORT               1u
#define CAPTIRQ_PIN                27u
#define CAPTPINT_SRC               kINPUTMUX_GpioPort1Pin27ToPintsel
#define CAPTIRQ_FUNC               IOCON_FUNC0
#define CAPTIRQ_PINCFG             IOCON_MODE_PULLUP | IOCON_DIGITAL_EN
#define CAPTPINT_CFG               (1<<CAPTIRQ_PIN)

#define CAPTRST_PORT               1u
#define CAPTRST_PIN                28u
#define CAPTRST_FUNC               IOCON_FUNC0
#define CAPTRST_PINCFG             IOCON_MODE_INACT   | IOCON_DIGITAL_EN

#define CAPTSDA_PORT               0u
#define CAPTSDA_PIN                13u
#define CAPTSDA_FUNC               IOCON_FUNC1
#define CAPTSDA_PINCFG             IOCON_MODE_INACT   | IOCON_DIGITAL_EN

#define CAPTSCL_PORT               0u
#define CAPTSCL_PIN                14u
#define CAPTSCL_FUNC               IOCON_FUNC1
#define CAPTSCL_PINCFG             IOCON_MODE_INACT   | IOCON_DIGITAL_EN

#define ALARM_PORT                 0u
#define ALARM_PIN                  5u
#define ALARM_FUNC                 IOCON_FUNC0
#define ALARM_PINCFG               IOCON_MODE_INACT   | IOCON_DIGITAL_EN

/* ----------------------------------------------------------------------------------------------------- */
/* --------------------------------------- CODE DEFINED ------------------------------------------------ */
/* ----------------------------------------------------------------------------------------------------- */
#define KEYWAKEUP           0x0000               /* Wakeup */

#define KEY1_VALUE          0x0001               /* 1 */
#define KEY2_VALUE          0x0002               /* 2 */
#define KEY3_VALUE          0x0004               /* 3 */
#define KEY4_VALUE          0x0008               /* 4 */
#define KEY5_VALUE          0x0010               /* 5 */
#define KEY6_VALUE          0x0020               /* 6 */
#define KEY7_VALUE          0x0040               /* 7 */
#define KEY8_VALUE          0x0080               /* 8 */
#define KEY9_VALUE          0x0100               /* 9 */
#define KEYA_VALUE          0x0200               /* * */
#define KEY0_VALUE          0x0400               /* 0 */ 
#define KEYB_VALUE          0x0800               /* # */

#define KEYIDLE             0xFFFF               /* IDLE */

#define CAPTVALIDPWD        0x01
#define CAPTWRONGPWD        0x02
#define CAPTSETEDPWD        0xF1
#define CAPTTASKIDLE        0x00

// TODO: Magicoe will fix this address define late
#define KL16ZI2CGNDADDR    0x10 // 0x68                 /* 7bit address: 8bit address 0xD0<<1 Chip_ID Pin = GND */
#define KL16ZI2CVDDADDR    0x78                 /* 7bit address: 8bit address 0xF0<<1 Chip_ID Pin = VDD */

#define SENSITIVITY1        0x02                 /* ch2,ch1 */
#define SENSITIVITY2        0x03                 /* ch4,ch3 */
#define SENSITIVITY3        0x04                 /* ch6,ch5 */
#define SENSITIVITY4        0x05                 /* ch8,ch7 */
#define SENSITIVITY5        0x06                 /* ch10,ch9 */
#define SENSITIVITY6        0x07                 /* ch12,ch11 */
#define SENSITIVITY7        0x22                 /* ch14,ch13 */
#define SENSITIVITY8        0x23                 /* ch16,ch15 */

#define KL16ZCTRL1         0x08
#define KL16ZCTRL2         0x09

#define KL16Z_REFRST1      0x0A
#define KL16Z_REFRST2      0x0B
#define KL16Z_CHHOLD1      0x0C                 /* Touch Key Channel Enable = 0x00 */
#define KL16Z_CHHOLD2      0x0D                 /* Touch Key Channel Enable = 0x00 */
#define KL16Z_CALHOLD1     0x0E                 /* Calibration Enable = 0x00 */
#define KL16Z_CALHOLD2     0x0F                 /* Calibration Enable = 0x00 */

#define KL16Z_OUTPUT1      0x10                 /* cs4~cs1 output */
#define KL16Z_OUTPUT2      0x11                 /* cs8~cs5 output */
#define KL16Z_OUTPUT3      0x12                 /* cs12~cs9 output */
#define KL16Z_OUTPUT4      0x13                 /* cs16~cs13 output */
#define KL16Z_LOCKMASK     0x3B                 /* Lock Mask */
#define KL16Z_FORCEEN      0x41                 /* Force Enable */

/** CAPT - DataStorage */
typedef struct {
    uint8_t  status;                             /* input password length*/
    uint8_t  key[CAPTKEYLENGTH];                 /* input password */
} CAPTData_Type;


extern volatile uint8_t  g_CAPTaskStatus;
extern volatile uint8_t  g_captTaskEnable;

extern          void     APP_CAPT_Init(void);
extern          uint8_t  APP_CAPT_Task(void);
extern          void     CAPT_KL16ZClean(void);
extern          uint8_t  CAPT_KL16ZRead(void);
extern          void     CAPT_KL16ZIntHandler(void);
extern          void     CAPT_KL16ZTickHandler(void);
extern void APP_CAPT_ReInit(void);

extern void APP_CAPT_RESET_0(void);
extern void APP_CAPT_RESET_1(void);

#if PINPADPWDCOMEN
extern uint8_t PINPAD_PasswordSet(uint8_t num);
extern uint8_t PINPAD_PasswordCompare();
#endif

#endif /* __APP_CAPTOUCH_H__ */
