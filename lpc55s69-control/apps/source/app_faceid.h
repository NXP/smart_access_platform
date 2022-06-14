/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __APP_FACEID_H__
#define __APP_FACEID_H__

#include "stdint.h"

/*******************************************************************************
 * Instructions
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/


/* FACEID RT117F Pins */
#if 1
#define FACEID_UART                   USART5
#define FACEID_UART_RST               kFC5_RST_SHIFT_RSTn
#define FACEID_UART_CLKATTACH         kMAIN_CLK_to_FLEXCOMM5 // kFRO12M_to_FLEXCOMM5 // kMAIN_CLK_to_FLEXCOMM5 // kFRO12M_to_FLEXCOMM5 // kMAIN_CLK_to_FLEXCOMM5
#define FACEID_UART_CLKFREQ           96000000 // CLOCK_GetFlexCommClkFreq(5U)
#define FACEID_UART_CLKSRC            kCLOCK_Flexcomm5
#define FACEID_UART_IRQNUM            FLEXCOMM5_IRQn
#define FACEID_UART_IRQHANDLER        FLEXCOMM5_IRQHandler

#define FACEIDTXD_PORT                0u
#define FACEIDTXD_PIN                 9u
#define FACEIDTXD_FUNC                IOCON_FUNC3

#define FACEIDRXD_PORT                0u
#define FACEIDRXD_PIN                 8u
#define FACEIDRXD_FUNC                IOCON_FUNC3

#define FACEIDWAKE_PORT               1u
#define FACEIDWAKE_PIN                29u
#define FACEIDWAKE_FUNC               IOCON_FUNC0


#if 0
#define FACEIDIRQI_PORT               1u
#define FACEIDIRQI_PIN                30u
#define FACEIDIRQI_FUNC               IOCON_FUNC0
#define FACEIDPINTI_SRC               kINPUTMUX_GpioPort1Pin30ToPintsel

#define FACEIDIRQO_PORT               0u
#define FACEIDIRQO_PIN                5u
#define FACEIDIRQO_FUNC               IOCON_FUNC0
#endif

#endif

#if 0
#define FACEID_UART                   USART0
#define FACEID_UART_RST               kFC0_RST_SHIFT_RSTn
#define FACEID_UART_CLKATTACH         kFRO12M_to_FLEXCOMM0
#define FACEID_UART_CLKFREQ           12000000UL
#define FACEID_UART_CLKSRC            kCLOCK_Flexcomm0
#define FACEID_UART_IRQNUM            FLEXCOMM0_IRQn
#define FACEID_UART_IRQHANDLER        FLEXCOMM0_IRQHandler

#define FACEIDTXD_PORT                0u
#define FACEIDTXD_PIN                 30u
#define FACEIDTXD_FUNC                IOCON_FUNC1

#define FACEIDRXD_PORT                0u
#define FACEIDRXD_PIN                 29u
#define FACEIDRXD_FUNC                IOCON_FUNC1

#define FACEIDIRQI_PORT               1u
#define FACEIDIRQI_PIN                30u
#define FACEIDIRQI_FUNC               IOCON_FUNC0
#define FACEIDPINTI_SRC               kINPUTMUX_GpioPort1Pin30ToPintsel

#define FACEIDIRQO_PORT               0u
#define FACEIDIRQO_PIN                5u
#define FACEIDIRQO_FUNC               IOCON_FUNC0
#endif

#define FACEID_BUFFER_SIZE       4096                                               /* FACEID Uart Buffer Length */

#define FACEIDTASKIDLE           0x00                                               /* FACEID Task is IDEL */
#define FACEIDTASKCHECKING       0x01

#define FACEIDUNLOCK             0x00000001                                         /* FACEID Command is Unlock the door */
#define FACEIDLOCK               0x00000002                                         /* FACEID Command is Lock the door */
#define FACEIDSTATUSOK           0x00000003
#define FACEIDSTATUSFAILED       0x00000004
#define FACEIDVALIDE             0x00000005                                         /* FACEID valid */
#define FACEIDUNVALIDE           0x00000006                                         /* FACEID un-valid */
#define FACEIDIDLE               0x00000000

#define FACEIDSTATUSIDLE         0xFFFFFFFF
 
extern volatile uint8_t  g_FACEIDTaskStatus;
extern volatile uint8_t  g_FACEIDDeviceStauts; // 0 - sleep, 1 - active
 
extern void     APP_FACEID_Init(void);
extern uint8_t  APP_FACEID_Task(void);

extern void     APP_FACEID_WAKEUP(void);

extern uint32_t APP_FACEID_Printf(const char *formatString, ...);

extern void     FACEID_UARTStringSend(char *str, uint32_t len);

#endif /* __APP_FACEID_H__ */
