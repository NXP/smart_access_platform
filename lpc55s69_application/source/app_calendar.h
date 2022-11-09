/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __APP_CALENDAR_H__
#define __APP_CALENDAR_H__

#include "fsl_rtc.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define RTCTASKDEBUG  0x01                 /* Debug RTC task function */

extern void           APP_RTC_Init(void);
extern void           APP_RTC_Set (rtc_datetime_t data);
extern rtc_datetime_t APP_RTC_Read(void);
extern void           APP_RTC_Task(void);


#endif /* __APP_CALENDAR_H__ */
