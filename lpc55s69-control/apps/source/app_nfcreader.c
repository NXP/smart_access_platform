/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "stdint.h"
#include "rc663_config.h"
#include "hal_reg.h"
#include "rc663.h"
#include "status_code.h"
#include "iso14443_3a.h"

#include "app_printf.h"
#include "drv_nfcreader663.h"
#include "app_nfcreader.h"

#include "app_syscfg.h"

#include "fsl_gpio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
 
volatile uint8_t  g_rc663SR = 0;
volatile uint8_t  g_rc663CardId[16];  /* Read Card ID# */
volatile uint8_t  g_rc663ATQA[4] = {0x0,0x0,};

volatile uint8_t  g_rc663CardNums   = 0;
volatile uint8_t  g_rc663LoopFailed = 0;
volatile uint8_t  g_rc663UIDSize    = 0;

volatile uint8_t  g_nfcTaskEnable = 0;
volatile uint8_t  g_NFCTaskStatus;
volatile uint8_t  g_NFCCardEnrollMode = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * @brief   NFC_ReaderInit
 * @param   NULL
 * @return  NULL
 */
void APP_NFC_Init(void)
{
#if 1
	gpio_pin_config_t   gpioPinConfig;
    /* Enables the clock for the I/O controller.: Enable Clock. */
    CLOCK_EnableClock(kCLOCK_Iocon);
	IOCON->PIO[0][25] = (IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN );
	IOCON->PIO[0][7]  = (IOCON_FUNC0 | IOCON_MODE_PULLUP| IOCON_DIGITAL_EN );
	IOCON->PIO[0][2]  = (IOCON_FUNC0 | IOCON_MODE_INACT| IOCON_DIGITAL_EN );
    /* Disables the clock for the I/O controller.: Disable Clock. To Save Power */
    CLOCK_DisableClock(kCLOCK_Iocon);

    gpioPinConfig.pinDirection = kGPIO_DigitalOutput;
    gpioPinConfig.outputLogic  = 0u; /* output high as default. */

    GPIO_PinInit(GPIO, 0, 25, &gpioPinConfig);
    gpioPinConfig.outputLogic  = 1u; /* output high as default. */
    GPIO_PinInit(GPIO, 0, 7,  &gpioPinConfig);
    GPIO_PinInit(GPIO, 0, 2,  &gpioPinConfig);
#endif

    RC663_PinInit();
}

uint8_t APP_NFC_CheckValid(uint8_t *data, uint32_t cards)
{
    uint32_t i, j;
    
    for(j=0; j<cards; j++)
    {
        for(i=0; i<50; i++)  // TODO : use define instead of 50 here
        {
            if(g_UserCfg.card_info[i][0] == 0xAA)
            {     
                if(g_UserCfg.card_info[i][1] == 0x04)
                {
                    if( (data[0+j*4] == g_UserCfg.card_info[i][2]) && (data[1+j*4] == g_UserCfg.card_info[i][3]) &&
                        (data[2+j*4] == g_UserCfg.card_info[i][4]) && (data[3+j*4] == g_UserCfg.card_info[i][5])
                      )
                    {
                        return NFCCARDVALID;
                    }
                }
                if(g_UserCfg.card_info[i][1] == 0x07)
                {
                    if( (data[0+j*4] == g_UserCfg.card_info[i][2]) && (data[1+j*4] == g_UserCfg.card_info[i][3]) &&
                        (data[2+j*4] == g_UserCfg.card_info[i][4]) && (data[3+j*4] == g_UserCfg.card_info[i][5]) &&
                        (data[4+j*4] == g_UserCfg.card_info[i][6]) && (data[5+j*4] == g_UserCfg.card_info[i][7]) &&
                        (data[6+j*4] == g_UserCfg.card_info[i][8])
                      )
                    {
                        return NFCCARDVALID;
                    }
                }
                if(g_UserCfg.card_info[i][1] == 0x08)
                {
                    if( (data[0+j*4] == g_UserCfg.card_info[i][2]) && (data[1+j*4] == g_UserCfg.card_info[i][3]) &&
                        (data[2+j*4] == g_UserCfg.card_info[i][4]) && (data[3+j*4] == g_UserCfg.card_info[i][5]) &&
                        (data[4+j*4] == g_UserCfg.card_info[i][6]) && (data[5+j*4] == g_UserCfg.card_info[i][7]) &&
                        (data[6+j*4] == g_UserCfg.card_info[i][8]) && (data[7+j*4] == g_UserCfg.card_info[i][9])
                      )
                    {
                        return NFCCARDVALID;
                    }
                }
            }
        }
    }

    return NFCCARDILLEGAL;
}

uint8_t APP_NFC_CardEnroll(uint8_t num, uint8_t* RCsnr, uint8_t *len)
{
    uint32_t i;
  
    memset(g_UserCfg.card_info[num], 0x00, 32);
    g_UserCfg.card_info[num][0] = 0xAA;
    g_UserCfg.card_info[num][1] = *len;
    for(i=0; i<*len; i++)
    {
        g_UserCfg.card_info[num][2+i] = *RCsnr;
        RCsnr++;
    }
    return true;
}


/**
 * @brief   
 * @param   
 * @return  
 */
uint8_t APP_NFC_Task(void)
{
    uint32_t i;
    uint8_t  ret = 0;
    volatile uint8_t  g_nfcStatus;
    volatile uint8_t  g_nfcSAK = 0;

    if(g_nfcTaskEnable == 1)
    {
    	APP_CAPT_RESET_0();

        PcdReset();
        PICC_PcdTypeA106Config();
        g_nfcTaskEnable = 2;
    }
    if(g_nfcTaskEnable == 2)
    {
    	//__disable_irq();
        /* -- PICC_CardActiveA(unsigned char req, unsigned char *atqa, unsigned char *uid, unsigned char *uid_size, unsigned char *sak) --*/
        g_nfcStatus = PICC_CardActiveA(0x26, (uint8_t *)g_rc663ATQA, (uint8_t *)g_rc663CardId, (uint8_t *)&g_rc663UIDSize, (uint8_t *)&g_nfcSAK);
        if(!g_nfcStatus)
        {
            g_rc663CardNums++;
            PICC_HaltA();
            g_rc663SR++;
            PRINTF("UID %x: ", g_rc663CardNums);
            for(i=0; i<g_rc663UIDSize; i++)
            {
                PRINTF("%x ", g_rc663CardId[i]);
            }
            PRINTF("\r\n");
        }
        else
        {
            g_rc663LoopFailed++;
        }

        if(g_rc663LoopFailed >= 2)
        {
            PcdRfReset(10);
            if(g_rc663CardNums > 0)
            {
                if(g_NFCCardEnrollMode == 0)
                {
                    ret = APP_NFC_CheckValid((uint8_t *)g_rc663CardId, g_rc663CardNums);
                }
                else
                {
                    APP_MP3_Play(4);
                    APP_NFC_CardEnroll((g_NFCCardEnrollMode&0x7F), (uint8_t *)g_rc663CardId, (uint8_t *)&g_rc663UIDSize);   /* */
                    APP_SYS_InfoSave();
                    APP_BLE_Printf("AT+NFC=OK\r\n");
                    g_NFCCardEnrollMode = 0;
                }
                PRINTF("End of Polling, Number of cards: %x return %x\r\n \r\n \r\n", g_rc663CardNums, ret);
            }
            else
            {
                ret = NFCCARDIDLE;
            }
            g_nfcStatus = 0;
            g_rc663CardNums = 0;
            g_rc663LoopFailed = 0;
            g_rc663SR = 0;
            RC663_ResetSet(1);
            //PRINTF("!!! NFC Power Down !!! \r\n");
            APP_NFC_TaskDisable();
            APP_CAPT_RESET_1();
            //__enable_irq();
            return ret;
        }
        //__enable_irq();
    }
    return NFCCARDIDLE;
}

void APP_NFC_TaskEnable(void)
{
	//if(g_nfcTaskEnable == 0)  g_nfcTaskEnable = 1;
	 g_nfcTaskEnable = 1;
}

void APP_NFC_TaskDisable(void)
{
    g_nfcTaskEnable = 0;
}

void APP_NFC_Disable(void)
{
    GPIO_PinWrite(GPIO, NFCRST_PORT, NFCRST_PIN, 1u);
}
