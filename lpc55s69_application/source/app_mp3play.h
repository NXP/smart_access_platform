/*
 * Copyright (c) 2017 - 2021, NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __APP_MP3PLAY_H__
#define __APP_MP3PLAY_H__

#include "stdint.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* CODEC CODEC */
/* Power Enable Pin */
/* Enable DC/DC */
#define CODEC_I2C_SHARED              0u                 /* I2C interface initialize by another chip */

/* CODEC I2C interface */
#define CODEC_I2C                     I2C1
#define CODEC_I2C_RST                 kFC1_RST_SHIFT_RSTn
#define CODEC_I2C_CLKATTACH           kFRO12M_to_FLEXCOMM1
#define CODEC_I2C_CLKFREQ             12000000UL
#define CODEC_I2C_CLKSRC              kCLOCK_Flexcomm1
#define CODEC_I2C_RATE                100000UL

#define CODECSDA_PORT                 0u
#define CODECSDA_PIN                  13u
#define CODECSDA_FUNC                 IOCON_FUNC1
#define CODECSDA_PINCFG               IOCON_MODE_INACT   | IOCON_DIGITAL_EN

#define CODECSCL_PORT                 0u
#define CODECSCL_PIN                  14u
#define CODECSCL_FUNC                 IOCON_FUNC1
#define CODECSCL_PINCFG               IOCON_MODE_INACT   | IOCON_DIGITAL_EN

#define CODEC_I2S                     I2S6
#define CODEC_I2S_RST                 kFC6_RST_SHIFT_RSTn
#define CODEC_I2S_CLKATTACH           kPLL0_DIV_to_FLEXCOMM6
#define CODEC_I2S_CLKFREQ             24576000UL
#define CODEC_I2S_CLKSRC              kCLOCK_Flexcomm6

#define MP3_AUDIO_BUF_SZ              0x1800 // (4 * 1024)
#define CODEC_I2STX                   I2S6
#define CODEC_I2STX_CHANNEL           (17)
#define I2S_CLOCK_DIVIDER             (CODEC_I2S_CLKFREQ / 32000U / 32U / 1U)

#define CODEC_PWR_PORT                0u
#define CODEC_PWR_PIN                 15u
#define CODEC_PWR_FUNC                (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN)

#define CODEC_MCLK_PORT               1u
#define CODEC_MCLK_PIN                31u
#define CODEC_MCLK_FUNC               (IOCON_FUNC1 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN)

#define CODEC_SCK_PORT                1u
#define CODEC_SCK_PIN                 12u
#define CODEC_SCK_FUNC                (IOCON_FUNC2 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN)

#define CODEC_DAT_PORT                1u
#define CODEC_DAT_PIN                 13u
#define CODEC_DAT_FUNC                (IOCON_FUNC2 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN)

#define CODEC_WS_PORT                 1u
#define CODEC_WS_PIN                  16u
#define CODEC_WS_FUNC                 (IOCON_FUNC2 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN)

extern void APP_MP3_Init(void);
extern void APP_MP3_Play(uint32_t index);

#endif /* __APP_MP3PLAY_H__ */
