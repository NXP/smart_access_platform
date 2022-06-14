/*
 * Copyright (c) 2017 - 2018 , NXP
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

// LPC55xx Chip peripherals & Boards includes
#include "fsl_common.h"
#include "fsl_clock.h"
#include "fsl_gpio.h"
#include "fsl_usart.h"
#include "fsl_iocon.h"
#include "clock_config.h"

#include "lpc_ring_buffer.h"
#include "app_printf.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define BOARD_NAME                 "ELOCK LPC55S69 2.0"
#define BOOT_VERSION_MAIN          1
#define BOOT_VERSION_MINI          0


#define AUDIO_IMAGE_ADDR           0x04000000


/* Use Alarm Button to enter BOOT loader */
#define BOOT_ENTRY_PORT            0u
#define BOOT_ENTRY_PIN             20u
#define BOOT_ENTRY_FUNC            IOCON_FUNC0

#endif

// End File
