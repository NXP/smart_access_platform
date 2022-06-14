/*
 * Copyright (c) 2017 - 2021 , NXP
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
/* Debug USART port */
#if 0    /* Select USART0 as debug port */
#define DEBUG_UART                 USART0
#define DEBUG_UART_TYPE            kSerialPort_Uart

#define DEBUG_UART_CLK_FREQ        CLOCK_GetFlexCommClkFreq(0U)
#define DEBUG_UART_CLK_ATTACH      kFRO12M_to_FLEXCOMM0
#define DEBUG_UART_RST             kFC0_RST_SHIFT_RSTn
#define DEBUG_UART_CLKSRC          kFRO12M_to_FLEXCOMM0
#define DEBUG_UART_IRQ_HANDLER     FLEXCOMM0_IRQHandler
#define DEBUG_UART_IRQNUM          FLEXCOMM0_IRQn
#define DEBUG_UART_BAUDRATE        115200
#define DEBUG_UART_BUFFERSIZE      64

#define DEBUGTX_PORT               0u
#define DEBUGTX_PIN                30u
#define DEBUGTX_FUNC               IOCON_FUNC1

#define DEBUGRX_PORT               0u
#define DEBUGRX_PIN                29u
#define DEBUGRX_FUNC               IOCON_FUNC1
#endif

#if 1    /* Select USART2 as debug port */
#define DEBUG_UART                 USART2
#define DEBUG_UART_TYPE            kSerialPort_Uart

#define DEBUG_UART_CLK_FREQ        CLOCK_GetFlexCommClkFreq(2U)
#define DEBUG_UART_CLK_ATTACH      kFRO12M_to_FLEXCOMM2
#define DEBUG_UART_RST             kFC2_RST_SHIFT_RSTn
#define DEBUG_UART_CLKSRC          kFRO12M_to_FLEXCOMM2
#define DEBUG_UART_IRQ_HANDLER     FLEXCOMM2_IRQHandler
#define DEBUG_UART_IRQNUM          FLEXCOMM2_IRQn
#define DEBUG_UART_BAUDRATE        115200
#define DEBUG_UART_BUFFERSIZE      512

#define DEBUGTX_PORT               1u
#define DEBUGTX_PIN                25u
#define DEBUGTX_FUNC               IOCON_FUNC1

#define DEBUGRX_PORT               1u
#define DEBUGRX_PIN                24u
#define DEBUGRX_FUNC               IOCON_FUNC1
#endif

extern void     PRINT_UARTInit(uint32_t baudrate);
extern uint32_t PRINT_UARTPrintf(const char *formatString, ...);

extern void     PRINT_UARTPutc(uint8_t c);
extern void     PRINT_UARTPuts(char *str);
extern uint8_t  PRINT_UARTGetc(void);
extern void     PRINT_UARTStringSend(char *str, uint32_t len);

extern char *strupr(char *str);

#define PRINTF  PRINT_UARTPrintf

extern void PRINT_TickHandler(void);

extern bool Terminal_GetCharTimeout(uint8_t *rxDat, uint32_t ms);
extern void Terminal_PutChar(uint8_t ch);

extern void APP_PRINTF_Task(void);

#endif /* __APP_PRINTF_H__ */
