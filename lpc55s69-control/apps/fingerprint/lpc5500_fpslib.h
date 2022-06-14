/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __LPC5500_FPSLIB_H__
#define __LPC5500_FPSLIB_H__

#include "stdint.h"

#include "app_printf.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define USE_FINGERPRINT 1

#define PRINTF  PRINT_UARTPrintf


typedef void     (*FPS_SensorInitFuncPtr)     (void);
typedef void     (*FPS_SensorResetFuncPtr)    (uint8_t status);
typedef void     (*FPS_SensorSPICSFuncPtr)    (uint8_t status); 
typedef int      (*FPS_SensorSPISendFuncPtr)  (uint8_t data);
typedef int      (*FPS_SensorSPIRecvFuncPtr)  (uint8_t *rx_buffer, const uint32_t num);
typedef void     (*FPS_SensorDelayFuncPtr)    (uint32_t times);
typedef uint32_t (*FPS_SensorGetAddrFuncPtr)  (void);

typedef struct
{
    FPS_SensorInitFuncPtr      sensor_init;
	FPS_SensorResetFuncPtr     sensor_reset;
	FPS_SensorSPICSFuncPtr     sensor_select;
	FPS_SensorSPISendFuncPtr   sensor_spisend;
	FPS_SensorSPIRecvFuncPtr   sensor_spirecv;
	FPS_SensorDelayFuncPtr     sensor_delayms;
	FPS_SensorDelayFuncPtr     sensor_delayus;
    FPS_SensorGetAddrFuncPtr   sensor_getaddress;
    uint32_t                   sensor_flashaddr;
    uint32_t                   sensor_flashsize;
}fingersensor_initfunc_t;

#define FPS_STORE_FINGERS              25u

#define FPS_IMAGEVALID                 0x01
#define FPS_IMAGEILLEGAL               0x02
#define FPS_IMAGEIDLE                  0x03

#define FPS_REGOK                      0xFE
#define FPS_REGFAILED                  0xFD
#define FPS_REGING                     0xFC
#define FPS_REGNOFINGER                0xFB
#define FPS_REGIDLE                    0xFA

#define FPS_IMAGEGETOK                 0x01
#define FPS_IMAGEGETNOBUF              0x02
#define FPS_IMAGEGETFAILED             0xFF


#define FPS_IDLE_MODE                  0x00 /* IDLE Mode */
#define FPS_ENROLLMENT_MODE            0x01 /* Enroll Mode */
#define FPS_VERIFY_MODE                0x02 /* Verify Mode */
#define FPS_GET_IMAGE_MODE             0x03 /* Get Image Data Mode */
#define FPS_FINGER_DETECT_MODE         0x04 /* Detect Finger Mode */

extern uint8_t  FPS_SensorInit(fingersensor_initfunc_t func);
extern uint32_t FPS_SensorIdGet(void);

extern void     FPS_IntHandler(void);  /* FPS_IntHandler,  GPIO interrupt Handler */
extern void     FPS_TickHandler(void); /* FPS_TickHandler, Should place to 1mS Timer handler */

extern void     FPS_TemplateDelete(uint8_t id, uint8_t num);
extern uint8_t  FPS_TemplateFindDelete(uint16_t id, uint16_t* finger_index);

extern void     FPS_WorkModeSet(uint8_t data);
extern uint8_t  FPS_WorkModeGet(void);
extern uint8_t  FPS_VerifyTask(uint16_t *finger_index, uint32_t *TimeCost);
extern uint8_t  FPS_RegistTask(uint16_t *finger_index, uint16_t *finger_times, uint16_t *finger_total);
extern uint8_t* FPS_GetImageTask(void);
extern uint8_t  FPS_LPC55FlashFormat(uint32_t addr, uint32_t sectors);

extern void     FPS_BTLGlobalInit(uint32_t SensorID, uint32_t ImageStorageAddr, uint32_t ImageStorageSize);

#endif // end file FPS_TemplateWrite
