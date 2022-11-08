/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __APP_PIR_H__
#define __APP_PIR_H__

#include "stdint.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define PIRSENSOR_PORT                 1u
#define PIRSENSOR_PIN                  18u
#define PIRSENSOR_FUNC                 IOCON_FUNC0
#define PIRSENSOR_PINCFG               IOCON_MODE_INACT | IOCON_DIGITAL_EN

#define PIR_HUMAN_CLEAN                0x00
#define PIR_HUMAN_DETECTED             0x01

extern void    APP_PIR_Init(void);
extern uint8_t APP_PIR_StatusRead(void);
extern void    APP_PIR_Task(void);

extern void    APP_PIR_IntHandler(void);
extern void    APP_PIR_TickHandler(void);

#endif /* __APP_PIR_H__ */
