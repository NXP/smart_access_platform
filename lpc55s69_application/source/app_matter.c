/*
 * app_matter.c
 *
 *  Created on: Apr 14, 2022
 *      Author: nxf65009
 */

#include "app.h"
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

#include "lpc5500_fpslib.h"

volatile uint8_t  g_MATTERRecvBuf[MATTER_BUFFER_SIZE];
volatile uint16_t g_MATTERrxIndex = 0;
volatile uint32_t g_MATTERTaskStatus = 0;
volatile uint8_t  g_MATTERCmdStatus  = 0;
volatile uint32_t g_MATTERSystick    = 0;

/**
 * @brief   Flexcomm Interrupt Handler
 * @param   NULL
 * @return  NULL
 */
void MATTER_UART_IRQHANDLER(void)
{
    uint8_t data;
    g_SysTicks = SLEEPTIME*OSINTERVAL;
    /* If new data arrived. */
    if ((kUSART_RxFifoNotEmptyFlag | kUSART_RxError) & USART_GetStatusFlags(MATTER_UART))
    {
        data = USART_ReadByte(MATTER_UART);

        /* If ring buffer is not full, add data to ring buffer. */
        if (g_MATTERrxIndex < MATTER_BUFFER_SIZE)
        {
        	g_MATTERRecvBuf[g_MATTERrxIndex] = data;
            g_MATTERrxIndex++;
        }
        else
        {
        	g_MATTERrxIndex = 0;
        }

        if( ((g_MATTERRecvBuf[g_MATTERrxIndex-2] == '\r') && (g_MATTERRecvBuf[g_MATTERrxIndex-1] == '\n')) ||
            ((g_MATTERRecvBuf[g_MATTERrxIndex-2] == '\n') && (g_MATTERRecvBuf[g_MATTERrxIndex-1] == '\r'))
          )
        {
            g_MATTERCmdStatus = 1;
        }
    }
    g_WakeupFlag = 1; // Wakeup System?
}

/**
 * @brief   RESET MATTER Module
 * @param   NULL
 * @return  NULL
 */
/* See fsl_debug_console.h for documentation of this function. */
uint32_t APP_MATTER_Printf(const char *formatString, ...)
{
    va_list arg;
    uint32_t logLength = 0U, result = 0U;
    uint8_t *PrintBuf = NULL;
    PrintBuf = malloc(256);
    if(PrintBuf == NULL)
    {
        PRINTF("*** Malloc MATTER printf buffer failed\r\n");
    }

    va_start(arg, formatString);
    /* format print log first */
    logLength = vsprintf((char *)PrintBuf, formatString, arg);

    va_end(arg);

    USART_WriteBlocking(MATTER_UART, PrintBuf, logLength);

    free(PrintBuf);

    return result;
}

/**
 * @brief   Initialize MATTER module
 * @param   rst : 0-not reset module, 1-reset module
 * @return  NULL
 */
void APP_MATTER_Init(void)
{
    usart_config_t      config;
    gpio_pin_config_t   gpioPinConfig;

    /* Enables the clock for the I/O controller.: Enable Clock. */
    CLOCK_EnableClock(kCLOCK_Iocon);
    /* Debug Pin Port Init */
	IOCON->PIO[MATTERTXD_PORT][MATTERTXD_PIN]   = (MATTERTXD_FUNC  | IOCON_MODE_INACT | IOCON_DIGITAL_EN );
	IOCON->PIO[MATTERRXD_PORT][MATTERRXD_PIN]   = (MATTERRXD_FUNC  | IOCON_MODE_INACT | IOCON_DIGITAL_EN );

    IOCON->PIO[MATTERIRQI_PORT][MATTERIRQI_PIN] = (MATTERIRQI_FUNC  | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN );
    IOCON->PIO[MATTERIRQO_PORT][MATTERIRQO_PIN] = (MATTERIRQO_FUNC  | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN );
    /* Disables the clock for the I/O controller.: Disable Clock. To Save Power */
    CLOCK_DisableClock(kCLOCK_Iocon);

    gpioPinConfig.pinDirection = kGPIO_DigitalOutput;                            /* Congig GPIO as output mode */
    gpioPinConfig.outputLogic  = 1u;                                             /* output high as default. */
    GPIO_PinInit(GPIO, MATTERIRQO_PORT, MATTERIRQO_PIN, &gpioPinConfig);

    g_MATTERSystick    = 0;
    g_MATTERrxIndex    = 0;
    memset((uint8_t*)g_MATTERRecvBuf, 0x00, MATTER_BUFFER_SIZE);

    /*
     * config.baudRate_Bps = 115200U;
     * config.parityMode = kUSART_ParityDisabled;
     * config.stopBitCount = kUSART_OneStopBit;
     * config.loopback = false;
     * config.enableTx = false;
     * config.enableRx = false;
     */
    CLOCK_AttachClk(MATTER_UART_CLKATTACH);
    USART_GetDefaultConfig(&config);
    config.baudRate_Bps = 115200;
    config.enableTx = true;
    config.enableRx = true;

    USART_Init(MATTER_UART, &config, MATTER_UART_CLKFREQ);
    /* Enable RX interrupt. */
    USART_EnableInterrupts(MATTER_UART, kUSART_RxLevelInterruptEnable | kUSART_RxErrorInterruptEnable);
    NVIC_SetPriority(MATTER_UART_IRQNUM, 99);
    NVIC_EnableIRQ(MATTER_UART_IRQNUM);

    g_MATTERTaskStatus = MATTERTASKIDLE;
}

/**·
 * @brief   Clean MATTER Status's flag and memory/buffer
 * @param   NULL
 * @return  NULL
 */
static void matterstatus_clean(void)
{
    g_MATTERrxIndex = 0;
    memset((uint8_t*)g_MATTERRecvBuf, 0x00, MATTER_BUFFER_SIZE);
    g_MATTERCmdStatus = 0;
}

/**
 * @brief   MATTER Tasks Loop
 * @param   NULL
 * @return  MATTER Task Status
 */
uint32_t matter_task(uint8_t *buf, uint32_t* ret)
{
    uint32_t i, j;
    volatile uint8_t *g_MATTERCmdCmp  = NULL;
    uint32_t id_num;

    strupr((char *)buf);
/*--------------------------------------- Smart Access MATTER Commands -------------------------------------------------------*/

    g_MATTERCmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+MATTERLOCK");
    if(g_MATTERCmdCmp != NULL)
    {
    	APP_MATTER_Printf("AT+MATTERLOCK=OK\r\n");
        return MATTERLOCK;
    }

    g_MATTERCmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+MATTERUNLOCK");
    if(g_MATTERCmdCmp != NULL)
    {
    	APP_MATTER_Printf("AT+MATTERUNLOCK=OK\r\n");
        return MATTERUNLOCK;
    }

    return MATTERTASKIDLE;
}

uint32_t APP_MATTER_Task(void)
{
    uint32_t ret, value;
    if(g_MATTERCmdStatus == 1)
    {
        ret = matter_task((uint8_t *)g_MATTERRecvBuf, &value);
        matterstatus_clean();
        g_MATTERCmdStatus = 0;
        g_MATTERrxIndex   = 0;
        return ret;
    }

    /* Task Return Idle Status */
    return MATTERTASKIDLE;
}
