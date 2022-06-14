/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "app.h"
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

#include "lpc5500_fpslib.h"

#define UWB_THRES_DISTANCE 50

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t g_BLETaskStatus = 0;
volatile uint8_t  g_BLECmdStatus  = 0;
volatile uint32_t g_BLESystick    = 0;

volatile uint8_t  g_BLERecvBuf[BLE_BUFFER_SIZE];

volatile uint16_t g_BLErxIndex = 0;                                              /* Index of the memory to save new arrived data. */
volatile uint8_t *g_BLECmdCmp  = NULL;                                           

volatile uint8_t  g_NFCCardNum = 0;
volatile uint8_t  g_eLockStatus = 0;

volatile uint16_t  g_FingerDetectNum  = 0;
volatile uint16_t  g_FingerEnrollNum  = 0;
bool g_FpEnrollInterrupted = false;

volatile uint8_t  g_BLEPwdNum = 0;
volatile uint8_t  g_BLEPwdLen = 0;
volatile uint8_t  g_BLETempbuf[32];

volatile uint8_t  g_BLEUserPWDLen = 0;
volatile uint8_t  g_BLEUserPWD[32];

volatile uint8_t  g_USERNum = 0;

volatile uint8_t  g_UserNameList[2048];

volatile uint8_t  g_UpdateUSERNum = 0;
volatile uint8_t  g_UpdateUSERNameLen = 0;
volatile uint8_t  g_UpdateUSERName[32];

volatile uint8_t  g_BLETransferLen = 0;

volatile uint8_t  g_UserIndexRecord = 0; // used for record VLN user ID
volatile uint8_t  g_BLEFaceIDdeletNum  = 0;

volatile uint8_t  g_UserCardIndex = 0xFF;

volatile uint8_t  g_UserTestBuf[1024];

volatile uint16_t dist = 0;
bool flag = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
/**
 * @brief   
 * @param   
 * @return  
 */
volatile uint32_t g_BLEmsCnt1, g_BLEmsCnt2;
void ble_delayms(uint32_t times)
{
    for (g_BLEmsCnt1 = 0; g_BLEmsCnt1 < times; g_BLEmsCnt1++)
    {
        for (g_BLEmsCnt2 = 0; g_BLEmsCnt2 < (11000); g_BLEmsCnt2++)
        {
            ;
        }
    }   
}

volatile uint32_t g_BLEusCnt1, g_BLEusCnt2;
void ble_delayus(uint32_t times)
{
    for (g_BLEusCnt1 = 0; g_BLEusCnt1 < times; g_BLEusCnt1++)
    {
        for (g_BLEusCnt2 = 0; g_BLEusCnt2 < (10); g_BLEusCnt2++)
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
void BLE_UART_IRQHANDLER(void)
{
    uint8_t data;
    g_SysTicks = SLEEPTIME*OSINTERVAL;  
    /* If new data arrived. */
    if ((kUSART_RxFifoNotEmptyFlag | kUSART_RxError) & USART_GetStatusFlags(BLE_UART))
    {
        data = USART_ReadByte(BLE_UART);

        /* If ring buffer is not full, add data to ring buffer. */
        if (g_BLErxIndex < BLE_BUFFER_SIZE)
        {
            g_BLERecvBuf[g_BLErxIndex] = data;
            g_BLErxIndex++;
        }
        else
        {
            g_BLErxIndex = 0;
        }
        
        if( ((g_BLERecvBuf[g_BLErxIndex-2] == '\r') && (g_BLERecvBuf[g_BLErxIndex-1] == '\n')) ||
            ((g_BLERecvBuf[g_BLErxIndex-2] == '\n') && (g_BLERecvBuf[g_BLErxIndex-1] == '\r'))
          )
        {
            g_BLECmdStatus = 1;
        }
#if 0
        if( (g_BLERecvBuf[g_BLErxIndex-1] == '!') )
        {
            g_BLERecvBuf[g_BLErxIndex-1] = 0x00;
            g_BLECmdStatus = 1;
        }
#endif
    }
    g_WakeupFlag = 1; // Wakeup System?
}

/**
 * @brief   RESET BLE Module
 * @param   NULL
 * @return  NULL
 */
/* See fsl_debug_console.h for documentation of this function. */
uint32_t APP_BLE_Printf(const char *formatString, ...)
{
    va_list arg;
    uint32_t logLength = 0U, result = 0U;
    uint8_t *PrintBuf = NULL;
    PrintBuf = malloc(256);
    if(PrintBuf == NULL)
    {
        PRINTF("*** Malloc BLE printf buffer failed\r\n");
    }

    va_start(arg, formatString);
    /* format print log first */
    logLength = vsprintf((char *)PrintBuf, formatString, arg);
    
    va_end(arg);
    
    USART_WriteBlocking(BLE_UART, PrintBuf, logLength);
    
    free(PrintBuf);

    return result;
}

/**
 * @brief   Initialize BLE module
 * @param   rst : 0-not reset module, 1-reset module
 * @return  NULL
 */
void APP_BLE_Init(void)
{
    usart_config_t      config;
    gpio_pin_config_t   gpioPinConfig;
    
    /* Enables the clock for the I/O controller.: Enable Clock. */
    CLOCK_EnableClock(kCLOCK_Iocon);
    /* Debug Pin Port Init */
	IOCON->PIO[BLETXD_PORT][BLETXD_PIN]   = (BLETXD_FUNC  | IOCON_MODE_INACT | IOCON_DIGITAL_EN );
	IOCON->PIO[BLERXD_PORT][BLERXD_PIN]   = (BLERXD_FUNC  | IOCON_MODE_INACT | IOCON_DIGITAL_EN );
    
    IOCON->PIO[BLEIRQI_PORT][BLEIRQI_PIN] = (BLEIRQI_FUNC  | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN );
    IOCON->PIO[BLEIRQO_PORT][BLEIRQO_PIN] = (BLEIRQO_FUNC  | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN );
    /* Disables the clock for the I/O controller.: Disable Clock. To Save Power */
    CLOCK_DisableClock(kCLOCK_Iocon);
    
    gpioPinConfig.pinDirection = kGPIO_DigitalOutput;                            /* Congig GPIO as output mode */
    gpioPinConfig.outputLogic  = 1u;                                             /* output high as default. */
    GPIO_PinInit(GPIO, BLEIRQO_PORT, BLEIRQO_PIN, &gpioPinConfig);
    
    g_BLESystick    = 0;
    g_BLErxIndex    = 0;
    memset((uint8_t*)g_BLERecvBuf, 0x00, BLE_BUFFER_SIZE);
    
    /*
     * config.baudRate_Bps = 115200U;
     * config.parityMode = kUSART_ParityDisabled;
     * config.stopBitCount = kUSART_OneStopBit;
     * config.loopback = false;
     * config.enableTx = false;
     * config.enableRx = false;
     */
    CLOCK_AttachClk(BLE_UART_CLKATTACH);
    USART_GetDefaultConfig(&config);
    config.baudRate_Bps = 115200;
    config.enableTx = true;
    config.enableRx = true;
    
    USART_Init(BLE_UART, &config, BLE_UART_CLKFREQ);
    /* Enable RX interrupt. */
    USART_EnableInterrupts(BLE_UART, kUSART_RxLevelInterruptEnable | kUSART_RxErrorInterruptEnable);
    NVIC_SetPriority(BLE_UART_IRQNUM, 99);
    NVIC_EnableIRQ(BLE_UART_IRQNUM);

    g_BLETaskStatus = BLETASKIDLE;
}

/**·
 * @brief   Clean BLE Status's flag and memory/buffer
 * @param   NULL
 * @return  NULL
 */
static void blestatus_clean(void)
{
    g_BLErxIndex = 0;
    memset((uint8_t*)g_BLERecvBuf, 0x00, BLE_BUFFER_SIZE);
    g_BLECmdStatus = 0;
}

/**
 * @brief   BLE Tasks Loop
 * @param   NULL
 * @return  BLE Task Status
 */
uint32_t ble_task(uint8_t *buf, uint32_t* ret)
{
    uint32_t i, j;
    volatile uint8_t *g_BLECmdCmp  = NULL;
    uint32_t id_num;

    strupr((char *)buf);
#if 1
    PRINTF("---");
    PRINTF("%s\r\n", buf);
#endif
/*--------------------------------------- Smart Access Commands -------------------------------------------------------*/
    g_BLECmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+APPTYPE=");
    if(g_BLECmdCmp != NULL)
    {
    	PRINTF("*** AT+APPTYPE=UWB,NFC,FACEID,PINPAD,BLE,MATTER,AUDIO,FINGERPRINT\r\n");
        APP_BLE_Printf("AT+APPTYPE=UWB,NFC,FACEID,PINPAD,BLE,MATTER,AUDIO,FINGERPRINT\r\n");
        return 0;
    }

    g_BLECmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+PWD");
    if(g_BLECmdCmp != NULL)
    {
        memset((uint8_t *)g_UpdateUSERName, 0x00, 32);
        memset((uint8_t *)g_BLETempbuf,     0x00, 32);
		memset((uint8_t *)g_BLEUserPWD,     0x00, 32);

		sscanf((void *)g_BLECmdCmp, "AT+PWD=%s\r\n", g_BLETempbuf);

		int i = 0;
		while (g_BLETempbuf[i] != ',') {
			g_UpdateUSERName[i] = g_BLETempbuf[i];
			i++;
		}
		i++;
		while (g_BLETempbuf[i] != '\0') {
			g_BLEUserPWD[i-strlen(g_UpdateUSERName)-1] = g_BLETempbuf[i];
			i++;
		}

		if(g_BLEPwdNum<50)
        {
            *ret = g_BLEPwdNum;

            id_num = APP_SYS_UserCreate(strlen((void*)g_UpdateUSERName), g_UpdateUSERName);
            if (id_num != 0xFFFFFFFF) {
                APP_SYS_PWDSet(id_num, strlen((void*)g_BLEUserPWD), (uint8_t *)g_BLEUserPWD);
                PRINTF("AT+PWD=%d\r\n",id_num);
                APP_BLE_Printf("AT+PWD=%d\r\n",id_num);
                return BLESETPWDOK;
            } else {
            	APP_BLE_Printf("AT+PWD=DUPLICATE\r\n");
                return BLESETPWDFAILED;
            }
        }

    	APP_BLE_Printf("AT+PWD=FAILED\r\n");
        return BLESETPWDFAILED;
    }

    g_BLECmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+NFC=");
    if(g_BLECmdCmp != NULL)
    {
    	APP_MP3_Play(3);
    	g_UserCardIndex = 0xFF;
    	sscanf((void *)g_BLECmdCmp, "AT+NFC=%d", &g_UserCardIndex);

        *ret = g_UserCardIndex;
        g_NFCCardEnrollMode = 0x80 + g_UserCardIndex;

    	PRINTF("--- AT+NFC=%d", g_UserCardIndex);
    	//APP_BLE_Printf("AT+NFC=OK\r\n");
    	//return BLEREGCARD;
    	return BLEREGCARDOK;
    }

    g_BLECmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FINGERPRINT");
    if(g_BLECmdCmp != NULL)
    {
    	APP_MP3_Play(9);
        //prince_encrypt(0);
    	sscanf((void *)g_BLECmdCmp, "AT+FINGERPRINT=%d", &g_FingerEnrollNum);
    	PRINTF("AT+FINGERPRINT %d\r\n", g_FingerEnrollNum);

        FPS_WorkModeSet(FPS_ENROLLMENT_MODE);

        *ret = g_FingerEnrollNum;
        g_FpEnrollInterrupted = false;

        return BLEFPENROLL;
    }

    g_BLECmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+ELOCK");
    if(g_BLECmdCmp != NULL)
    {
        sscanf((void *)g_BLECmdCmp, "AT+ELOCK=%d", &g_eLockStatus);
        PRINTF("AT+ELOCK is %d\r\n", g_eLockStatus);
        if(g_eLockStatus == 0x00)
        {
        	APP_BLE_Printf("AT+ELOCK=OK\r\n");
            return BLEUNLOCK;
        }
        else
        {
        	APP_BLE_Printf("AT+ELOCK=OK\r\n");
            return BLELOCK;
        }
    }

    g_BLECmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+UWBDIST");
    if (g_BLECmdCmp != NULL)
    {
    	sscanf((void *)g_BLECmdCmp, "AT+UWBDIST=%d", &dist);
    	APP_BLE_Printf("%d\n", dist);
    	if (dist <= UWB_THRES_DISTANCE && flag == false)
    	{
    		APP_MOTOR_Task(14);
    	    flag = true;
    	}
    	else if (dist <= UWB_THRES_DISTANCE && flag == true)
    	{
    		APP_MOTOR_Set(4);
    	}
    	else if (dist > UWB_THRES_DISTANCE && flag == true)
    	{
    		APP_MOTOR_Task(13);
    		flag = false;
    	}
    	else
    	{
    		APP_MOTOR_Set(4);
    	}
    }


    g_BLECmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+UNLOCKPASS");
    if(g_BLECmdCmp != NULL)
    {
        g_BLEPwdLen = 0;
        g_UpdateUSERNum = 0;

        memset((uint8_t *)g_UpdateUSERName, 0x00, 32);
        memset((uint8_t *)g_BLETempbuf,     0x00, 32);
		memset((uint8_t *)g_BLEUserPWD,     0x00, 32);

        sscanf((void *)g_BLECmdCmp, "AT+UNLOCKPASS=%d,%s\r\n", &g_UpdateUSERNum, g_BLEUserPWD);

        g_BLEPwdLen    = strlen((void *)g_BLEUserPWD);

        PRINTF("AT+UNLOCKPASS ID %d len %d, %s\r\n", g_UpdateUSERNum, g_BLEPwdLen, g_BLEUserPWD);

        for(i=0; i<50; i++)   /* check user index */
        {
        	if(g_UserCfg.index[i] == g_UpdateUSERNum) /* the correct index*/
        	{
        		if(g_UserCfg.upwd_info[i][0] == 0xAA) /* password exist */
        		{
        			if(g_UserCfg.upwd_info[i][1] == g_BLEPwdLen) /* password length same */
        			{
        				char *kResult = NULL;
        				kResult = NULL;
                        kResult = strstr((const char *)(g_BLEUserPWD), (const char *)&g_UserCfg.upwd_info[i][2]);
                        if(kResult != NULL) /* valid password */
                        {
                        	APP_BLE_Printf("AT+UNLOCKPASS=OK\r\n");
                        	return BLEUNLOCK;
                        }
                        else
                        {
                        	PRINTF("--- AT+UNLOCKPASS=FAILED pwd is %s\r\n", (const char *)&g_UserCfg.upwd_info[i][2]);
                        }
        			}
        		}
        	}
        }
		APP_BLE_Printf("AT+UNLOCKPASS=FAILED\r\n");
		return BLEUNLOCKFAILED;
    }

    g_BLECmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACEACTIVE=");
    if(g_BLECmdCmp != NULL)
    {
    	APP_FACEID_WAKEUP();
    	return BLEFACEACTIVE;
    }

    g_BLECmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACEREG=");
    if(g_BLECmdCmp != NULL)
    {
    	APP_FACEID_WAKEUP();
    	sscanf((void *)g_BLECmdCmp, "AT+FACEREG=%d\r\n", &g_UserIndexRecord);

    	APP_FACEID_Printf("%s", (const char *)g_BLECmdCmp);
    	return BLEFACEREG;
    }

    g_BLECmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+FACERREG=");
    if(g_BLECmdCmp != NULL)
    {
    	APP_FACEID_WAKEUP();

    	g_UserIndexRecord = 0xFF;
    	uint32_t temp = 0;
#if 0
    	if(g_BLECmdCmp[13] >= 0x30) temp += (g_BLECmdCmp[13]-0x30)*1000;
    	if(g_BLECmdCmp[14] >= 0x30) temp += (g_BLECmdCmp[14]-0x30)*100;
    	if(g_BLECmdCmp[15] >= 0x30) temp += (g_BLECmdCmp[15]-0x30)*10;
    	if(g_BLECmdCmp[16] >= 0x30) temp += (g_BLECmdCmp[16]-0x30);
#else
    	if(g_BLECmdCmp[15] >= 0)    temp += (g_BLECmdCmp[15])*10;
    	if(g_BLECmdCmp[16] >= 0)    temp += (g_BLECmdCmp[16]);

#endif
    	if(temp <= 50)
    	{
    		g_UserIndexRecord = temp;
    	}
    	ble_delayms(100);
    	PRINTF("*** FACERREG Len %d RREGNum %d \r\n", g_BLErxIndex, g_UserIndexRecord);

    	temp = 0;
#if 1
    	__disable_irq();
#if 0
    	FACEID_UARTStringSend(&g_BLECmdCmp[0],  17);
    	ble_delayus(100);
    	FACEID_UARTStringSend(&g_BLECmdCmp[17],     100);ble_delayus(1);
    	FACEID_UARTStringSend(&g_BLECmdCmp[17+100], 100);ble_delayus(1);
    	FACEID_UARTStringSend(&g_BLECmdCmp[17+200], 100);ble_delayus(1);
    	FACEID_UARTStringSend(&g_BLECmdCmp[17+300], 100);ble_delayus(1);
    	FACEID_UARTStringSend(&g_BLECmdCmp[17+400], 100);ble_delayus(1);
    	FACEID_UARTStringSend(&g_BLECmdCmp[17+500], 100);ble_delayus(1);
    	FACEID_UARTStringSend(&g_BLECmdCmp[17+600], 100);ble_delayus(1);
    	FACEID_UARTStringSend(&g_BLECmdCmp[17+700], 100);ble_delayus(1);
#else
    	FACEID_UARTStringSend(&g_BLECmdCmp[0],  g_BLErxIndex);
#endif
    	__enable_irq();
#else

  //  	FACEID_UARTStringSend(&g_BLECmdCmp[0],  12);
  //  	ble_delayus(100);
  //  	FACEID_UARTStringSend(&g_BLECmdCmp[12], g_BLErxIndex-12);

    	//FACEID_UARTStringSend(g_UserTestBuf, 804);
    	USART_WriteBlocking(FACEID_UART, g_UserTestBuf, 804);
#endif
    	//__enable_irq();
    	return BLEFACERREG;
    }

    g_BLECmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+USERDEL");
    if(g_BLECmdCmp != NULL)
    {
        APP_FACEID_WAKEUP();

        g_FpEnrollInterrupted = true;

        sscanf((void *)g_BLECmdCmp, "AT+USERDEL=%d\r\n", &g_BLEFaceIDdeletNum);

        if(g_UserCfg.vizn_id[g_BLEFaceIDdeletNum] <= 50)
        {
            APP_FACEID_Printf("AT+FACEDEL=%d\r\n", g_UserCfg.vizn_id[g_BLEFaceIDdeletNum]);
        }

        if(APP_SYS_UserNumDelete(g_BLEFaceIDdeletNum) != 0xFFFFFFFF)
        {
        	APP_BLE_Printf("AT+USERDEL=SUCCESS\r\n");
            return BLEUSERDELOK;
        }
        else
        {
        	APP_BLE_Printf("AT+USERDEL=FAILED\r\n");
            return BLEUSERDELFAILED;
        }

    	return BLEFACEDEL;
    }

    g_BLECmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+GETUSERNO");
    if(g_BLECmdCmp != NULL)
    {
    	g_USERNum = 0;
    	g_USERNum = APP_SYS_UserNumGet();
    	PRINTF("AT+GETUSERNO=%d\r\n", g_USERNum);
    	APP_BLE_Printf("AT+GETUSERNO=%d\r\n", g_USERNum);
    	return BLEUSERGETNO;
    }

    g_BLECmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+GETINFO");
    if(g_BLECmdCmp != NULL)
    {
    	memset((void *)g_UserNameList, 0x00, 2048);
    	g_BLETransferLen = APP_SYS_UserInfoGet(g_UserNameList);

    	PRINTF("AT+GETINFO=");
    	USART_WriteBlocking(DEBUG_UART, (const uint8_t *)g_UserNameList, g_BLETransferLen);

    	APP_BLE_Printf("AT+GETINFO=");
    	USART_WriteBlocking(BLE_UART, (const uint8_t *)g_UserNameList, g_BLETransferLen);

    	APP_BLE_Printf("\r\n");
    	return BLEUSERGETINFO;
    }

    g_BLECmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+UPDTUSER=");
    if(g_BLECmdCmp != NULL)
    {
    	g_UpdateUSERNum = 0;
    	memset(g_UpdateUSERName, 0, 32);
    	sscanf((void *)g_BLECmdCmp, "AT+UPDTUSER=%d,%s", &g_UpdateUSERNum, g_UpdateUSERName);
        g_UpdateUSERNameLen = strlen((void *)g_UpdateUSERName);

        if(APP_SYS_UserNameUpdate(g_UpdateUSERNum, g_UpdateUSERNameLen, g_UpdateUSERName) != 0xFFFFFFFF)
        {
        	APP_BLE_Printf("AT+UPDTUSER=OK\r\n");
        }
        else
        {
        	APP_BLE_Printf("AT+UPDTUSER=DUPLICATE\r\n");
        }

    	return BLEUSERUPDATEUSER;
    }

    g_BLECmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+UPDTUSERPASS=");
    if(g_BLECmdCmp != NULL)
    {
    	g_UpdateUSERNum = 0;
    	g_BLEPwdLen = 0;
    	memset((uint8_t *)g_BLETempbuf, 0x00, 32);

    	sscanf((void *)g_BLECmdCmp, "AT+UPDTUSERPASS=%d,%s\r\n", &g_UpdateUSERNum, g_BLETempbuf);
        g_BLEPwdLen = strlen((void *)g_BLETempbuf);
        PRINTF("UPDATE PASSWORD %d, len %d, %s\r\n", g_BLEPwdNum, g_BLEPwdLen, g_BLETempbuf);

    	PRINTF("AT+UPDTUSERPASS=%d\r\n", g_UpdateUSERNum);

        if(APP_SYS_UserPWDUpdate(g_UpdateUSERNum, g_BLEPwdLen, g_BLETempbuf) != 0xFFFFFFFF)
        {
        	APP_BLE_Printf("AT+UPDTUSERPASS=OK\r\n"); // TODO : should be in task
            APP_MP3_Play(6);
            return BLESETPWDOK;
        }
        else
        {
        	APP_BLE_Printf("AT+UPDTUSERPASS=FAILED\r\n");
            return BLESETPWDFAILED;
        }
    }

    g_BLECmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+AUTH");
    if(g_BLECmdCmp != NULL)
    {
    	g_BLEPwdLen = 0;
    	memset((uint8_t *)g_BLETempbuf, 0x00, 32);
    	sscanf((void *)g_BLECmdCmp, "AT+UPDTUSERPASS=%s\r\n", g_BLETempbuf);
        g_BLEPwdLen = strlen((void *)g_BLETempbuf);
        PRINTF("AUTH PASSWORD len %d, %s\r\n", g_BLEPwdLen, g_BLETempbuf);

		APP_BLE_Printf("AT+AUTH=OK\r\n");
		APP_MP3_Play(6);
		return BLEUSERAUTH;
    }

    g_BLECmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"AT+CHGPASS");
    if(g_BLECmdCmp != NULL)
    {
    	g_BLEPwdLen = 0;
    	memset((uint8_t *)g_BLETempbuf, 0x00, 32);
    	sscanf((void *)g_BLECmdCmp, "AT+CHGPASS=%s\r\n", g_BLETempbuf);
        g_BLEPwdLen = strlen((void *)g_BLETempbuf);
        PRINTF("CHGPASS PASSWORD len %d, %s\r\n", g_BLEPwdLen, g_BLETempbuf);

		APP_BLE_Printf("AT+CHGPASS=OK\r\n");
		APP_MP3_Play(6);
		return BLEUSERCHGPASS;
    }

    g_BLECmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"SETPWD");
    if(g_BLECmdCmp != NULL)
    {
        g_BLEPwdLen = 0;
        //g_BLEPwdNum = (g_BLECmdCmp[7]-0x30)*10 + (g_BLECmdCmp[8]-0x30);
        memset((uint8_t *)g_BLETempbuf, 0x00, 32);
        sscanf((void *)g_BLECmdCmp, "SETPWD=%d.%s", &g_BLEPwdNum, g_BLETempbuf);
        g_BLEPwdLen = strlen((void *)g_BLETempbuf);
        PRINTF("SET PASSWORD %d, len %d, %s\r\n", g_BLEPwdNum, g_BLEPwdLen, g_BLETempbuf);
        if(g_BLEPwdNum<50)
        {
            *ret = g_BLEPwdNum;
            APP_SYS_PWDSet(g_BLEPwdNum, g_BLEPwdLen, (uint8_t *)g_BLETempbuf);
            APP_MP3_Play(6);
            return BLESETPWDOK;
        }
        else
        {
            return BLESETPWDFAILED;
        }
    }

    g_BLECmdCmp = (volatile uint8_t *)strstr((const char *)buf, (const char *)"FPENROLL");
    if(g_BLECmdCmp != NULL)
    {
    	APP_MP3_Play(9);
        //prince_encrypt(0);
        sscanf((void *)g_BLECmdCmp, "FPENROLL=%d", &g_FingerEnrollNum);
        PRINTF("FPENROLL %d\r\n", g_FingerEnrollNum);

        FPS_WorkModeSet(FPS_ENROLLMENT_MODE);

        *ret = g_FingerEnrollNum;
        return BLEFPENROLL;
    }

    g_BLECmdCmp = (volatile uint8_t *)strstr( (const char *)buf, (const char *)"REGCARD" );
    if(g_BLECmdCmp != NULL)
    {
        sscanf((void *)g_BLECmdCmp, "REGCARD=%d", &g_NFCCardNum);
        PRINTF("REGCARD %d\r\n", g_NFCCardNum);
        APP_MP3_Play(3);
        if(g_NFCCardNum < 50)
        {
            *ret = g_NFCCardNum;
            g_NFCCardEnrollMode = 0x80 + g_NFCCardNum;
            return BLEREGCARDOK;
        }
        else
        {
            *ret = g_NFCCardNum;
            return BLEREGCARDFAILED;
        }
        return BLEREGCARDFAILED;
    }

    return BLETASKIDLE;
}

volatile uint8_t g_PUFKEY[16];
uint32_t APP_BLE_Task(void)
{
    uint32_t ret, value;
    if(g_BLECmdStatus == 1)
    {
        ret = ble_task((uint8_t *)g_BLERecvBuf, &value);
        /* lock / un-lock */
        if(ret == BLEUNLOCK)
        {
            PRINTF("*** BLE unlock the door\r\n");
            //mp3_play(16);
        }
        if(ret == BLELOCK)
        {
            PRINTF("*** BLE lock the door\r\n");
        }
        if(ret == BLESETPWDOK)
        {
            PRINTF("!!! BLE Success Set Password Num %d \r\n", value);
            //mp3_play(6);
        }
        if(ret == BLESETPWDFAILED)
        {
            PRINTF("!!! BLE Failed Set Password Num %d \r\n", value);
        }
        /* reg CARD */
        /* AT+REGCARD=0\r\n */
        if(ret == BLEREGCARDOK)
        {
            PRINTF("!!! NFC Reg Card Num %d OK\r\n", value);
        }
        if(ret == BLEREGCARDFAILED)
        {
            PRINTF("!!! NFC Reg Card Num %d Failed\r\n", value);
        }
        /* format system */
        /* FORMAT\r\n */
        if(ret == BLESYSFORMAT)
        {
            PRINTF("!!! System Config Format Done\r\n");
        }
        /* Get RTC date/time value */
        /* GETRTC\r\n */
        if(ret == BLEGETRTC)
        {
            rtc_datetime_t date;
            RTC_GetDatetime(RTC, &date);           /* Get RTC values from IP */
            PRINTF("!!! Get RTC is %4d-%2d-%2d  %2d:%2d:%2d\r\n", date.year, date.month, date.day, date.hour, date.minute, date.second );
        }
        if(ret == BLESETRTC)
        {
            rtc_datetime_t date;
            RTC_GetDatetime(RTC, &date);           /* Set RTC values from IP */
            PRINTF("!!! Set RTC is %4d-%2d-%2d  %2d:%2d:%2d\r\n", date.year, date.month, date.day, date.hour, date.minute, date.second );
        }
        blestatus_clean();
        g_BLECmdStatus = 0;    
        g_BLErxIndex   = 0;     
        return ret;
    }
    
    /* Task Return Idle Status */
    return BLETASKIDLE;
}
