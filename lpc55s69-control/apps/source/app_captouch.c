/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>

#include "fsl_common.h"
#include "fsl_clock.h"
#include "fsl_gpio.h"
#include "fsl_i2c.h"
#include "fsl_inputmux.h"
#include "fsl_iocon.h"

#include "app_captouch.h"
#include "multi_button.h"

#include "app_syscfg.h"

#if CAPTTASKDEBUG || CAPTFUNCDEBUG
#include "app_printf.h"
#endif

#if CAPTLEDENABLE
#include "app_led.h"
#endif

#include "app_syscfg.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
volatile uint8_t  g_captTaskEnable = 0x00;       /* Enable/Disable Captouch Task */
volatile uint8_t  g_CAPTaskStatus  = 0x00;       /* Captouch Task Status */

static   uint32_t s_captTaskFlag   = 0x00000000; /* Captouch flash mask */
static   uint32_t s_captSystick    = 0x00UL;     /* touched filter */
static   uint16_t s_captValue      = 0xFFFF;

static   uint8_t  s_captTxBuf[16];               /* Captouch IC transfer template data buffer */
static   uint8_t  s_captRxBuf[16];               /* Captouch IC received template data buffer */

CAPTData_Type     s_capTKeys[CAPTSTORAGENUMS];
CAPTData_Type     s_capTKeysInput;

static   uint8_t  s_KL16ZBuf[4];
static   uint8_t  s_TouchValue[17];

struct   Button   g_CapButton[17];
volatile uint8_t  btn_event_val[17];

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/**
 * @brief   KL16Z_Delayus() -- Captouch sensor read delays.
 * @param   times   : Desired delay time / uS
 * @return  return NULL.
 */
static uint32_t s_captDelayUs1, s_captDelayUs2;
void KL16Z_Delayus(uint32_t times)
{
    for (s_captDelayUs1 = 0; s_captDelayUs1 < times; s_captDelayUs1++)
    {
        for (s_captDelayUs2 = 0; s_captDelayUs2 < 12; s_captDelayUs2++)
        {
            ;
        }
    }
}

/**
 * @brief   KL16Z_I2CWrite() -- write sensor register value to address
 * @param   addr -- device command address, value -- set data
 * @return  0x00 -- good, 0xFF -- failed
 */
static uint8_t KL16Z_I2CWrite(uint8_t addr, uint8_t value)
{
    status_t reVal = kStatus_Fail;
    if (kStatus_Success == I2C_MasterStart(CAPTOUCH_I2C, KL16ZI2CGNDADDR, kI2C_Write))
    {
        s_captTxBuf[0] = addr;
        s_captTxBuf[1] = value;
        reVal = I2C_MasterWriteBlocking(CAPTOUCH_I2C, (const void *)s_captTxBuf, 2, 0);
        if (reVal != kStatus_Success)
        {
#if CAPTFUNCDEBUG
            PRINTF("!!!KL16Z I2C MasterWriteBlocking %x %x\r\n", addr, value);
#endif
            return 0xFF;
        }
        reVal = I2C_MasterStop(CAPTOUCH_I2C);
        if (reVal != kStatus_Success)
        {
            PRINTF("!!!KL16Z I2C MasterStop \r\n");
            return 0xFF;
        }
        KL16Z_Delayus(20);
    }
    return 0x00;
}

/**
 * @brief   KL16Z_I2CRead() -- read sensor register value from address
 * @param   NULL
 * @return  NULL
 */
static uint8_t KL16Z_I2CRead(uint8_t addr, uint8_t* value)
{
    status_t reVal = kStatus_Fail;
    KL16Z_Delayus(2000);
    s_captTxBuf[0] = addr;
    if (kStatus_Success == I2C_MasterStart(CAPTOUCH_I2C, KL16ZI2CGNDADDR, kI2C_Write))
    {
        reVal = I2C_MasterWriteBlocking(CAPTOUCH_I2C, (void *)s_captTxBuf, 1, 0);
        if (reVal != kStatus_Success)
        {
#if CAPTFUNCDEBUG
            PRINTF("!!!KL16Z I2C MasterWriteBlocking %x\r\n", addr);
#endif
            return 0xFF;
        }
        reVal = I2C_MasterStop(CAPTOUCH_I2C);
        if (reVal != kStatus_Success)
        {
#if CAPTFUNCDEBUG
            PRINTF("!!!KL16Z I2C Read MasterStop \r\n");
#endif
            return 0xFF;
        }
    }
    reVal = I2C_MasterRepeatedStart(CAPTOUCH_I2C, KL16ZI2CGNDADDR, kI2C_Read);
    if (kStatus_Success == reVal)
    {
        reVal = I2C_MasterReadBlocking(CAPTOUCH_I2C, (void *)s_captRxBuf, 2, 0);
        if (reVal != kStatus_Success)
        {
#if CAPTFUNCDEBUG
            PRINTF("!!!KL16Z I2C I2C_MasterReadBlocking %x\r\n", s_captRxBuf[0]);
#endif
            return 0xFF;
        }
        reVal = I2C_MasterStop(CAPTOUCH_I2C);
        if (reVal != kStatus_Success)
        {
#if CAPTFUNCDEBUG
            PRINTF("!!!KL16Z I2C Read MasterStop \r\n");
#endif
            return 0xFF;
        }
        KL16Z_Delayus(20);
    }
    *value = s_captRxBuf[0];
    return 0x00;
}

#if 0
uint8_t read_btn0_status(void)
{
    if(s_TouchValue[0] == 0x03) return 0;
    else return 1;
}

uint8_t read_btn1_status(void)
{
    if(s_TouchValue[1] == 0x03) return 0;
    else return 1;
}

uint8_t read_btn2_status(void)
{
    if(s_TouchValue[2] == 0x03) return 0;
    else return 1;
}

uint8_t read_btn3_status(void)
{
    if(s_TouchValue[3] == 0x03) return 0;
    else return 1;
}

uint8_t read_btn4_status(void)
{
    if(s_TouchValue[4] == 0x03) return 0;
    else return 1;
}

uint8_t read_btn5_status(void)
{
    if(s_TouchValue[5] == 0x03) return 0;
    else return 1;
}

uint8_t read_btn6_status(void)
{
    if(s_TouchValue[6] == 0x03) return 0;
    else return 1;
}

uint8_t read_btn7_status(void)
{
    if(s_TouchValue[7] == 0x03) return 0;
    else return 1;
}

uint8_t read_btn8_status(void)
{
    if(s_TouchValue[8] == 0x03) return 0;
    else return 1;
}

uint8_t read_btn9_status(void)
{
    if(s_TouchValue[9] == 0x03) return 0;
    else return 1;
}

uint8_t read_btn10_status(void)
{
    if(s_TouchValue[10] == 0x03) return 0;
    else return 1;
}

uint8_t read_btn11_status(void)
{
    if(s_TouchValue[11] == 0x03) return 0;
    else return 1;
}

uint8_t read_btn12_status(void)
{
    if(s_TouchValue[12] == 0x03) return 0;
    else return 1;
}

uint8_t read_btn13_status(void)
{
    if(s_TouchValue[13] == 0x03) return 0;
    else return 1;
}

uint8_t read_btn14_status(void)
{
    if(s_TouchValue[14] == 0x03) return 0;
    else return 1;
}

uint8_t read_btn15_status(void)
{
    if(s_TouchValue[15] == 0x03) return 0;
    else return 1;
}
#else
uint8_t read_btn0_status(void)
{
    if(s_TouchValue[0] == 0x02) return 0;
    else return 1;
}

uint8_t read_btn1_status(void)
{
    if(s_TouchValue[1] == 0x02) return 0;
    else return 1;
}

uint8_t read_btn2_status(void)
{
    if(s_TouchValue[2] == 0x02) return 0;
    else return 1;
}

uint8_t read_btn3_status(void)
{
    if(s_TouchValue[3] == 0x02) return 0;
    else return 1;
}

uint8_t read_btn4_status(void)
{
    if(s_TouchValue[4] == 0x02) return 0;
    else return 1;
}

uint8_t read_btn5_status(void)
{
    if(s_TouchValue[5] == 0x02) return 0;
    else return 1;
}

uint8_t read_btn6_status(void)
{
    if(s_TouchValue[6] == 0x02) return 0;
    else return 1;
}

uint8_t read_btn7_status(void)
{
    if(s_TouchValue[7] == 0x02) return 0;
    else return 1;
}

uint8_t read_btn8_status(void)
{
    if(s_TouchValue[8] == 0x02) return 0;
    else return 1;
}

uint8_t read_btn9_status(void)
{
    if(s_TouchValue[9] == 0x02) return 0;
    else return 1;
}

uint8_t read_btn10_status(void)
{
    if(s_TouchValue[10] == 0x02) return 0;
    else return 1;
}

uint8_t read_btn11_status(void)
{
    if(s_TouchValue[11] == 0x02) return 0;
    else return 1;
}

uint8_t read_btn12_status(void)
{
    if(s_TouchValue[12] == 0x02) return 0;
    else return 1;
}

uint8_t read_btn13_status(void)
{
    if(s_TouchValue[13] == 0x02) return 0;
    else return 1;
}

uint8_t read_btn14_status(void)
{
    if(s_TouchValue[14] == 0x02) return 0;
    else return 1;
}

uint8_t read_btn15_status(void)
{
    if(s_TouchValue[15] == 0x02) return 0;
    else return 1;
}

uint8_t read_btn16_status(void)
{
    if(GPIO_PinRead(GPIO, ALARM_PORT,  ALARM_PIN) == 0) return 0;
    else return 1;
}
#endif

/**
 * @brief   KL16Z_Init() -- initialize captouch ic - KL16Z
 * @param   NULL
 * @return  NULL
 */
static void KL16Z_Init(void)
{
    button_init(&g_CapButton[0],  read_btn0_status,  0);
    button_init(&g_CapButton[1],  read_btn1_status,  0);
    button_init(&g_CapButton[2],  read_btn2_status,  0);
    button_init(&g_CapButton[3],  read_btn3_status,  0);
    button_init(&g_CapButton[4],  read_btn4_status,  0);
    button_init(&g_CapButton[5],  read_btn5_status,  0);
    button_init(&g_CapButton[6],  read_btn6_status,  0);
    button_init(&g_CapButton[7],  read_btn7_status,  0);
    button_init(&g_CapButton[8],  read_btn8_status,  0);
    button_init(&g_CapButton[9],  read_btn9_status,  0);
    button_init(&g_CapButton[10], read_btn10_status, 0);
    button_init(&g_CapButton[11], read_btn11_status, 0);
    button_init(&g_CapButton[12], read_btn12_status, 0);
    button_init(&g_CapButton[13], read_btn13_status, 0);
    button_init(&g_CapButton[14], read_btn14_status, 0);
    button_init(&g_CapButton[15], read_btn15_status, 0);
    button_init(&g_CapButton[16], read_btn16_status, 0);
    
    button_start(&g_CapButton[0]);
    button_start(&g_CapButton[1]);
    button_start(&g_CapButton[2]);
    button_start(&g_CapButton[3]);
    button_start(&g_CapButton[4]);
    button_start(&g_CapButton[5]);
    button_start(&g_CapButton[6]);
    button_start(&g_CapButton[7]);
    button_start(&g_CapButton[8]);
    button_start(&g_CapButton[9]);
    button_start(&g_CapButton[10]);
    button_start(&g_CapButton[11]);
    button_start(&g_CapButton[12]);
    button_start(&g_CapButton[13]);
    button_start(&g_CapButton[14]);
    button_start(&g_CapButton[15]);
    button_start(&g_CapButton[16]);
}

void APP_CAPT_RESET_1(void)
{
	GPIO_PinWrite(GPIO, CAPTRST_PORT, CAPTRST_PIN, 1u);
}

void APP_CAPT_RESET_0(void)
{
	GPIO_PinWrite(GPIO, CAPTRST_PORT, CAPTRST_PIN, 0u);
}


/**
 * @brief   CAPT_KL16ZInit() -- initialize captouch ic
 * @param   NULL
 * @return  NULL
 */
void APP_CAPT_Init(void)
{
    i2c_master_config_t masterConfig;
    gpio_pin_config_t   gpioPinConfig;
    
    /* Enables the clock for the I/O controller.: Enable Clock. */
    CLOCK_EnableClock(kCLOCK_Iocon);
    
#if TOUCHIC_I2C_SHARED

#else
    IOCON->PIO[CAPTSDA_PORT][CAPTSDA_PIN] = (CAPTSDA_FUNC  | CAPTSDA_PINCFG );  /* Touch KL16Z I2C SDA Pin */
    IOCON->PIO[CAPTSCL_PORT][CAPTSCL_PIN] = (CAPTSCL_FUNC  | CAPTSCL_PINCFG );  /* Touch KL16Z I2C SCL Pin */
#endif

#ifdef CAPTRST_PIN
    IOCON->PIO[CAPTRST_PORT][CAPTRST_PIN] = (CAPTRST_FUNC  | CAPTRST_PINCFG );  /* Touch KL16Z Reset */
#endif
    IOCON->PIO[CAPTIRQ_PORT][CAPTIRQ_PIN] = (CAPTIRQ_FUNC  | CAPTIRQ_PINCFG );  /* Touch KL16Z Interrupt */

#ifdef ALARM_PIN
    IOCON->PIO[ALARM_PORT][ALARM_PIN] = (ALARM_FUNC  | ALARM_PINCFG );          /* Alarm function pin */
#endif

    /* Disables the clock for the I/O controller.: Disable Clock. To Save Power */
    CLOCK_DisableClock(kCLOCK_Iocon);
    
    gpioPinConfig.pinDirection = kGPIO_DigitalInput;
    gpioPinConfig.outputLogic  = 1u; /* output high as default. */
    
#ifdef CAPTRST_PIN
    GPIO_PinInit(GPIO, CAPTIRQ_PORT, CAPTIRQ_PIN, &gpioPinConfig);
    gpioPinConfig.pinDirection = kGPIO_DigitalOutput;
    GPIO_PinInit(GPIO, CAPTRST_PORT, CAPTRST_PIN, &gpioPinConfig);
#endif

#ifdef ALARM_PIN
    gpioPinConfig.pinDirection = kGPIO_DigitalInput;
    gpioPinConfig.outputLogic  = 1u; /* input. */
    GPIO_PinInit(GPIO, ALARM_PORT, ALARM_PIN, &gpioPinConfig);
#endif

#if TOUCHIC_I2C_SHARED

#else
    /* attach 12 MHz clock to FLEXCOMM4 (I2C master) */
    CLOCK_AttachClk(CAPTOUCH_I2C_CLKATTACH);
    /* reset FLEXCOMM for I2C */
    RESET_PeripheralReset(CAPTOUCH_I2C_RST);
    /*
     * masterConfig.debugEnable = false;
     * masterConfig.ignoreAck = false;
     * masterConfig.pinConfig = kI2C_2PinOpenDrain;
     * masterConfig.baudRate_Bps = 100000U;
     * masterConfig.busIdleTimeout_ns = 0;
     * masterConfig.pinLowTimeout_ns = 0;
     * masterConfig.sdaGlitchFilterWidth_ns = 0;
     * masterConfig.sclGlitchFilterWidth_ns = 0;
     */
    I2C_MasterGetDefaultConfig(&masterConfig);
    /* Change the default baudrate configuration */
    masterConfig.baudRate_Bps = CAPTOUCH_I2C_RATE;
    /* Initialize the I2C master peripheral */
    I2C_MasterInit(CAPTOUCH_I2C, &masterConfig, CAPTOUCH_I2C_CLKFREQ);
#endif

#ifdef CAPTRST_PIN
    /* KL16Z is high reset */
    GPIO_PinWrite(GPIO, CAPTRST_PORT, CAPTRST_PIN, 1);
    KL16Z_Delayus(10);
    GPIO_PinWrite(GPIO, CAPTRST_PORT, CAPTRST_PIN, 0);
    KL16Z_Delayus(2000);     /* Delay 2mS */
    GPIO_PinWrite(GPIO, CAPTRST_PORT, CAPTRST_PIN, 1);
#endif
    KL16Z_Delayus(100000);   /* Delay 100mS to wait IC normal */

    KL16Z_Init();            /* Init KL16Z */

    s_capTKeysInput.status = 0x01;
    memset(s_capTKeysInput.key, 0xFF, CAPTKEYLENGTH);
}


/**
 * @brief   CAPT_KL16ZInit() -- initialize captouch ic
 * @param   NULL
 * @return  NULL
 */
void APP_CAPT_ReInit(void)
{
    i2c_master_config_t masterConfig;
    gpio_pin_config_t   gpioPinConfig;
    
    /* Enables the clock for the I/O controller.: Enable Clock. */
    CLOCK_EnableClock(kCLOCK_Iocon);
    
    IOCON->PIO[CAPTSDA_PORT][CAPTSDA_PIN] = (CAPTSDA_FUNC  | CAPTSDA_PINCFG );  /* Touch KL16Z I2C SDA Pin */
    IOCON->PIO[CAPTSCL_PORT][CAPTSCL_PIN] = (CAPTSCL_FUNC  | CAPTSCL_PINCFG );  /* Touch KL16Z I2C SCL Pin */

    IOCON->PIO[CAPTIRQ_PORT][CAPTIRQ_PIN] = (CAPTIRQ_FUNC  | CAPTIRQ_PINCFG );  /* Touch KL16Z Interrupt */
    
    IOCON->PIO[0][15] = (IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN);

    /* Disables the clock for the I/O controller.: Disable Clock. To Save Power */
    CLOCK_DisableClock(kCLOCK_Iocon);
    
    gpioPinConfig.pinDirection = kGPIO_DigitalInput;
    gpioPinConfig.outputLogic  = 0u; /* output high as default. */
    GPIO_PinInit(GPIO, 0,  15,  &gpioPinConfig);

    /* attach 12 MHz clock to FLEXCOMM4 (I2C master) */
    CLOCK_AttachClk(CAPTOUCH_I2C_CLKATTACH);
    /* reset FLEXCOMM for I2C */
    RESET_PeripheralReset(CAPTOUCH_I2C_RST);
    /*
     * masterConfig.debugEnable = false;
     * masterConfig.ignoreAck = false;
     * masterConfig.pinConfig = kI2C_2PinOpenDrain;
     * masterConfig.baudRate_Bps = 100000U;
     * masterConfig.busIdleTimeout_ns = 0;
     * masterConfig.pinLowTimeout_ns = 0;
     * masterConfig.sdaGlitchFilterWidth_ns = 0;
     * masterConfig.sclGlitchFilterWidth_ns = 0;
     */
    I2C_MasterGetDefaultConfig(&masterConfig);
    /* Change the default baudrate configuration */
    masterConfig.baudRate_Bps = CAPTOUCH_I2C_RATE;
    /* Initialize the I2C master peripheral */
    I2C_MasterInit(CAPTOUCH_I2C, &masterConfig, CAPTOUCH_I2C_CLKFREQ);

    s_capTKeysInput.status = 0x01;
    memset(s_capTKeysInput.key, 0xFF, CAPTKEYLENGTH);
}


/**
 * @brief   CAPT_KL16ZRead() -- read touch pad values from captouch sensor
 * @param   NULL
 * @return  keyValues
 */
uint8_t CAPT_KL16ZRead(void)
{
    uint16_t keyValue;
    uint8_t  retValue, errCnt;
    errCnt = 3;
    //KL16Z_Delayus(2500);
    retValue = KL16Z_I2CRead(KL16Z_OUTPUT1, (uint8_t *)&s_KL16ZBuf[0]);
    if( (retValue == 0xFF) && (errCnt != 0) )
    {
        retValue = KL16Z_I2CRead(KL16Z_OUTPUT1, (uint8_t *)&s_KL16ZBuf[0]);
        errCnt --;
    }
    
    errCnt = 3;
    retValue = KL16Z_I2CRead(KL16Z_OUTPUT2, (uint8_t *)&s_KL16ZBuf[1]);
    if( (retValue == 0xFF) && (errCnt != 0) )
    {
        retValue = KL16Z_I2CRead(KL16Z_OUTPUT2, (uint8_t *)&s_KL16ZBuf[1]);
        errCnt --;
    }
    
    errCnt = 3;
    retValue = KL16Z_I2CRead(KL16Z_OUTPUT3, (uint8_t *)&s_KL16ZBuf[2]);
    if( (retValue == 0xFF) && (errCnt != 0) )
    {
        retValue = KL16Z_I2CRead(KL16Z_OUTPUT3, (uint8_t *)&s_KL16ZBuf[2]);
        errCnt --;
    }
    
    errCnt = 3;
    retValue = KL16Z_I2CRead(KL16Z_OUTPUT4, (uint8_t *)&s_KL16ZBuf[3]);
    if( (retValue == 0xFF) && (errCnt != 0) )
    {
        retValue = KL16Z_I2CRead(KL16Z_OUTPUT4, (uint8_t *)&s_KL16ZBuf[3]);
        errCnt --;
    }
    
#if CAPTFUNCDEBUG
    PRINTF("*** KL16Z Read Value %x %x %x %x \r\n", s_KL16ZBuf[0], s_KL16ZBuf[1], s_KL16ZBuf[2], s_KL16ZBuf[3]);
#endif
    s_TouchValue[0]  = (s_KL16ZBuf[0] >> 0) & 0x03;
    s_TouchValue[1]  = (s_KL16ZBuf[0] >> 2) & 0x03;
    s_TouchValue[2]  = (s_KL16ZBuf[0] >> 4) & 0x03;
    s_TouchValue[3]  = (s_KL16ZBuf[0] >> 6) & 0x03;
    s_TouchValue[4]  = (s_KL16ZBuf[1] >> 0) & 0x03;
    s_TouchValue[5]  = (s_KL16ZBuf[1] >> 2) & 0x03;
    s_TouchValue[6]  = (s_KL16ZBuf[1] >> 4) & 0x03;
    s_TouchValue[7]  = (s_KL16ZBuf[1] >> 6) & 0x03;
    s_TouchValue[8]  = (s_KL16ZBuf[2] >> 0) & 0x03;
    s_TouchValue[9]  = (s_KL16ZBuf[2] >> 2) & 0x03;
    s_TouchValue[10] = (s_KL16ZBuf[2] >> 4) & 0x03;
    s_TouchValue[11] = (s_KL16ZBuf[2] >> 6) & 0x03;
    s_TouchValue[12] = (s_KL16ZBuf[3] >> 0) & 0x03;
    s_TouchValue[13] = (s_KL16ZBuf[3] >> 2) & 0x03;
    s_TouchValue[14] = (s_KL16ZBuf[3] >> 4) & 0x03;
    s_TouchValue[15] = (s_KL16ZBuf[3] >> 6) & 0x03;
    
#if CAPTFUNCDEBUG
    PRINTF("*** Capt Value %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\r\n", 
                           s_TouchValue[0],  s_TouchValue[1],  s_TouchValue[2],  s_TouchValue[3], 
                           s_TouchValue[4],  s_TouchValue[5],  s_TouchValue[6],  s_TouchValue[7],
                           s_TouchValue[8],  s_TouchValue[9],  s_TouchValue[10], s_TouchValue[11],
                           s_TouchValue[12], s_TouchValue[13], s_TouchValue[14], s_TouchValue[15]
    );
#endif
    keyValue = 0;
    button_ticks();

    return keyValue;
}

/**
 * @brief   CAPT_KL16ZIntHandler() -- captouch touched interrupt handler
 * @param   NULL
 * @return  NULL
 */
void CAPT_KL16ZIntHandler(void)
{
    s_captTaskFlag = 0xAA5555AA;
    if(s_capTKeysInput.status == 0) s_capTKeysInput.status = 1;
}


/**
 * @brief   CAPT_KL16ZTickHandler() -- captouch touched tick handler
 * @param   NULL
 * @return  NULL
 */
void CAPT_KL16ZTickHandler(void)
{
    s_captSystick++;
    if((s_captSystick%TICKS_INTERVAL) == 1) g_captTaskEnable = 1;
}

/**
 * @brief      password_compare() -- Compare Password with input data  
 * @param      buf2,
 * @return     CAPTWRONGPWD,
 *             CAPTVALIDPWD,
 */
uint8_t PINPAD_PasswordCompare(CAPTData_Type* buf2)
{
    char *kResult = NULL;
    uint32_t i;

    for(i=0; i<50; i++)
    {
        if(g_UserCfg.upwd_info[i][0] == 0xAA)
        {
            PRINTF("!!! UPWD %d is 0xAA Len %d, Target %d %s\r\n", i, g_UserCfg.upwd_info[i][1], (*buf2).status, &g_UserCfg.upwd_info[i][2]);
            if(g_UserCfg.upwd_info[i][1] <= (*buf2).status)
            {
                kResult = strstr((const char *)((*buf2).key), (const char *)&g_UserCfg.upwd_info[i][2]);
                if(kResult != NULL) 
                {
                    PRINTF("!!! Valid Password\r\n");
                    return CAPTVALIDPWD;
                }
            }
        }
    }
    return CAPTWRONGPWD;   
}

/**
 * @brief   CAPT_KL16ZTask() -- Captouch Tasks
 * @param   NULL
 * @return  task status
 */
uint8_t APP_CAPT_Task(void)
{
    uint32_t i;
    
    if(g_captTaskEnable == 1)
    {
        g_captTaskEnable = 0;
        CAPT_KL16ZRead();
        for(i=0; i<17; i++)
        {
            if(btn_event_val[i] != get_button_event(&g_CapButton[i]))
            {
                btn_event_val[i] = get_button_event(&g_CapButton[i]);
                if(btn_event_val[i] == PRESS_DOWN) {
                	extern volatile uint32_t g_NFCReadCnt;
                	g_NFCReadCnt = 5*1000;
                    PRINTF("KEY %d Pressed Down\r\n", i);
                    APP_MP3_Play(17);
                    s_captValue = 0x00;

                    if(i == 0)   s_captValue = '1';
                    if(i == 1)   s_captValue = '2';
                    if(i == 2)   s_captValue = '3';
                    if(i == 3)   s_captValue = '4';
                    if(i == 4)   s_captValue = '5';
                    if(i == 5)   s_captValue = '6';
                    if(i == 6)   s_captValue = '7';
                    if(i == 7)   s_captValue = '8';
                    if(i == 12)  s_captValue = '9';
                    if(i == 9)   s_captValue = '0';
                    if(i == 13)  s_captValue = '*';
                    if(i == 14)  s_captValue = '#';
                    if(i == 16)  s_captValue = 0x16;

                } else if(btn_event_val[i] == SINGLE_CLICK) {
                	/* Click alarm button */
                    if(i == 16)
                    {
                    	APP_MATTER_Printf("AT+MATTERRESET=\r\n");
                    }
                } else if(btn_event_val[i] == LONG_PRESS_HOLD) {
                    PRINTF("KEY %d Pressed Hold\r\n", i);
                	/* Hold ALARM button for a while */
                    if(i == 16)
                    {
                    	__disable_irq();
                    	APP_SYS_Format(0);
                    	APP_SYS_InfoLoad();
                    	__enable_irq();
                    	APP_MP3_Play(18); APP_MP3_Play(18); APP_MP3_Play(18);
                    }
                }
            }
        } 

        if(s_captValue != KEYIDLE)
        {

            /* Switch clock type */
            if(s_captValue == 0x16)
            {
            	g_UserCfg.locktype++;
            	if(g_UserCfg.locktype >= 3) g_UserCfg.locktype = 0;
            	if(g_UserCfg.locktype == 0)
            	{
            		APP_MP3_Play(18);
            	}
            	if(g_UserCfg.locktype == 1)
            	{
            		APP_MP3_Play(18); APP_MP3_Play(18);
            	}
            	if(g_UserCfg.locktype == 2)
            	{
            		APP_MP3_Play(18); APP_MP3_Play(18); APP_MP3_Play(18);
            	}
            	APP_SYS_InfoSave();
            	s_captValue = 0;
            }

            /* Wakeup MCU 10s */
            if(s_captValue == '*') /* '*' button as backspace */
            {
#if CAPTTASKDEBUG
                PRINTF("*** CAPT delete 1bit\r\n");
#endif
#if CAPTLEDENABLE
                APP_LED_Set('*', 35);
#endif       
                /* Maybe '*' for the first touched */
                if(s_capTKeysInput.status > 1)
                {
                    s_capTKeysInput.key[s_capTKeysInput.status-2] = 0xFF;
                    s_capTKeysInput.status--;
                }
                s_captValue = KEYIDLE;
            }
            else if(s_captValue == '#') /* '#' button */
            {
#if CAPTTASKDEBUG
                PRINTF("*** CAPT Confirm\r\n");
#endif
#if CAPTLEDENABLE
                APP_LED_Set('#', 35);
#endif
                /* if input keys number is beyond valid length this input is valid */
                if(s_capTKeysInput.status > (CAPTKEYVALIDLENGTH))
                {
#if CAPTTASKDEBUG
                    PRINTF("*** CAPT KEY Input Done ");
                    for(i=0; i<s_capTKeysInput.status; i++)
                    {
                        PRINTF("0x%x ", s_capTKeysInput.key[i]);
                    }
                    PRINTF("\r\n");
#endif
                    
#if PINPADPWDCOMEN        
                    if(PINPAD_PasswordCompare(&s_capTKeysInput) == CAPTVALIDPWD)
                    {
                        /* Clean-up Key Status */
                        s_capTKeysInput.status = 1;
                        PRINTF("*** Input Right\r\n");
                        return CAPTVALIDPWD;
                    }
                    else
                    {
                        /* Clean-up Key Status */
                        s_capTKeysInput.status = 1;
                        PRINTF("*** Input Wrong\r\n");
                        return CAPTWRONGPWD;
                    }
#endif 
                    return CAPTWRONGPWD;
                }
                else
                {
                    /* Clean-up Key Status */
                    s_capTKeysInput.status = 1;
                }
            }
            else
            {
#if CAPTTASKDEBUG
                PRINTF("*** CAPT Pressed %c\r\n", s_captValue);
#endif
                if(s_capTKeysInput.status <= CAPTKEYLENGTH)
                {
                    s_capTKeysInput.key[s_capTKeysInput.status-1] = s_captValue&0x00FF;
                    s_capTKeysInput.status++;
                }
#if CAPTLEDENABLE
                APP_LED_Set(s_captValue, 35);
#endif
            }
            /* Set Captouch as IDLE mode */
            s_captValue = KEYIDLE;
        }
    }
    return CAPTTASKIDLE;
}

/**
 * @brief   CAPT_KL16ZClean() -- clean-up captouch task status
 * @param   NULL
 * @return  NULL
 */
void CAPT_KL16ZClean(void)
{
    uint32_t i;
    s_captValue = KEYIDLE;
    s_capTKeysInput.status = 0;

    /* Reset Storaged Key Status */
    for(i=0; i<CAPTKEYLENGTH; i++)
    {
        s_capTKeysInput.key[i] = 0x00;
    }
    s_captSystick = 0;
}
