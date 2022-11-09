/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <app_fingerprint.h>
#include "app.h"
#include "lpc5500_fpslib.h"
#include "app_led.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define SENSOR_PINT_PIN_INT0_SRC kINPUTMUX_GpioPort1Pin17ToPintsel


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t   g_SystickValue;
volatile uint32_t   g_StopTimeValue;

volatile uint8_t    TemplateDownStatus;
volatile uint8_t    TemplateUpStatus;
volatile uint32_t   TemplateRecTick;

volatile uint16_t   g_FPSEnrollTimes = 0;
volatile uint16_t   g_FPSEnrollTotal = 0;

volatile uint8_t    g_FPSTaskStatus;

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * @brief
 * @param
 * @return
 */
void sensor_intr_callback(pint_pin_int_t pintr, uint32_t pmatch_status)
{
    FPS_IntHandler();  	
}

/**
 * @brief
 * @param
 * @return
 */
/* Sensor Touch Interrupt Initialize */
void FPS_TouchIntInit(void)
{
	/* Initialize PINT */
    PINT_Init(PINT);
    /* Connect trigger sources to PINT */
    INPUTMUX_Init(INPUTMUX);
    INPUTMUX_AttachSignal(INPUTMUX, kPINT_PinInt0, SENSOR_PINT_PIN_INT0_SRC);
    /* Turnoff clock to inputmux to save power. Clock is only needed to make changes */
    INPUTMUX_Deinit(INPUTMUX);

    /* Setup Pin Interrupt 1 for rising edge */
    PINT_PinInterruptConfig(PINT, kPINT_PinInt0, kPINT_PinIntEnableRiseEdge, sensor_intr_callback);
    
    /* Enable callbacks for PINT0 by Index */
    PINT_EnableCallbackByIndex(PINT,kPINT_PinInt0);

	APP_NFC_TaskEnable();
	g_AliveTicks = SLEEPTIME*OSINTERVAL;            /* System Active 10 seconds */
}

/**
 * @brief
 * @param
 * @return
 */
/* MRT0 Interrupt Handler */
void MRT0_IRQHandler(void)
{
    /* Clear interrupt flag.*/
    MRT_ClearStatusFlags(MRT0, kMRT_Channel_0, kMRT_TimerInterruptFlag);
	g_SystickValue++;
	FPS_TickHandler();
}

/**
 * @brief
 * @param
 * @return
 */
void FPS_TimerInit(void)
{
    static uint32_t mrt_clock;

    mrt_config_t mrtConfig;                                                      /* Structure of initialize MRT */
    mrt_clock = CLOCK_GetFreq(kCLOCK_BusClk);

    MRT_GetDefaultConfig(&mrtConfig);                                            /* mrtConfig.enableMultiTask = false; */
    MRT_Init(MRT0, &mrtConfig);                                                  /* Init mrt module */
    MRT_SetupChannelMode(MRT0, kMRT_Channel_0, kMRT_RepeatMode);                 /* Setup Channel 0 to be repeated */
    MRT_EnableInterrupts(MRT0, kMRT_Channel_0, kMRT_TimerInterruptEnable);       /* Enable timer interrupts for channel 0 */
    EnableIRQ(MRT0_IRQn);                                                        /* Enable at the NVIC */
    MRT_StartTimer(MRT0, kMRT_Channel_0, USEC_TO_COUNT(1000U, mrt_clock));       /* Start channel 0, 1ms*/
}

/**
 * @brief
 * @param
 * @return
 */
void FPS_BTLSensorReset(uint8_t RST_LEVEL)
{
	if(RST_LEVEL == 0)  GPIO_PinWrite(GPIO, FPS_RST_PORT, FPS_RST_PIN, 0U);
	else                GPIO_PinWrite(GPIO, FPS_RST_PORT, FPS_RST_PIN, 1U);
}

/**
 * @brief
 * @param
 * @return
 */
void FPS_BTLSensorSelect(uint8_t CS_LEVEL)
{
    if(CS_LEVEL == 0)   GPIO_PinWrite(GPIO, FPS_CS_PORT,  FPS_CS_PIN,  0U);
    else                GPIO_PinWrite(GPIO, FPS_CS_PORT,  FPS_CS_PIN,  1U);
}

/**
 * @brief
 * @param
 * @return
 */
int FPS_BTLSensorSend(uint8_t nByte)
{
    uint32_t temp;
    /* clear tx/rx errors and empty FIFOs */
    FPS_SPI->FIFOCFG  |= SPI_FIFOCFG_EMPTYTX_MASK | SPI_FIFOCFG_EMPTYRX_MASK;
    FPS_SPI->FIFOSTAT |= SPI_FIFOSTAT_TXERR_MASK | SPI_FIFOSTAT_RXERR_MASK;
    FPS_SPI->FIFOWR    = nByte | 0x07300000;
    /* wait if TX FIFO of previous transfer is not empty */
    while ((FPS_SPI->FIFOSTAT & SPI_FIFOSTAT_RXNOTEMPTY_MASK) == 0)
    {
        ;
    }
    temp = (FPS_SPI->FIFORD)&0x000000FF;
    return 0;
}

/**
 * @brief
 * @param
 * @return
 */
int FPS_BTLSensorRead(uint8_t *rx_buffer,const uint32_t num)
{
	uint32_t i;
	for(i = 0; i < num; i++)
	{  
        /* clear tx/rx errors and empty FIFOs */
        FPS_SPI->FIFOCFG  |= SPI_FIFOCFG_EMPTYTX_MASK | SPI_FIFOCFG_EMPTYRX_MASK;
        FPS_SPI->FIFOSTAT |= SPI_FIFOSTAT_TXERR_MASK | SPI_FIFOSTAT_RXERR_MASK;
        FPS_SPI->FIFOWR    = 0x07300000;
        /* wait if TX FIFO of previous transfer is not empty */
        while ((FPS_SPI->FIFOSTAT & SPI_FIFOSTAT_RXNOTEMPTY_MASK) == 0)
        {
            ;
        }
        rx_buffer[i] = (FPS_SPI->FIFORD)&0x000000FF; 
	}
	return 0;
}

/**
 * @brief
 * @param
 * @return
 */
volatile uint32_t s_delayMsCnt1, s_delayMsCnt2;
static void FPS_BTLDelayms(uint32_t times)
{
    for (s_delayMsCnt1 = 0; s_delayMsCnt1 < times; s_delayMsCnt1++)
    {
        for (s_delayMsCnt2 = 0; s_delayMsCnt2 < 11000; s_delayMsCnt2++)
        {
            ;
        }
    }   
}

/**
 * @brief
 * @param
 * @return
 */
volatile uint32_t s_delayUsCnt1, s_delayUsCnt2;
static void FPS_BTLDelayus(uint32_t times)
{
    for (s_delayUsCnt1 = 0; s_delayUsCnt1 < times; s_delayUsCnt1++)
    {
        for (s_delayUsCnt2 = 0; s_delayUsCnt2 < 12; s_delayUsCnt2++)
        {
            ;
        }
    }
}

/**
 * @brief
 * @param
 * @return
 */
void FPS_BTLSensorInit(void)
{
    gpio_pin_config_t   gpioPinConfig;
    spi_master_config_t userConfig = {0};
    
    /* Enables the clock for the I/O controller.: Enable Clock. */
    CLOCK_EnableClock(kCLOCK_Iocon);
#if FPS_SPI_SHARED /* SPI shared with NFC in smart access platform */
    IOCON->PIO[FPS_RST_PORT][FPS_RST_PIN] = (FPS_RST_FUNC ); 
    IOCON->PIO[FPS_INT_PORT][FPS_INT_PIN] = (FPS_INT_FUNC );
    IOCON->PIO[FPS_CS_PORT][FPS_CS_PIN] = (FPS_CS_FUNC );
#else
    IOCON->PIO[FPS_MI_PORT][FPS_MI_PIN] = (FPS_MI_FUNC );
    IOCON->PIO[FPS_MO_PORT][FPS_MO_PIN] = (FPS_MO_FUNC );
    IOCON->PIO[FPS_CS_PORT][FPS_CS_PIN] = (FPS_CS_FUNC );
    IOCON->PIO[FPS_CK_PORT][FPS_CK_PIN] = (FPS_CK_FUNC );
#endif
    
    /* Disables the clock for the I/O controller.: Disable Clock. To Save Power */
    CLOCK_DisableClock(kCLOCK_Iocon);
    
    gpioPinConfig.pinDirection = kGPIO_DigitalOutput;
    gpioPinConfig.outputLogic  = 1u; /* output high as default. */

    GPIO_PinInit(GPIO, FPS_RST_PORT, FPS_RST_PIN, &gpioPinConfig);
    GPIO_PinInit(GPIO, FPS_CS_PORT,  FPS_CS_PIN,  &gpioPinConfig);
    
//    GPIO_PinInit(GPIO, FPS_PWR_PORT, FPS_PWR_PIN, &gpioPinConfig);

    gpioPinConfig.pinDirection = kGPIO_DigitalInput;
    gpioPinConfig.outputLogic  = 1u; /* output high as default. */
    GPIO_PinInit(GPIO, FPS_INT_PORT, FPS_INT_PIN, &gpioPinConfig);

#if FPS_SPI_SHARED

#else
    /* SPI shared with NFC in smart access platform */
    /* attach 12 MHz clock to SPI3 */
    CLOCK_AttachClk(FPS_SPI_CLKATTACH);
    /* reset FLEXCOMM for SPI */
    RESET_PeripheralReset(FPS_SPI_RST);
    
    /*
     * userConfig.enableLoopback = false;8
     * userConfig.enableMaster = true;
     * userConfig.polarity = kSPI_ClockPolarityActiveHigh;
     * userConfig.phase = kSPI_ClockPhaseFirstEdge;
     * userConfig.direction = kSPI_MsbFirst;
     * userConfig.baudRate_Bps = 500000U;
     */
    SPI_MasterGetDefaultConfig(&userConfig);
    userConfig.baudRate_Bps = FPS_SENSOR_CLOCKRATE;
    userConfig.sselNum = (spi_ssel_t)0;
    userConfig.sselPol = (spi_spol_t)kSPI_SpolActiveAllLow;
    SPI_MasterInit(FPS_SPI, &userConfig, FPS_SPI_CLKFREQ);
#endif
}

/**
 * @brief
 * @param
 * @return
 */
uint32_t FPS_BTLGetFlashAddress(void)
{
    return FPS_FLASH_ADDR;
}

/**
 * @brief
 * @param
 * @return
 */
fingersensor_initfunc_t s_fingerFuncAPI;
void APP_FPS_Init(void)
{
    uint32_t retValue;
    
    FPS_TouchIntInit();
    FPS_TimerInit();
    
    s_fingerFuncAPI.sensor_init       = FPS_BTLSensorInit;
    s_fingerFuncAPI.sensor_delayms    = FPS_BTLDelayms;
    s_fingerFuncAPI.sensor_delayus    = FPS_BTLDelayus;
    s_fingerFuncAPI.sensor_reset      = FPS_BTLSensorReset;
    s_fingerFuncAPI.sensor_select     = FPS_BTLSensorSelect;
    s_fingerFuncAPI.sensor_spirecv    = FPS_BTLSensorRead;
    s_fingerFuncAPI.sensor_spisend    = FPS_BTLSensorSend;
    s_fingerFuncAPI.sensor_getaddress = FPS_BTLGetFlashAddress;
    s_fingerFuncAPI.sensor_flashaddr  = FPS_FLASH_ADDR;
    s_fingerFuncAPI.sensor_flashsize  = FPS_FLASH_SIZE;
    
    retValue = FPS_SensorInit(s_fingerFuncAPI);
    PRINTF("*** FPS_SensorInit %d\r\n", retValue);
    retValue = FPS_SensorIdGet();
    PRINTF("*** FPS_SensorId %x\r\n", retValue);
    FPS_WorkModeSet(FPS_VERIFY_MODE);
}

/**
 * @brief
 * @param
 * @return
 */
void APP_FPS_ReInit(void)
{
    uint32_t retValue;
    FPS_TouchIntInit();
    FPS_TimerInit();
    s_fingerFuncAPI.sensor_init       = FPS_BTLSensorInit;
    s_fingerFuncAPI.sensor_delayms    = FPS_BTLDelayms;
    s_fingerFuncAPI.sensor_delayus    = FPS_BTLDelayus;
    s_fingerFuncAPI.sensor_reset      = FPS_BTLSensorReset;
    s_fingerFuncAPI.sensor_select     = FPS_BTLSensorSelect;
    s_fingerFuncAPI.sensor_spirecv    = FPS_BTLSensorRead;
    s_fingerFuncAPI.sensor_spisend    = FPS_BTLSensorSend;
    s_fingerFuncAPI.sensor_getaddress = FPS_BTLGetFlashAddress;
    s_fingerFuncAPI.sensor_flashaddr  = FPS_FLASH_ADDR;
    s_fingerFuncAPI.sensor_flashsize  = FPS_FLASH_SIZE;
    
    retValue = FPS_SensorInit(s_fingerFuncAPI);
    PRINTF("*** FPS_SensorInit %d\r\n", retValue);
    FPS_WorkModeSet(FPS_VERIFY_MODE);
}

/**
 * @brief
 * @param
 * @return
 */
uint8_t APP_FPS_Enroll_Task(uint8_t index)
{
    volatile uint8_t retValue;
    volatile uint32_t s_FPSTimeCost;
    uint8_t fp_id = 0xFF;

    APP_SYS_FP_ID_Delete(index);

    retValue = FPS_RegistTask(&fp_id, (uint16_t *)&g_FPSEnrollTimes, (uint16_t *)&g_FPSEnrollTotal);
    if (retValue == FPS_REGOK)
    {
        APP_MP3_Play(12);
        FPS_WorkModeSet(FPS_VERIFY_MODE);
        PRINTF("*** Enroll ID %d (%d) Successfully\r\n", index, fp_id);
        APP_SYS_FP_ID_Update(index, fp_id);
        APP_BLE_Printf("AT+FINGERPRINT=OK\r\n");
    }
    else if (retValue == FPS_REGFAILED)
    {
        APP_MP3_Play(13);
        FPS_WorkModeSet(FPS_VERIFY_MODE);
        PRINTF("*** Enroll ID %d Failed\r\n", index);
        APP_BLE_Printf("AT+FINGERPRINT=FAILED\r\n");
    }
    else if (retValue == FPS_REGING)
    {
        APP_MP3_Play(10);
        PRINTF("*** Enroll ID %d Ongoing (%d / %d)\r\n", index, g_FPSEnrollTimes, g_FPSEnrollTotal);
        APP_BLE_Printf("AT+FINGERPRINT=%d\r\n", g_FPSEnrollTimes);
    }

    return retValue;
}

/**
 * @brief
 * @param
 * @return
 */
uint8_t APP_FPS_Verify_Task(void)
{
    volatile uint8_t retValue;
    volatile uint32_t s_FPSTimeCost;
    uint8_t fp_id = 0xFF;

    retValue = FPS_VerifyTask(&fp_id, (uint32_t *)&s_FPSTimeCost);
    if (retValue == FPS_IMAGEVALID)
    {
        PRINTF("*** Verify ID %d Success, %dmS\r\n", fp_id, s_FPSTimeCost);
    }
    else if (retValue == FPS_IMAGEILLEGAL)
    {
        PRINTF("*** Verify Failed, %dmS\r\n", s_FPSTimeCost);
    }

    return retValue;
}
