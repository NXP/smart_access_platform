/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __APP_INTERRUPT_H__
#define __APP_INTERRUPT_H__

#include "stdint.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
extern volatile uint32_t g_SysTicks;
extern volatile uint32_t g_AliveTicks;

extern volatile uint32_t g_WakeupFlag;

extern void APP_PinInterruptInit(void);
extern volatile uint32_t g_NFCReadCnt;

#endif /* __APP_INTERRUPT_H__ */
