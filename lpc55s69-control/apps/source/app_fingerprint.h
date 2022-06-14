/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __APP_FINGERPRINT_H__
#define __APP_FINGERPRINT_H__

#include "fsl_gint.h"

#include "fsl_common.h"

#include "app_printf.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FPS_SPI_SHARED                 1

#define FPS_PRINTF                     PRINT_UARTPrintf

#define FPS_FLASH_ADDR                 0x00060000   /* save fingepint template region*/
#define FPS_FLASH_SIZE                 0x00032000
#define FPS_SECTOR_NUM                 3u                  

#define FPS_DYNAMIC_UPDATE             0u

#define FPS_RST_PORT                   0u
#define FPS_RST_PIN                    25u
#define FPS_RST_FUNC                   (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN)

#define FPS_INT_PORT                   1u
#define FPS_INT_PIN                    17u
#define FPS_INT_FUNC                   (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN)

#if FPS_SPI_SHARED /* Shared with NFC in smart access hardware */

#define FPS_CS_PORT                    0u
#define FPS_CS_PIN                     7u
#define FPS_CS_FUNC                    (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN)

#define FPS_SPI                        SPI3

#else
#define FPS_MI_PORT                    0u
#define FPS_MI_PIN                     2u
#define FPS_MI_FUNC                    (IOCON_FUNC1 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN)

#define FPS_MO_PORT                    0u
#define FPS_MO_PIN                     3u
#define FPS_MO_FUNC                    (IOCON_FUNC1 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN)

#define FPS_CS_PORT                    0u
#define FPS_CS_PIN                     7u
#define FPS_CS_FUNC                    (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN)

#define FPS_CK_PORT                    0u
#define FPS_CK_PIN                     6u
#define FPS_CK_FUNC                    (IOCON_FUNC1 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN)

#define FPS_SPI                        SPI3
#define FPS_SPI_RST                    kFC3_RST_SHIFT_RSTn
#define FPS_SPI_CLKATTACH              kFRO12M_to_FLEXCOMM3
#define FPS_SPI_CLKFREQ                12000000UL
#define FPS_SPI_CLKSRC                 kCLOCK_Flexcomm3
#define FPS_SENSOR_CLOCKRATE           6000000UL

#endif


extern volatile uint16_t g_fingerEnrollIndex;
extern volatile uint8_t  g_FPSTaskStatus;

extern void    APP_FPS_Init(void);
extern uint8_t APP_FPS_Task(uint16_t* index);
extern void    APP_FPS_ReInit(void);

#endif /* __APP_FINGERPRINT_H__ */

