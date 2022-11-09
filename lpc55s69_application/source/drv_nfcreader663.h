/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __DRV_NFCREADER663_H__
#define __DRV_NFCREADER663_H__

#include "stdint.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief nfc reader status */
#define NFCREADER_DEBUG       0u
#define NFCFUNCDEBUG          1u

#define RC663_SPI_SHARED      0

#define RC663_USE_I2C         0u                             // CLRC630 Com Interface is I2C
#define RC663_USE_SPI         1u                             // CLRC630 Com Interface is SPI
#define RC663_SPI_SIM         0u                             // Use GPIO simulate SPI interface

#if RC663_USE_I2C
#define RC663I2CADDR          0x28U                          // CLRC630 I2C Address
#endif

/* RC630 SPI Pins */
#if RC663_SPI_SHARED

#define NFCREADER_SPI              SPI3                         // CLR663/630 SPI Handler

#else
#define NFCREADER_SPI              SPI3                         // CLR663/630 SPI Handler
#define NFCREADER_SPI_RST          kFC3_RST_SHIFT_RSTn
#define NFCREADER_SPI_CLKATTACH    kFRO12M_to_FLEXCOMM3
#define NFCREADER_SPI_CLKSOURCE    12000000UL
#define NFCREADER_SPI_CLKFREQ        600000UL

#define NFCMO_PORT                 0u
#define NFCMO_PIN                  3u
#define NFCMO_FUNC                 IOCON_FUNC1

#define NFCMI_PORT                 0u
#define NFCMI_PIN                  2u
#define NFCMI_FUNC                 IOCON_FUNC1

#define NFCSCK_PORT                0u
#define NFCSCK_PIN                 6u
#define NFCSCK_FUNC                IOCON_FUNC1
#endif

#define NFCSEL_PORT                0u
#define NFCSEL_PIN                 4u
#define NFCSEL_FUNC                IOCON_FUNC0

/* CLRC630 NFC Reader */
#define NFCRST_PORT                0u
#define NFCRST_PIN                 16u
#define NFCRST_FUNC                IOCON_FUNC0

#define NFCIRQ_PORT                1u
#define NFCIRQ_PIN                 0u
#define NFCIRQ_FUNC                IOCON_FUNC0
#define NFCPINT_SRC                kINPUTMUX_GpioPort1Pin0ToPintsel

extern volatile uint8_t  g_NFCTaskStatus;

extern void RC663_PinInit(void);

#endif /* __DRV_NFCREADER663_H__ */
