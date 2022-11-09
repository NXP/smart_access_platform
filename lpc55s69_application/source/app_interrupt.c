/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <app.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t g_SysTicks   = 0;
volatile uint32_t g_AliveTicks = 0;

volatile uint32_t g_WakeupFlag = 0;
volatile uint32_t g_NFCReadCnt = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * @brief   
 * @param   
 * @return  
 */
void SysTick_Handler(void)
{
    g_SysTicks++;
    /* System Alive Countdown */
    if(g_AliveTicks != 0)  g_AliveTicks--;
    
	if(g_LEDTicks != 0)    g_LEDTicks--;                        /* Tick for LED Task */
    else                   APP_LED_Task();

//    if( (g_SysTicks % OSINTERVAL) == 1)
//    {
//        APP_LED_TestTask();
//    }

	if(g_NFCReadCnt!=0) g_NFCReadCnt--;

	CAPT_KL16ZTickHandler();
    APP_PIR_TickHandler();
    PRINT_TickHandler();
  //  APP_PIR_IntHandler();
}

/**
 * @brief   
 * @param   
 * @return  
 */
extern volatile uint32_t g_EnterLowPowerFlag;
#if 0
void GINT1_IRQHandler(void)
{
    PRINTF("\r\123\r\n");
    if(g_EnterLowPowerFlag == 1)
    {
   // BOARD_BootClockFRO12M();
    
    PRINT_UARTInit(115200);
    /* Print the initial banner from Primary core */
    PRINTF("\r\nWakeup 123\r\n");
        g_EnterLowPowerFlag = 0;
    }
    CAPT_KL16ZTickHandler(); /* Captouch interrupt handler */
    //APP_NFC_TaskEnable();
    /* Clear interrupt before callback */
    GINT1->CTRL |= GINT_CTRL_INT_MASK;
}
#endif

extern volatile uint8_t g_SystemStatus;
void gint1_callback(void)
{
	g_WakeupFlag = 1;
	g_AliveTicks = SLEEPTIME*OSINTERVAL;            /* System Active 10 seconds */
}

void gint0_callback(void)
{
	if(g_SystemStatus == 2)
    {
		g_SystemStatus = 1;
		g_WakeupFlag = 1;
		APP_NFC_TaskEnable();
		g_AliveTicks = SLEEPTIME*OSINTERVAL;            /* System Active 10 seconds */
    }

	if(g_NFCReadCnt == 0)  APP_NFC_TaskEnable();

	if(GPIO_PinRead(GPIO, CAPTIRQ_PORT,  CAPTIRQ_PIN) == 0)
	{
		CAPT_KL16ZIntHandler(); /* Captouch interrupt handler */
	}

	APP_PIR_IntHandler();
}


/**
 * @brief   
 * @param   
 * @return  
 */
void APP_PinInterruptInit(void)
{
    GINT_Init(GINT0);
// CAPTOUCH
    GINT_SetCtrl(GINT0, kGINT_CombineOr, kGINT_TrigEdge, gint0_callback);                /* Setup GINT1 for edge trigger, "OR" mode */
#if 1
    GINT_ConfigPins( GINT0, kGINT_Port1, ~((1<<27)|(1<<18)), ((1<<27)|(1<<18)) );        /* Select pins & polarity for GINT1 */
#else
    GINT_ConfigPins( GINT0, kGINT_Port1, ~((1<<27)), ((1<<27)) );        /* Select pins & polarity for GINT1 */
#endif
//    GINT_ConfigPins( GINT0, kGINT_Port1, ~((1<<27)), ((1<<27)) );
    GINT_EnableCallback(GINT0);

    NVIC_SetPriority(GINT0_IRQn, 100);
// Fingerprint
    GINT_SetCtrl(GINT1, kGINT_CombineOr, kGINT_TrigEdge, gint1_callback);  /* Setup GINT1 for edge trigger, "OR" mode */
    GINT_ConfigPins( GINT1, kGINT_Port1, (1<<17) , (1<<17) );             /* Select pins & polarity for GINT1 */
    GINT_EnableCallback(GINT1);                                            /* Enable callbacks for GINT1 */
    NVIC_SetPriority(GINT1_IRQn, 101);
}
