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

// LPC55xx Chip peripherals & Boards includes
#include "fsl_common.h"
#include "fsl_clock.h"
#include "fsl_gpio.h"
#include "fsl_iocon.h"

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
static uint8_t   s_PIRSensorStatus = 0;
static uint32_t  s_PIRSensorStatusTimeCnt = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/
/**
 * @brief   PIR Initialize
 * @param   NULL
 * @return  NULL
 */
void APP_PIR_Init(void)
{
    /* Enables the clock for the I/O controller.: Enable Clock. */
    CLOCK_EnableClock(kCLOCK_Iocon);
    /* PIR Sensor Pin Port Init */
    IOCON->PIO[PIRSENSOR_PORT][PIRSENSOR_PIN] = (PIRSENSOR_FUNC | PIRSENSOR_PINCFG  );
    /* Disables the clock for the I/O controller.: Disable Clock. To Save Power */
    CLOCK_DisableClock(kCLOCK_Iocon);
    
	// PIR Sensor GPIO init as input mode
    gpio_pin_config_t   gpioPinConfig;
    gpioPinConfig.pinDirection = kGPIO_DigitalInput;
    gpioPinConfig.outputLogic  = 1u;
    
    GPIO_PinInit (GPIO, PIRSENSOR_PORT,  PIRSENSOR_PIN,  &gpioPinConfig);

    s_PIRSensorStatus = 0;
}

/**
 * @brief   PIR Task
 * @param   NULL
 * @return  NULL
 */
void APP_PIR_Task(void)
{
	/* PIR sensor timeout */
	if( (s_PIRSensorStatusTimeCnt == 0) && (s_PIRSensorStatus != 0))
	{
		s_PIRSensorStatus = 0;
	}
}

/**
 * @brief   APP_PIR_StatusRead
 * @param   *flag, 0-no human body, 1-YES, human nearby
 * @return  NULL
 */
uint8_t APP_PIR_StatusRead(void)
{
	return s_PIRSensorStatus;
}

/**
 * @brief   APP_PIR_IntHandler
 * @param   NULL
 * @return  NULL
 */
void APP_PIR_IntHandler(void)
{
	if(GPIO_PinRead(GPIO, PIRSENSOR_PORT,  PIRSENSOR_PIN) == 0)
	{
		APP_FACEID_WAKEUP();
		s_PIRSensorStatus = PIR_HUMAN_DETECTED;
		s_PIRSensorStatusTimeCnt = OSINTERVAL*10;
	}
}

/**
 * @brief   APP_PIR_TickHandler
 * @param   NULL
 * @return  NULL
 */
void APP_PIR_TickHandler(void)
{
	if(s_PIRSensorStatusTimeCnt != 0) s_PIRSensorStatusTimeCnt--;
}
