/*
 * Copyright (c) 2017 - 2022 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include "stdio.h"

#include "fsl_common.h"
#include "fsl_iocon.h"
#include "fsl_usart.h"
#include "fsl_gpio.h"

#include "app_syscfg.h"
#include "app_faceid.h"
#include "app_ble.h"
#include "app_printf.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint8_t  g_FACEIDTaskStatus = 0;
volatile uint8_t  g_FACEIDCmdStatus  = 0;
volatile uint32_t g_FACEIDSystick    = 0;

volatile uint8_t  g_FACEIDRecvBuf[FACEID_BUFFER_SIZE];

volatile uint16_t g_FACEIDrxIndex = 0;                                              /* Index of the memory to save new arrived data. */
volatile uint8_t *g_FACEIDCmdCmp  = NULL;

volatile uint8_t  g_FACEIDPwdNum = 0;
volatile uint8_t  g_FACEIDPwdLen = 0;
volatile uint8_t  g_FACEIDTempbuf[32];

volatile uint8_t  g_FACEIDDeviceStauts = 0; // 0 - sleep, 1 - active
volatile uint8_t  g_FACEIDVIZNid = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/
/**
 * @brief   
 * @param   
 * @return  
 */
volatile uint32_t g_FACEIDmsCnt1, g_FACEIDmsCnt2;
void faceid_delayms(uint32_t times)
{
    for (g_FACEIDmsCnt1 = 0; g_FACEIDmsCnt1 < times; g_FACEIDmsCnt1++)
    {
        for (g_FACEIDmsCnt2 = 0; g_FACEIDmsCnt2 < (SystemCoreClock/9000); g_FACEIDmsCnt2++)
        {
            ;
        }
    }
}

/**
 * @brief   Flexcomm Interrupt Handler
 * @param   NULL
 * @return  NULL
 */
void FACEID_UART_IRQHANDLER(void)
{
    uint8_t data;

    /* If new data arrived. */
    if ((kUSART_RxFifoNotEmptyFlag | kUSART_RxError) & USART_GetStatusFlags(FACEID_UART))
    {
        data = USART_ReadByte(FACEID_UART);
        USART_WriteByte(DEBUG_UART, data);
        /* If ring buffer is not full, add data to ring buffer. */
        if (g_FACEIDrxIndex < FACEID_BUFFER_SIZE)
        {
            g_FACEIDRecvBuf[g_FACEIDrxIndex] = data;
            g_FACEIDrxIndex++;
        }
        else
        {
            g_FACEIDrxIndex = 0;
        }

        if( ((g_FACEIDRecvBuf[g_FACEIDrxIndex-2] == '\r') && (g_FACEIDRecvBuf[g_FACEIDrxIndex-1] == '\n')) ||
            ((g_FACEIDRecvBuf[g_FACEIDrxIndex-2] == '\n') && (g_FACEIDRecvBuf[g_FACEIDrxIndex-1] == '\r'))
          )
        {
            g_FACEIDCmdStatus = 1;
        }
#if 0
        /*  */
        if( (g_FACEIDRecvBuf[g_FACEIDrxIndex-1] == '!') )
        {
            g_FACEIDRecvBuf[g_FACEIDrxIndex-1] = 0x00;
            g_FACEIDCmdStatus = 1;
        }
#endif
    }
}

/**
 * @brief   APP_FACEID_Printf
 * @param   formatString， printf("%x", x);
 * @return  NULL
 */
/* See fsl_debug_console.h for documentation of this function. */
volatile uint8_t g_FDPrintBuf[4096];
uint32_t APP_FACEID_Printf(const char *formatString, ...)
{
    va_list arg;
    uint32_t logLength = 0U, result = 0U;
    uint8_t *PrintBuf = NULL;
    memset((void *)g_FDPrintBuf, 0x00, 4096);
    va_start(arg, formatString);
    /* format print log first */
    logLength = vsprintf((char *)g_FDPrintBuf, formatString, arg);
    va_end(arg);
    USART_WriteBlocking(FACEID_UART, (const uint8_t *)g_FDPrintBuf, logLength);

    return result;
}

/**
 * @brief   FACEID_UARTPutc
 * @param   c - char
 * @return  NULL
 */
void FACEID_UARTPutc(uint8_t c)
{
    USART_WriteBlocking(FACEID_UART, &c, 1);
}

/**
 * @brief   FACEID_UARTPuts
 * @param   str -- string
 * @return  NULL
 */
void FACEID_UARTPuts(char *str)
{
    while (*str) {
    	FACEID_UARTPutc(*str++);
    }
}

/**
 * @brief   FACEID_UARTStringSend
 * @param   str -- string, len -- string length
 * @return  NULL
 */
void FACEID_UARTStringSend(char *str, uint32_t len)
{
    uint32_t i;
    for(i=0; i<len; i++)
    {
    	FACEID_UARTPutc(*str++);
    }
}

/**
 * @brief   Initialize FACEID module
 * @param   rst : 0-not reset module, 1-reset module
 * @return  NULL
 */
void APP_FACEID_Init(void)
{
    usart_config_t      config;
    gpio_pin_config_t   gpioPinConfig;

    /* Enables the clock for the I/O controller.: Enable Clock. */
    CLOCK_EnableClock(kCLOCK_Iocon);
    /* Debug Pin Port Init */
	IOCON->PIO[FACEIDTXD_PORT][FACEIDTXD_PIN]   = (FACEIDTXD_FUNC  | IOCON_MODE_INACT  | IOCON_DIGITAL_EN );
	IOCON->PIO[FACEIDRXD_PORT][FACEIDRXD_PIN]   = (FACEIDRXD_FUNC  | IOCON_MODE_INACT  | IOCON_DIGITAL_EN );
	IOCON->PIO[FACEIDWAKE_PORT][FACEIDWAKE_PIN] = (FACEIDWAKE_FUNC | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN );

#if 0
    IOCON->PIO[FACEIDIRQI_PORT][FACEIDIRQI_PIN] = (FACEIDIRQI_FUNC  | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN );
    IOCON->PIO[FACEIDIRQO_PORT][FACEIDIRQO_PIN] = (FACEIDIRQO_FUNC  | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN );
#endif

    /* Disables the clock for the I/O controller.: Disable Clock. To Save Power */
    CLOCK_DisableClock(kCLOCK_Iocon);

    gpioPinConfig.pinDirection = kGPIO_DigitalOutput;                            /* Set Wakeup GPIO as output */
    gpioPinConfig.outputLogic  = 0u;                                             /* output high as default. */
    GPIO_PinInit(GPIO, FACEIDWAKE_PORT, FACEIDWAKE_PIN, &gpioPinConfig);

    GPIO_PinWrite(GPIO, FACEIDWAKE_PORT, FACEIDWAKE_PIN, 1u);

    g_FACEIDSystick    = 0;
    g_FACEIDrxIndex    = 0;
    memset((uint8_t*)g_FACEIDRecvBuf, 0x00, FACEID_BUFFER_SIZE);

    /*
     * config.baudRate_Bps = 115200U;
     * config.parityMode = kUSART_ParityDisabled;
     * config.stopBitCount = kUSART_OneStopBit;
     * config.loopback = false;
     * config.enableTx = false;
     * config.enableRx = false;
     */
    CLOCK_AttachClk(FACEID_UART_CLKATTACH);
    USART_GetDefaultConfig(&config);
    config.baudRate_Bps = 115200;
    config.enableTx = true;
    config.enableRx = true;

    USART_Init(FACEID_UART, &config, FACEID_UART_CLKFREQ);
    /* Enable RX interrupt. */
    USART_EnableInterrupts(FACEID_UART, kUSART_RxLevelInterruptEnable | kUSART_RxErrorInterruptEnable);
    NVIC_SetPriority(FACEID_UART_IRQNUM, 98);
    NVIC_EnableIRQ(FACEID_UART_IRQNUM);
    g_FACEIDTaskStatus = FACEIDTASKIDLE;

	GPIO_PinWrite(GPIO, FACEIDWAKE_PORT, FACEIDWAKE_PIN, 0u);
	faceid_delayms(100);
	GPIO_PinWrite(GPIO, FACEIDWAKE_PORT, FACEIDWAKE_PIN, 1u);
	faceid_delayms(500);

    g_FACEIDDeviceStauts = 0; // active
}

/**
 * @brief   Clean FACEID Task Status
 * @param   NULL
 * @return  NULL
 */
static void faceid_status_clean(void)
{
    g_FACEIDrxIndex = 0;
    memset((uint8_t*)g_FACEIDRecvBuf, 0x00, FACEID_BUFFER_SIZE);
    g_FACEIDCmdStatus = 0;
}


/**
 * @brief   FACE ID Tasks Loop
 * @param   NULL
 * @return  FACEID Task Status
 */
uint32_t faceid_task(uint8_t *buf, uint32_t* ret)
{
    uint32_t i;
    volatile uint8_t *g_FACEIDCmdCmp  = NULL;
    volatile uint8_t *g_FACEIDCmdCmp2 = NULL;
    strupr((char *)buf);

#if 0
    PRINTF("&&&& ");
    PRINTF("%s\r\n", buf);
#endif

/**************************** result   **************************************/
    g_FACEIDCmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACEMODE=LP");
	if(g_FACEIDCmdCmp != NULL)
	{
		// TODO : MP3 play
		g_FACEIDDeviceStauts = 1; // sleep
		PRINTF("&&& AT+FACEMODE=LP %d\r\n", g_FACEIDDeviceStauts);
		g_FACEIDDeviceStauts = 1; // sleep
		return 0;
	}

    g_FACEIDCmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACERES=FAIL");
	if(g_FACEIDCmdCmp != NULL)
	{
        // MP3 play: face recognition failed
        APP_MP3_Play(30);
		PRINTF("&&& AT+FACERES=FAIL\r\n");
		return FACEIDUNVALIDE;
	}

    g_FACEIDCmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACERES=");
	if(g_FACEIDCmdCmp != NULL)
	{
        // MP3 play: face recognition success
        APP_MP3_Play(29);
		PRINTF("&&& %s", g_FACEIDCmdCmp);
		return FACEIDVALIDE;
	}

/**************************** registration  **************************************/
    g_FACEIDCmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACEREG=DUPLICATE");
    if (g_FACEIDCmdCmp != NULL)
    {
        // TODO : MP3 play
        PRINTF("&&& AT+FACEREG=DUPLICATE\r\n");
        APP_BLE_Printf("%s", g_FACEIDCmdCmp);
        return 0;
    }

    g_FACEIDCmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACEDREG=OK");
    if (g_FACEIDCmdCmp != NULL)
    {
        // TODO : MP3 play
        PRINTF("&&& AT+FACEDREG=OK\r\n");
        return 0;
    }

    g_FACEIDCmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACEDEL=SUCCESS");
    if (g_FACEIDCmdCmp != NULL)
    {
        // TODO : MP3 play
        PRINTF("&&& AT+FACEDEL=SUCCESS\r\n");
        return 0;
    }

    g_FACEIDCmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACEDEL=FAIL");
    if (g_FACEIDCmdCmp != NULL)
    {
        // TODO : MP3 play
        PRINTF("&&& AT+FACEDEL=FAIL\r\n");
        return 0;
    }

    g_FACEIDCmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACERREG=DUPLICATE");
	if(g_FACEIDCmdCmp != NULL)
	{
		// TODO : MP3 play
		PRINTF("&&& AT+FACERREG=DUPLICATE\r\n");
		APP_BLE_Printf("%s", g_FACEIDCmdCmp);
		return 0;
	}

    g_FACEIDCmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACERREG=OK");
	if(g_FACEIDCmdCmp != NULL)
	{
		// TODO : MP3 play
		PRINTF("&&& AT+FACERREG=OK\r\n");
		APP_BLE_Printf("%s", g_FACEIDCmdCmp);
		return 0;
	}

    g_FACEIDCmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACERREG=FAIL");
	if(g_FACEIDCmdCmp != NULL)
	{
		// TODO : MP3 play
		PRINTF("&&& AT+FACERREG=FAIL\r\n");
		APP_BLE_Printf("%s", g_FACEIDCmdCmp);
		return 0;
	}

    g_FACEIDCmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACERREG=");
    if (g_FACEIDCmdCmp != NULL)
    {
        // TODO: need record # here
        g_FACEIDVIZNid = 0xFF;
        sscanf((void *)g_FACEIDCmdCmp, "AT+FACERREG=%d\r\n", &g_FACEIDVIZNid);
        if ((g_FACEIDVIZNid <= 50) && (g_UserIndexRecord <= 50))
        {
            APP_SYS_VIZNIDUpdate(g_UserIndexRecord, g_FACEIDVIZNid);
        }
        PRINTF("&&&  %s", g_FACEIDCmdCmp);
        APP_BLE_Printf("%s", g_FACEIDCmdCmp);
        return 0;
    }

#if 0
    g_FACEIDCmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACEREG=DUPLICATE");
	if(g_FACEIDCmdCmp != NULL)
	{
		// TODO : MP3 play
		PRINTF("&&& AT+FACEREG=DUPLICATE\r\n");
		APP_BLE_Printf("%s", g_FACEIDCmdCmp);
		return 0;
	}
#endif

    g_FACEIDCmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACEREG=LEFT");
	if(g_FACEIDCmdCmp != NULL)
	{
		// TODO : MP3 play
		PRINTF("&&& AT+FACEREG=LEFT\r\n");
		return 0;
	}

    g_FACEIDCmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACEREG=RIGHT");
	if(g_FACEIDCmdCmp != NULL)
	{
		// TODO : MP3 play
		PRINTF("&&& AT+FACEREG=RIGHT\r\n");
		return 0;
	}

    g_FACEIDCmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACEREG=FAKE");
	if(g_FACEIDCmdCmp != NULL)
	{
		// TODO : MP3 play
		PRINTF("&&& AT+FACEREG=FAKE\r\n");
		return 0;
	}

    g_FACEIDCmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACEREG=FRONT");
	if(g_FACEIDCmdCmp != NULL)
	{
		// TODO : MP3 play
		PRINTF("&&&  AT+FACEREG=FRONT\r\n");
		return 0;
	}

    // TODO: remove this command
    g_FACEIDCmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACEREG=OK");
	if(g_FACEIDCmdCmp != NULL)
	{
		PRINTF("&&&  AT+FACEREG=OK\r\n");
		APP_BLE_Printf("%s", "AT+FACEREG=OK\r\n");
		return 0;
	}

    g_FACEIDCmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACEREG=FAIL");
	if(g_FACEIDCmdCmp != NULL)
	{
        // MP3 play: enroll face failed
        APP_MP3_Play(21);
		PRINTF("&&&  AT+FACEREG=FAIL\r\n");
		APP_BLE_Printf("%s", "AT+FACEREG=FAIL\r\n");
		return 0;
	}

    g_FACEIDCmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACEREG=");
    if (g_FACEIDCmdCmp != NULL)
    {
        // MP3 play: enroll face success
        APP_MP3_Play(20);
        g_FACEIDVIZNid = 0xFF;
        sscanf((void *)g_FACEIDCmdCmp, "AT+FACEREG=%d\r\n", &g_FACEIDVIZNid);
        if ((g_FACEIDVIZNid <= 50) && (g_UserIndexRecord <= 50 ))
        {
            APP_SYS_VIZNIDUpdate(g_UserIndexRecord, g_FACEIDVIZNid);
        }
        PRINTF("&&&  %s", g_FACEIDCmdCmp);
        APP_BLE_Printf("AT+FACEREG=%d\r\n", g_FACEIDVIZNid);
        return 0;
    }

    g_FACEIDCmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACEMODE=CONFIRM");
	if(g_FACEIDCmdCmp != NULL)
	{
		// TODO : MP3 play
		PRINTF("&&& AT+FACEMODE=CONFIRM\r\n");
		return 0;
	}

    g_FACEIDCmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACERES=");
	if(g_FACEIDCmdCmp != NULL)
	{
		// TODO : MP3 play
		PRINTF("&&& AT+FACERES=\r\n");
		return 0;
	}
}

/**
 * @brief   FACE ID Tasks Loop
 * @param   NULL
 * @return  BLE Task Status
 */
uint8_t APP_FACEID_Task(void)
{
    uint32_t ret, value;

    if(g_FACEIDCmdStatus == 1)
    {
        ret = faceid_task((uint8_t *)g_FACEIDRecvBuf, &value);

        faceid_status_clean();
    }

    if(ret == FACEIDVALIDE)
    {
    	ret = FACEIDIDLE;
    	PRINTF("*** Valid User Face, unlock the door\r\n");
    	return FACEIDUNLOCK;
    }
    /* Task Return Idle Status */
    return FACEIDTASKIDLE;
}

/**
 * @brief   APP_FACEID_WAKEUP
 * @param   NULL
 * @return  NULL
 */
void APP_FACEID_WAKEUP(void)
{
	/* Check FACEID status, if in sleep mode, then we need wakeup it */
	if(g_FACEIDDeviceStauts == 1)
	{
		GPIO_PinWrite(GPIO, FACEIDWAKE_PORT, FACEIDWAKE_PIN, 0u);
		faceid_delayms(100);
		GPIO_PinWrite(GPIO, FACEIDWAKE_PORT, FACEIDWAKE_PIN, 1u);
		faceid_delayms(500);
		g_FACEIDDeviceStauts = 0;  /* Set FACEID as sleep mode */
	}
}
