/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __APP_NFCREADER_H__
#define __APP_NFCREADER_H__

#include "stdint.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define NFCCARDVALID          0x01                           // Find Valid Card
#define NFCCARDILLEGAL        0x02                           // 
#define NFCCARDREGOK          0xF1
#define NFCCARDIDLE           0x00

extern volatile uint8_t  g_NFCCardEnrollMode;
extern volatile uint8_t  g_nfcTaskEnable;

extern void    APP_NFC_Init(void);
extern uint8_t APP_NFC_Task(void);
extern void    APP_NFC_TaskEnable(void);
extern void    APP_NFC_TaskDisable(void);
extern void    APP_NFC_Disable(void);

#endif /* __APP_NFCREADER_H__ */
