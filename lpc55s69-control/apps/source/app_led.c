/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// C Standard Lib includes
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

// LPC55Sxx Chip peripherals & Boards includes
#include "fsl_common.h"
#include "fsl_clock.h"
#include "fsl_gpio.h"
#include "fsl_spi.h"
#include "fsl_iocon.h"

#include "app_led.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t g_LEDTicks = 0;
volatile uint32_t g_LEDStatus = 0xE000, g_LEDStatusBak = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * @brief   capt_delayus() -- Captouch sensor read delays.
 * @param   times   : Desired delay time / uS
 * @return  return NULL.
 */
volatile uint32_t g_SN3218DelayUs1, g_SN3218DelayUs2;
void led_delayus(uint32_t times)
{
    for (g_SN3218DelayUs1 = 0; g_SN3218DelayUs1 < times; g_SN3218DelayUs1++)
    {
        for (g_SN3218DelayUs2 = 0; g_SN3218DelayUs2 < 12; g_SN3218DelayUs2++)
        {
            ;
        }
    }
}

/**
 * @brief   LED Initialize
 * @param   NULL
 * @return  NULL
 */
void APP_LED_Init(void)
{
    /* Enables the clock for the I/O controller.: Enable Clock. */
    CLOCK_EnableClock(kCLOCK_Iocon);
    /* Debug Pin Port Init */
    IOCON->PIO[LEDMO_PORT][LEDMO_PIN]   = (LEDMO_FUNC   | LEDMO_PINCFG  ); 
    IOCON->PIO[LEDSCK_PORT][LEDSCK_PIN] = (LEDSCK_FUNC  | LEDSCK_PINCFG );
    IOCON->PIO[LEDPD_PORT][LEDPD_PIN]   = (LEDPD_FUNC   | LEDSCK_PINCFG );
    IOCON->PIO[LEDSTC_PORT][LEDSTC_PIN] = (LEDSTC_FUNC  | LEDSCK_PINCFG );
    IOCON->PIO[LEDOE_PORT][LEDOE_PIN]   = (LEDOE_FUNC   | LEDSCK_PINCFG );
    /* Disables the clock for the I/O controller.: Disable Clock. To Save Power */
    CLOCK_DisableClock(kCLOCK_Iocon);
    
	// LED 74HC595 GPIO init
    gpio_pin_config_t   gpioPinConfig;
    gpioPinConfig.pinDirection = kGPIO_DigitalOutput;
    gpioPinConfig.outputLogic  = 1u; /* output high as default. */
    
    GPIO_PinInit (GPIO, LEDPD_PORT,  LEDPD_PIN,  &gpioPinConfig);
    GPIO_PinWrite(GPIO, LEDPD_PORT,  LEDPD_PIN,  0u);

    GPIO_PinInit (GPIO, LEDSTC_PORT, LEDSTC_PIN, &gpioPinConfig);
    GPIO_PinWrite(GPIO, LEDSTC_PORT, LEDSTC_PIN, 0u);

    GPIO_PinInit (GPIO, LEDOE_PORT,  LEDOE_PIN,  &gpioPinConfig);
    GPIO_PinWrite(GPIO, LEDOE_PORT,  LEDOE_PIN,  0u);

    spi_master_config_t userConfig = {0};
    /* attach 12 MHz clock to NFC Reader's SPI */
    CLOCK_AttachClk(LED_SPI_CLKATTACH);
    /* reset FLEXCOMM for SPI */
    RESET_PeripheralReset(LED_SPI_RST);
    
    /*
     * userConfig.enableLoopback = false;
     * userConfig.enableMaster = true;
     * userConfig.polarity = kSPI_ClockPolarityActiveHigh;
     * userConfig.phase = kSPI_ClockPhaseFirstEdge;
     * userConfig.direction = kSPI_MsbFirst;
     * userConfig.baudRate_Bps = 500000U;
     */
    SPI_MasterGetDefaultConfig(&userConfig);
    userConfig.baudRate_Bps = 1000000U;
    userConfig.sselNum = (spi_ssel_t)0;
    userConfig.sselPol = (spi_spol_t)kSPI_SpolActiveAllLow;
    SPI_MasterInit(LED_SPI, &userConfig, LED_SPI_CLKFREQ);
    
//    g_LEDStatus = 0;
    APP_LED_AllOn(0);
}

/**
 * @brief   led_test
 * @param   data
 * @return  NULL
 */
void led_test(uint16_t data)
{
    /* clear tx/rx errors and empty FIFOs */
    LED_SPI->FIFOCFG  |= SPI_FIFOCFG_EMPTYTX_MASK | SPI_FIFOCFG_EMPTYRX_MASK;
    LED_SPI->FIFOSTAT |= SPI_FIFOSTAT_TXERR_MASK | SPI_FIFOSTAT_RXERR_MASK;
    LED_SPI->FIFOWR    = 0x0F300000|data;
    /* wait if TX FIFO of previous transfer is not empty */
    while ((LED_SPI->FIFOSTAT & SPI_FIFOSTAT_RXNOTEMPTY_MASK) == 0)
    {
        ;
    }
    GPIO_PinWrite(GPIO, LEDSTC_PORT, LEDSTC_PIN, 1u);
    led_delayus(10);
    GPIO_PinWrite(GPIO, LEDSTC_PORT, LEDSTC_PIN, 0u);
    led_delayus(10);
    GPIO_PinWrite(GPIO, LEDSTC_PORT, LEDSTC_PIN, 1u);
}

/**
 * @brief     APP_LED_Set
 * @param     num - LED numbers, status - 0/1 off/on
 * @return    NULL
 */
void APP_LED_Set(uint8_t num, uint8_t status)
{
    g_LEDTicks = LEDOSINTERVAL/5;
    
    if(num == '0') {
        g_LEDStatus |= LED_CH_11;   
    }
    else if(num == '1') {
        g_LEDStatus |= LED_CH_01; 
    }
    else if(num == '2') {
        g_LEDStatus |= LED_CH_02; 
    }
    else if(num == '3') {
        g_LEDStatus |= LED_CH_03;
    }
    else if(num == '4') {
        g_LEDStatus |= LED_CH_04;
    }
    else if(num == '5') {
        g_LEDStatus |= LED_CH_05;   
    } 
    else if(num == '6') {
        g_LEDStatus |= LED_CH_06;
    }
    else if(num == '7') {
        g_LEDStatus |= LED_CH_07; 
    } 
    else if(num == '8') {
        g_LEDStatus |= LED_CH_08;
    } 
    else if(num == '9') {
        g_LEDStatus |= LED_CH_09;  
    } 
    else if(num == '*') {
        g_LEDStatus |= LED_CH_10;
    } 
    else if(num == '#') {
        g_LEDStatus |= LED_CH_12;  
    } 
    else if(num == 'A') {
        g_LEDStatus |= LED_CH_13;  
    }
    else if(num == 'B') {
        g_LEDStatus |= LED_CH_14;  
    } 
    else if(num == 'C') {
        g_LEDStatus |= LED_CH_15;  
    } 
    else if(num == 'D') {
        g_LEDStatus |= LED_CH_16;  
    } 
    else {
        g_LEDStatus = 0x0000;
    }
    
    GPIO_PinWrite(GPIO, LEDSTC_PORT, LEDSTC_PIN, 0u);
    /* clear tx/rx errors and empty FIFOs */
    LED_SPI->FIFOCFG  |= SPI_FIFOCFG_EMPTYTX_MASK | SPI_FIFOCFG_EMPTYRX_MASK;
    LED_SPI->FIFOSTAT |= SPI_FIFOSTAT_TXERR_MASK | SPI_FIFOSTAT_RXERR_MASK;
    LED_SPI->FIFOWR    = 0x0F300000 | (0x00001FFF & (~g_LEDStatus));
    /* wait if TX FIFO of previous transfer is not empty */
    while ((LED_SPI->FIFOSTAT & SPI_FIFOSTAT_RXNOTEMPTY_MASK) == 0)
    {
        ;
    }
    
    GPIO_PinWrite(GPIO, LEDSTC_PORT, LEDSTC_PIN, 1u);    
}

/**
 * @brief     APP_LED_AllOn
 * @param     delay - delay how many ticks
 * @return    NULL
 */
void APP_LED_AllOn(uint8_t delay)
{
    if(delay == 1)  g_LEDTicks = LEDOSINTERVAL;
    else            g_LEDTicks = LEDOSINTERVAL*2;

    /* clear tx/rx errors and empty FIFOs */
    LED_SPI->FIFOCFG  |= SPI_FIFOCFG_EMPTYTX_MASK | SPI_FIFOCFG_EMPTYRX_MASK;
    LED_SPI->FIFOSTAT |= SPI_FIFOSTAT_TXERR_MASK | SPI_FIFOSTAT_RXERR_MASK;
    LED_SPI->FIFOWR    = 0x0F301FFF;
    /* wait if TX FIFO of previous transfer is not empty */
    while ((LED_SPI->FIFOSTAT & SPI_FIFOSTAT_RXNOTEMPTY_MASK) == 0)
    {
        ;
    }
    GPIO_PinWrite(GPIO, LEDSTC_PORT, LEDSTC_PIN, 1u);
    led_delayus(10);
    GPIO_PinWrite(GPIO, LEDSTC_PORT, LEDSTC_PIN, 0u);
    led_delayus(10);
    GPIO_PinWrite(GPIO, LEDSTC_PORT, LEDSTC_PIN, 1u);
}

/**
 * @brief     APP_LED_AllOff
 * @param     NULL
 * @return    NULL
 */
void APP_LED_AllOff(void)
{
    g_LEDTicks = LEDOSINTERVAL/5;
    
    /* clear tx/rx errors and empty FIFOs */
    LED_SPI->FIFOCFG  |= SPI_FIFOCFG_EMPTYTX_MASK | SPI_FIFOCFG_EMPTYRX_MASK;
    LED_SPI->FIFOSTAT |= SPI_FIFOSTAT_TXERR_MASK | SPI_FIFOSTAT_RXERR_MASK;
    LED_SPI->FIFOWR    = 0x0F300000;
    /* wait if TX FIFO of previous transfer is not empty */
    while ((LED_SPI->FIFOSTAT & SPI_FIFOSTAT_RXNOTEMPTY_MASK) == 0)
    {
        ;
    }
    GPIO_PinWrite(GPIO, LEDSTC_PORT, LEDSTC_PIN, 1u);
    led_delayus(10);
    GPIO_PinWrite(GPIO, LEDSTC_PORT, LEDSTC_PIN, 0u);
    led_delayus(10);
    GPIO_PinWrite(GPIO, LEDSTC_PORT, LEDSTC_PIN, 1u);
    g_LEDStatus = 0x00FFF;
}

/**
 * @brief     APP_LED_AllSet
 * @param     status - 0/1 all leds off/on
 * @return    NULL
 */
void APP_LED_AllSet(uint8_t status)
{
    if(status == 0)
    {
        APP_LED_AllOn(1);
    }
    else
    {
        APP_LED_AllOff();
    }
}

/**
 * @brief   led_task
 * @param   NULL
 * @return  NULL
 */
void APP_LED_Task(void)
{
    if( (g_LEDTicks == 0) && (g_LEDStatus != 0) )
    {
        APP_LED_AllOn(0);
        g_LEDStatus = 0;
    }
}

/**
 * @brief   APP_LED_TestTask
 * @param   NULL
 * @return  NULL
 */
volatile uint32_t g_LEDTestNum = 0x01;
void APP_LED_TestTask(void)
{
    led_test(g_LEDTestNum);
    g_LEDTestNum = g_LEDTestNum<<1;
    if(g_LEDTestNum == 0x10000) g_LEDTestNum = 0x01;
}

/**
 * @brief   APP_LED_Disable
 * @param   NULL
 * @return  NULL
 */
void APP_LED_Disable(void)
{
    GPIO_PinWrite(GPIO, LEDOE_PORT,  LEDOE_PIN,  1u);
}
