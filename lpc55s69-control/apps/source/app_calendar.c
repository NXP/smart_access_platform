/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_rtc.h"
#include "app_calendar.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

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
 * @brief   Init RTC
 * @param   NULL
 * @return  NULL
 */
void APP_RTC_Init(void)
{
    rtc_datetime_t date;
    
    /* Init RTC */
    CLOCK_EnableClock(kCLOCK_Rtc);                              /* Enable the RTC peripheral clock */

    if( RTC->COUNT == 0x00 )
    {
        RESET_PeripheralReset(kRTC_RST_SHIFT_RSTn);
        /* Make sure the reset bit is cleared */
        RTC->CTRL &= ~RTC_CTRL_SWRESET_MASK;
        /* Make sure the RTC OSC is powered up */
        RTC->CTRL &= ~RTC_CTRL_RTC_OSC_PD_MASK;
        
        /* Set a start date time and start RTC */
        date.year   = 2020U;
        date.month  = 6U;
        date.day    = 2U;
        date.hour   = 12U;
        date.minute = 0;
        date.second = 0;
        
        /* RTC time counter has to be stopped before setting the date & time in the TSR register */
        RTC_StopTimer(RTC);
        /* Set RTC time to default */
        RTC_SetDatetime(RTC, &date);
        /* Start the RTC time counter */
        RTC_StartTimer(RTC);
    }
}

/**
 * @brief   Set RTC value
 * @param   date - 
 * @return  NULL
 */
void APP_RTC_Set(rtc_datetime_t date)
{
    RTC_StopTimer(RTC);                                 /* RTC time counter has to be stopped before setting */
    RTC_SetDatetime(RTC, &date);                        /* Set RTC time to default */
    RTC_StartTimer(RTC);                                /* Start the RTC time counter */
}

/**
 * @brief   Read RTC value
 * @param   NULL
 * @return  date
 */
rtc_datetime_t APP_RTC_Read(void)
{
    rtc_datetime_t date;
    RTC_GetDatetime(RTC, &date);                        /* Get date time */
    return date;
}


/**
 * @brief   RTC Task
 * @param   NULL
 * @return  NULL
 */
void APP_RTC_Task(void)
{
    rtc_datetime_t date;
    RTC_GetDatetime(RTC, &date);                       /* Read RTC Value */
}
