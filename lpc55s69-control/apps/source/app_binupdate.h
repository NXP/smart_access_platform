/*
 * Copyright (c) 2017 - 2018 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __APP_BINUPDATE_H__
#define __APP_BINUPDATE_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/
 
#define DEBUG_BUFFER_SIZE       200   /* BLE Uart Buffer Length */

#define RECV_FILE_NUMS          34    /* Defined receive hom many music files from host */

extern volatile uint8_t  g_DebugRecvBuf[];
extern volatile uint16_t g_DebugrxIndex;                   /* Index of the memory to save new arrived data. */
extern volatile uint8_t *g_DebugCmdCmp;     
extern volatile uint8_t  g_DebugCmdStatus;
extern volatile uint32_t g_UpdateTickCnt;
extern volatile uint32_t g_BinaryCnt;
extern volatile uint8_t *g_BinaryImage;

extern void    binupdate_init(void);
extern uint8_t binupdate_task(void);

#endif /* __APP_BINUPDATE_H__ */
