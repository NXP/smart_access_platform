/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __APP_MOTOR_H__
#define __APP_MOTOR_H__

#include "stdint.h"
#include "fsl_iocon.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
// MOTOR
#define MOTORA_PORT                   1u
#define MOTORA_PIN                    7u
#define MOTORA_FUNC                   IOCON_FUNC0
#define MOTORA_PINCFG                 IOCON_MODE_INACT | IOCON_DIGITAL_EN

#define MOTORB_PORT                   0u
#define MOTORB_PIN                    1u
#define MOTORB_FUNC                   IOCON_FUNC0
#define MOTORB_PINCFG                 IOCON_MODE_INACT | IOCON_DIGITAL_EN

#define MOTOSLEEP_PORT                1u
#define MOTOSLEEP_PIN                 4u
#define MOTOSLEEP_FUNC                IOCON_FUNC0
#define MOTOSLEEP_PINCFG              IOCON_MODE_INACT | IOCON_DIGITAL_EN

extern void APP_MOTOR_Init(void);
extern void APP_MOTOR_Set(uint8_t status);
extern void APP_MOTOR_Task(uint8_t state);

#endif /* __APP_MOTOR_H__ */
