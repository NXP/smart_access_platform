/*
 * Copyright (c) 2017 - 2020 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __APP_PRINTF_H__
#define __APP_PRINTF_H__

#include "stdint.h"
#include "fsl_common.h"
#include "fsl_iocon.h"
#include "fsl_usart.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define RINGBUF_ENABLE             0   
   
#define DEBUG_UART                 USART0
#define DEBUG_UART_TYPE            kSerialPort_Uart

#define DEBUG_UART_CLK_FREQ        CLOCK_GetFlexCommClkFreq(0U)
#define DEBUG_UART_CLK_ATTACH      kFRO12M_to_FLEXCOMM0
#define DEBUG_UART_RST             kFC0_RST_SHIFT_RSTn
#define DEBUG_UART_CLKSRC          kFRO12M_to_FLEXCOMM0
#define DEBUG_UART_IRQ_HANDLER     FLEXCOMM0_IRQHandler
#define DEBUG_UART_IRQNUM          FLEXCOMM0_IRQn
#define DEBUG_UART_BAUDRATE        115200
#define DEBUG_UART_BUFFERSIZE      2048

#define DEBUGTX_PORT               0u
#define DEBUGTX_PIN                30u
#define DEBUGTX_FUNC               IOCON_FUNC1

#define DEBUGRX_PORT               0u
#define DEBUGRX_PIN                29u
#define DEBUGRX_FUNC               IOCON_FUNC1

extern void     PRINT_UARTInit(uint32_t baudrate);
extern uint32_t PRINT_UARTPrintf(const char *formatString, ...);

extern void     PRINT_UARTPutc(uint8_t c);
extern void     PRINT_UARTPuts(char *str);
extern uint8_t  PRINT_UARTGetc(void);
extern void     PRINT_UARTStringSend(char *str, uint32_t len);

#define PRINTF  PRINT_UARTPrintf

#endif

// end file
