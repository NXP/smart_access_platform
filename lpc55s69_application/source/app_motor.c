/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "stdint.h"
#include "fsl_gpio.h"
#include <stdbool.h>

#include "app_motor.h"

#include "app_syscfg.h"

#include "app_printf.h"

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
 * @brief   MOTOR Delay
 * @param   times : delay times / mS
 * @return  NULL
 */
volatile uint32_t g_MotorCnt1, g_MotorCnt2;
void motor_delayms(uint32_t times)
{
    for (g_MotorCnt1 = 0; g_MotorCnt1 < times; g_MotorCnt1++)
    {
        for (g_MotorCnt2 = 0; g_MotorCnt2 < (11000); g_MotorCnt2++)
        {
            ;
        }
    }
}

/**
 * @brief   MOTOR Initialize
 * @param   NULL
 * @return  NULL
 */
void APP_MOTOR_Init(void)
{
    gpio_pin_config_t   gpioPinConfig;
    
    /* Enables the clock for the I/O controller.: Enable Clock. */
    CLOCK_EnableClock(kCLOCK_Iocon);
    /* Debug Pin Port Init */
    IOCON->PIO[MOTORA_PORT][MOTORA_PIN]       = (MOTORA_FUNC    | MOTORA_PINCFG  );
    IOCON->PIO[MOTORB_PORT][MOTORB_PIN]       = (MOTORB_FUNC    | MOTORB_PINCFG );
    IOCON->PIO[MOTOSLEEP_PORT][MOTOSLEEP_PIN] = (MOTOSLEEP_FUNC | MOTOSLEEP_PINCFG );
    /* Disables the clock for the I/O controller.: Disable Clock. To Save Power */
    CLOCK_DisableClock(kCLOCK_Iocon);
    
    gpioPinConfig.pinDirection = kGPIO_DigitalOutput;
    gpioPinConfig.outputLogic  = 0u; /* output low as default. */
    /* Set as Low for low power, 0.08uA */
    GPIO_PinInit(GPIO, MOTORA_PORT,    MOTORA_PIN,    &gpioPinConfig);
    GPIO_PinInit(GPIO, MOTORB_PORT,    MOTORB_PIN,    &gpioPinConfig);
    GPIO_PinInit(GPIO, MOTOSLEEP_PORT, MOTOSLEEP_PIN, &gpioPinConfig);
    
    GPIO_PinWrite(GPIO, MOTORA_PORT,    MOTORA_PIN,    0u);
    GPIO_PinWrite(GPIO, MOTORB_PORT,    MOTORB_PIN,    0u);
    GPIO_PinWrite(GPIO, MOTOSLEEP_PORT, MOTOSLEEP_PIN, 0u);
}

/**
 * @brief   Motro status set
 * @param   status / 1 - un-lock, 2 - lock, others - no activities
 * @return  NULL
 */
void APP_MOTOR_Set(uint8_t status)
{
    if(status == 1) {
    	GPIO_PinWrite(GPIO, MOTOSLEEP_PORT, MOTOSLEEP_PIN, 1u);
        GPIO_PinWrite(GPIO, MOTORA_PORT, MOTORA_PIN, 1u);
        GPIO_PinWrite(GPIO, MOTORB_PORT, MOTORB_PIN, 0u);
    }
    else if(status == 2) {
    	GPIO_PinWrite(GPIO, MOTOSLEEP_PORT, MOTOSLEEP_PIN, 1u);
        GPIO_PinWrite(GPIO, MOTORA_PORT, MOTORA_PIN, 0u);
        GPIO_PinWrite(GPIO, MOTORB_PORT, MOTORB_PIN, 1u);
    }
    else if(status == 3) {
    	GPIO_PinWrite(GPIO, MOTOSLEEP_PORT, MOTOSLEEP_PIN, 0u);
        GPIO_PinWrite(GPIO, MOTORA_PORT, MOTORA_PIN, 1u);
        GPIO_PinWrite(GPIO, MOTORB_PORT, MOTORB_PIN, 1u);
    }
    else {
    	GPIO_PinWrite(GPIO, MOTOSLEEP_PORT, MOTOSLEEP_PIN, 0u);
        GPIO_PinWrite(GPIO, MOTORA_PORT, MOTORA_PIN, 0u);
        GPIO_PinWrite(GPIO, MOTORB_PORT, MOTORB_PIN, 0u);
    }
}

/**
 * @brief   ELOCK MOTOR Operation
 * @param   request: refer motor_request_t
 * @return  NULL
 */
void APP_MOTOR_Task(motor_request_t request)
{
    switch (request)
    {
        case MOTOR_UNLOCK:
        {
            APP_MOTOR_Set(g_UserCfg.locktype == 0 ? 2 : 1);
            motor_delayms(300);
            APP_MOTOR_Set(4);
            break;
        }
        case MOTOR_LOCK:
        {
            APP_MOTOR_Set(g_UserCfg.locktype == 0 ? 1 : 2);
            motor_delayms(300);
            APP_MOTOR_Set(4);
            break;
        }
        case MOTOR_UNLOCK_AND_LOCK:
        {
            APP_MOTOR_Set(g_UserCfg.locktype == 0 ? 2 : 1);
            motor_delayms(300);
            APP_MOTOR_Set(4);
            motor_delayms(1000);
            APP_MOTOR_Set(g_UserCfg.locktype == 0 ? 1 : 2);
            motor_delayms(300);
            APP_MOTOR_Set(4);
            break;
        }
        default:
        {
            break;
        }
    }
}
