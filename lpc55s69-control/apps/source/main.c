/*
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include "app.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint8_t g_SystemStatus = 0;                /* Record system status, 1 for active, 2 for sleep */

void HardFault_Handler(void)
{
	APP_SYS_Format(1);
	NVIC_SystemReset();
}

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{
	uint8_t i;
#if 1
	BOARD_BootClockFROHF96M();                      /* Initialize SystemClock to 96MHz */
#else
	BOARD_BootClockPLL150M();
#endif
	SystemCoreClockUpdate();                        /* Update SystemClock Value */
    CLOCK_EnableClock(kCLOCK_Gpio0);                /* GPIO 0 Pins. */
    CLOCK_EnableClock(kCLOCK_Gpio1);                /* GPIO 1 Pins. */

    PRINT_UARTInit(115200);                         /* Print the initial banner from Primary core */
    PRINTF("\r\n------------------------------------------------------\r\n");
    PRINTF("*** Hello World from Board %s Core0 !\r\n", BOARD_NAME);
    PRINTF("*** APP Core0 version %d.%d\r\n", APPCORE0_VERSION_MAIN, APPCORE0_VERSION_MINI);
	PRINTF("*** Build Date %s %s\r\n", __DATE__, __TIME__);

	APP_SYS_InfoLoad();                             /* Initialize configure parameters */
	if(g_UserCfg.locktype > 4) g_UserCfg.locktype = 0;

	spiflash_init();                                /* Initialize SPI flash with SPI interface */

    APP_LED_Init();                                 /* Initialize LED matrix */
	APP_RTC_Init();                                 /* Initialize RTC IP on LPC55S69 */
	APP_MOTOR_Init();                               /* Initialize Motor control */
	APP_NFC_Init();                                 /* Initialize RC663 */
	APP_MP3_Init();                                 /* Initialize Audio codec */

//	APP_MP3_Play(0);
#if 0
	APP_MP3_Play(32);
	for(int i=0; i<13; i++)
	{
		APP_MP3_Play(29+i);
		//SDK_DelayAtLeastUs(3000*1000U, 96000000);
	}
#endif

	/* On Smart Access platform, this MP3 init must after CAPT due to shared the same I2C */
	APP_CAPT_Init();                                /* Initialize Captouch */

    /* On Smart Access platform, this FPS init must after RC663 due to shared the same SPI */
    APP_FPS_Init();                                 /* Initialize finger print Sensor */

    APP_BLE_Init();                                 /* Initialize BLE Flexcomm as UART */
    APP_MATTER_Init();

    APP_FACEID_Init();                              /* Initialize RT117F FACEID Flexcomm as UART */
    APP_PIR_Init();                                 /* Initialize PIR sensor */

	APP_PinInterruptInit();                         /* Initialize interrupt handler */
	APP_NFC_TaskEnable();                           /* Enabled RC663 read card task */
    g_AliveTicks = SLEEPTIME*OSINTERVAL;            /* System Active 10 seconds */

    SystemCoreClockUpdate();                        /* Update System Clock value */
    SysTick_Config(SystemCoreClock / OSINTERVAL);   /* Configure SystemTick timer */
    g_SystemStatus = 1;                             /* System is Active */

    binupdate_init();

    while(1)
    {
    	APP_PRINTF_Task();

    	if(g_SystemStatus == 1)
    	{
    		//binupdate_task();                         /* Update mp3 file binary task */
			APP_RTC_Task();                             /* RTC         Calendar Task */
			APP_PIR_Task();                             /* PIR         Sensor   Task */

			if(g_nfcTaskEnable == 0)
			{
				g_CAPTaskStatus = APP_CAPT_Task();      /* KL16 Captouch Task */
			}

			g_NFCTaskStatus    = APP_NFC_Task();        /* RC663       Reader   Task */

			g_BLETaskStatus    = APP_BLE_Task();        /* QN9090      BLE      Task */
			g_MATTERTaskStatus = APP_MATTER_Task();     /* K32      MATTER      Task */

			g_FACEIDTaskStatus = APP_FACEID_Task();     /* RT117F      FACEID   Task */

			/* Fingerprint Task */
			if(FPS_WorkModeGet() == FPS_ENROLLMENT_MODE)  APP_FPS_Task(&g_FingerEnrollNum);
			if(FPS_WorkModeGet() == FPS_VERIFY_MODE)      g_FPSTaskStatus = APP_FPS_Task(&g_FingerDetectNum);

	        if(g_CAPTaskStatus == CAPTWRONGPWD)
	        {
	            APP_MP3_Play(8);
	            APP_LED_AllOff();
	            g_CAPTaskStatus = CAPTTASKIDLE;
	        }

	        if(g_NFCTaskStatus == NFCCARDILLEGAL)
	        {
	            APP_MP3_Play(1);
	            APP_LED_AllOff();
	            g_NFCTaskStatus = NFCCARDIDLE;
	        }

	        if(g_FPSTaskStatus == FPS_IMAGEILLEGAL)
	        {
	            APP_MP3_Play(15);
	            APP_LED_AllOff();
	            g_FPSTaskStatus = FPS_IMAGEIDLE;
	        }

			if(g_MATTERTaskStatus == MATTERUNLOCK)
			{
				if(g_UserCfg.locktype == 0) APP_MOTOR_Task(11);                              /* Operate lock system */
				if(g_UserCfg.locktype == 1) APP_MOTOR_Task(13);                              /* Operate lock system */
				g_MATTERTaskStatus = MATTERTASKIDLE;                  /* Clean up Status */
				g_AliveTicks = SLEEPTIME*OSINTERVAL;            /* System Active 10 seconds */
				APP_MP3_Play(32);
			}

			if(g_MATTERTaskStatus == MATTERLOCK)
			{
				if(g_UserCfg.locktype == 0) APP_MOTOR_Task(12);                              /* Operate lock system */
				if(g_UserCfg.locktype == 1) APP_MOTOR_Task(14);                              /* Operate lock system */
				g_MATTERTaskStatus = MATTERTASKIDLE;                  /* Clean up Status */
				g_AliveTicks = SLEEPTIME*OSINTERVAL;            /* System Active 10 seconds */
				APP_MP3_Play(33);
			}
			// TODO : AT+EXTLOCK=\r\n
			// TODO : AT+EXTUNLOCK=\r\n
			if( (g_BLETaskStatus == BLEUNLOCK)    ||
				(g_NFCTaskStatus == NFCCARDVALID) ||
				(g_CAPTaskStatus == CAPTVALIDPWD) ||
				(g_FACEIDTaskStatus == FACEIDUNLOCK) ||
				(g_FPSTaskStatus == FPS_IMAGEVALID)  )
			{
				APP_MP3_Play(0);

				if(g_UserCfg.locktype == 0) APP_MOTOR_Task(1);       /* Operate lock system */
				if(g_UserCfg.locktype == 1) APP_MOTOR_Task(2);
				g_BLETaskStatus = BLETASKIDLE;                       /* Clean up Status */
				g_NFCTaskStatus = NFCCARDIDLE;
				g_CAPTaskStatus = CAPTTASKIDLE;
				g_FPSTaskStatus = FPS_IMAGEIDLE;
				g_FACEIDTaskStatus = FACEIDIDLE;
				// TODO : APP_MP3_Wait();
				g_AliveTicks = 0;
			}
    	}
    	if(g_AliveTicks == 0)
    	{
    //		APP_LED_AllOff();                                    /* turn off all LEDs */
    //		g_SystemStatus = 2;
    	}
    	//g_AliveTicks = SLEEPTIME*OSINTERVAL;
#if 0
        /* LOW POWER */
        if(g_AliveTicks == 0)
        {
        	if(g_SystemStatus == 1)                                  /* previous is active mode */
        	{
        		g_SystemStatus = 2;                                  /* goto sleep mode */
        		APP_LED_AllOff();                                    /* turn off all LEDs */
        		btl_set_work_mode(BTL_FINGER_DETECT_MODE);           /* Set finger sensor to detect mode */
        		BOARD_BootClockFRO12M();
        		SystemCoreClockUpdate();                             /* Update SystemClock Value */
        		g_WakeupFlag = 0;

        	}
        	if(g_SystemStatus == 2)                                  /* previous is sleep mode then wakeup*/
        	{
        		APP_LED_AllOff();                                    /* turn off all LEDs */
        		if(g_WakeupFlag == 1)
        		{
        			g_WakeupFlag = 0;                                /* Clear wakeup flag */
					BOARD_BootClockFROHF96M();                       /* Initialize SystemClock to 96MHz */
					SystemCoreClockUpdate();                         /* Update SystemClock Value */
        			btl_set_work_mode(BTL_VERIFY_MODE);              /* Set finger sensor to verify mode */
					g_AliveTicks = SLEEPTIME*OSINTERVAL;             /* System Active 10 seconds */
					g_SystemStatus  = 1;                             /* Wakeup */
        		}
        	}
        }
#endif
    }
}
