/*
 * Copyright (c) 2017 - 2020 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_iocon.h"
#include "fsl_usart.h"

#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include "stdio.h"

#include "lpc_ring_buffer.h"
#include "app_printf.h"
#include "app_binupdate.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint8_t  s_debugSendBuf[DEBUG_UART_BUFFERSIZE];
static uint8_t  s_debugRecvBuf[DEBUG_UART_BUFFERSIZE];

#if RINGBUF_ENABLE
ring_buffer_t   g_debugRingBuffer;
#else
static uint32_t s_debugRecbBufCnt = 0;
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/
/**
 * @brief   Flexcomm0 Interrupt Handler
 * @param   NULL
 * @return  NULL
 */
void DEBUG_UART_IRQ_HANDLER(void)
{
    uint8_t data;
    
#if 0
    /* If new data arrived. */
    if ((kUSART_RxFifoNotEmptyFlag | kUSART_RxError) & USART_GetStatusFlags(DEBUG_UART))
    {
        data = USART_ReadByte(DEBUG_UART);
#if RINGBUF_ENABLE
        RingBuf_Write1Byte(&g_debugRingBuffer, (const uint8_t *)&data);
#else
        /* If ring buffer is not full, add data to ring buffer. */
        s_debugRecvBuf[s_debugRecbBufCnt] = data;
        s_debugRecbBufCnt++;
        if(s_debugRecbBufCnt >= DEBUG_UART_BUFFERSIZE) s_debugRecbBufCnt = 0x00;
#endif
    }
#else
    /* If new data arrived. */
    if ((kUSART_RxFifoNotEmptyFlag | kUSART_RxError) & USART_GetStatusFlags(DEBUG_UART))
    {
        data = USART_ReadByte(DEBUG_UART);
        /* If ring buffer is not full, add data to ring buffer. */
        if(g_DebugCmdStatus == 0)
        {
            if (g_DebugrxIndex < DEBUG_BUFFER_SIZE)
            {
                g_DebugRecvBuf[g_DebugrxIndex] = data;
                g_DebugrxIndex++;
            }
            else
            {
                g_DebugrxIndex = 0;
            }
            
            /* 检测到结束标识符 */
            if( ((g_DebugRecvBuf[g_DebugrxIndex-2] == '\r') && (g_DebugRecvBuf[g_DebugrxIndex-1] == '\n')) ||
                ((g_DebugRecvBuf[g_DebugrxIndex-2] == '\n') && (g_DebugRecvBuf[g_DebugrxIndex-1] == '\r'))
              )
            {
                g_DebugCmdStatus = 1;
                g_UpdateTickCnt = 0x700000;
            }
        }
        if(g_DebugCmdStatus == 2)
        {
            g_BinaryImage[g_BinaryCnt] = data;
            g_BinaryCnt++;
            g_UpdateTickCnt = 0x700000;
        }
    }
#endif
}

void PRINT_UARTInit(uint32_t baudrate)
{
    usart_config_t      config;
    
    /* Enables the clock for the I/O controller.: Enable Clock. */
    CLOCK_EnableClock(kCLOCK_Iocon);
    /* Debug Pin Port Init */
    IOCON->PIO[DEBUGTX_PORT][DEBUGTX_PIN]   = (DEBUGTX_FUNC  | IOCON_MODE_INACT | IOCON_DIGITAL_EN );
    IOCON->PIO[DEBUGRX_PORT][DEBUGRX_PIN]   = (DEBUGRX_FUNC  | IOCON_MODE_INACT | IOCON_DIGITAL_EN );
    /* Disables the clock for the I/O controller.: Disable Clock. To Save Power */
    CLOCK_DisableClock(kCLOCK_Iocon);

    /*
     * config.baudRate_Bps = 115200U;
     * config.parityMode = kUSART_ParityDisabled;
     * config.stopBitCount = kUSART_OneStopBit;
     * config.loopback = false;
     * config.enableTx = false;
     * config.enableRx = false;
     */
    CLOCK_AttachClk(DEBUG_UART_CLKSRC);
    RESET_ClearPeripheralReset(DEBUG_UART_RST);
    
    USART_GetDefaultConfig(&config);
    config.baudRate_Bps = baudrate;
    config.enableTx = true;
    config.enableRx = true;
#if RINGBUF_ENABLE
    RingBuf_Init(&g_debugRingBuffer, (uint8_t *)s_debugRecvBuf, DEBUG_UART_BUFFERSIZE); /* Initialize data ring buffer */
#else
    s_debugRecbBufCnt = 0;
#endif
    USART_Init(DEBUG_UART, &config, DEBUG_UART_CLK_FREQ);
    /* Enable RX interrupt. */
    USART_EnableInterrupts(DEBUG_UART, kUSART_RxLevelInterruptEnable | kUSART_RxErrorInterruptEnable);
    NVIC_EnableIRQ(DEBUG_UART_IRQNUM);
}

/**
 * @brief   debug printf function
 * @param   string
 * @return  length
 */
/* See fsl_debug_console.h for documentation of this function. */
uint32_t PRINT_UARTPrintf(const char *formatString, ...)
{
    va_list arg;
    uint32_t logLength = 0U, result = 0U;
    memset(s_debugSendBuf, 0x00, DEBUG_UART_BUFFERSIZE);
    va_start(arg, formatString);
    logLength = vsprintf((char *)s_debugSendBuf, formatString, arg);            /* format print log first */
    va_end(arg);
    USART_WriteBlocking(DEBUG_UART, s_debugSendBuf, logLength);                /* Send Log data to BLE uart */                                                   /* free buffer */
    return result;
}

/**
 * @brief   DbgConsole_Printf
 * @param   formatString
 * @return  result
 */
uint32_t DbgConsole_Printf(const char *formatString, ...)
{
    va_list arg;
    uint32_t logLength = 0U, result = 0U;
    memset(s_debugSendBuf, 0x00, DEBUG_UART_BUFFERSIZE);
    va_start(arg, formatString);
    logLength = vsprintf((char *)s_debugSendBuf, formatString, arg);            /* format print log first */
    va_end(arg);
    USART_WriteBlocking(DEBUG_UART, s_debugSendBuf, logLength);                /* Send Log data to BLE uart */                                                   /* free buffer */
    return result;
}

/**
 * @brief   PRINT_UARTPutc
 * @param   c - char
 * @return  NULL
 */
void PRINT_UARTPutc(uint8_t c)
{
    USART_WriteBlocking(DEBUG_UART, &c, 1);
}

/**
 * @brief   PRINT_UARTGetc
 * @param   NULL
 * @return  NULL
 */
uint8_t PRINT_UARTGetc(void)
{
    uint8_t c = 0;
//#if 1
//        int bytes = RingBuf_Read1Byte(&g_debugRingBuffer, &c);
//        if (bytes > 0) {
//            return c;
//        }
//        else
//        {
//            return 0x00;
//        }
//#else
//    while (1) {
//        int bytes = RingBuf_Read1Byte(&g_debugRingBuffer, &c);
//        if (bytes > 0) {
//            return c;
//        }
//    }
//#endif
    return 1;
}

/**
 * @brief   PRINT_UARTPuts
 * @param   str -- string
 * @return  NULL
 */
void PRINT_UARTPuts(char *str)
{
    while (*str) {
        PRINT_UARTPutc(*str++);
    }
}

/**
 * @brief   PRINT_UARTStringSend
 * @param   str -- string, len -- string length
 * @return  NULL
 */
void PRINT_UARTStringSend(char *str, uint32_t len)
{
    uint32_t i;
    for(i=0; i<len; i++)
    {
        PRINT_UARTPutc(*str++);
    }
}

// end file
