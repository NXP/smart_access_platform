/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __APP_BLE_H__
#define __APP_BLE_H__

#include "stdint.h"

/*******************************************************************************
 * Instructions
 ******************************************************************************/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BLE_BUFFER_SIZE       4096                                                     /* BLE Uart Buffer Length */

#define BLETASKIDLE           0x00000000                                               /* BLE Task is IDEL */

#define BLEUNLOCK             0x00000001                                               /* BLE Command is Unlock the door */
#define BLELOCK               0x00000002                                               /* BLE Command is Lock the door */
#define BLESETPWD             0x00000003                                               /* BLE Command is set user password */
#define BLEFPENROLL           0x00000004                                               /* BLE Command is enroll a finger print */
#define BLEREGCARD            0x00000005                                               /* BLE Command is enroll a Card */

#define BLESETPWDOK           0x00000006
#define BLESETPWDFAILED       0x00000007
#define BLEREGCARDOK          0x00000008
#define BLEREGCARDFAILED      0x00000009

#define BLESYSFORMAT          0x0000000A
#define BLEGETRTC             0x0000000B
#define BLESETRTC             0x0000000C
#define BLEPUFENROLL          0x0000000D
#define BLEPUFKEYGEN          0x0000000E
#define BLEPUFKEYGET          0x0000000F

#define BLEUNLOCKPASS         0x00000010                                              /* BLE Command is Unlock the door by password */
#define BLEUNLOCKFAILED       0x00000020

#define BLEFACEREG            0x00000100                                              /* BLE REG  ID */
#define BLEFACERREG           0x00000200                                              /* BLE RREG ID */
#define BLEFACEDREG           0x00000300                                              /* BLE destroy ID */
#define BLEFACEDEL            0x00000400                                              /* BLE delete ID */
#define BLEFACEACTIVE         0x00000500                                              /* BLE active faceid*/

#define BLEUSERGETNO          0x00010000
#define BLEUSERGETINFO        0x00020000
#define BLEUSERUPDATEUSER     0x00030000
#define BLEUSERUPDATEUSERPWD  0x00040000
#define BLEUSERAUTH           0x00050000
#define BLEUSERCHGPASS        0x00060000
#define BLEUSERDELOK          0x00070000
#define BLEUSERDELFAILED      0x00080000

#define BLEMATTERLOCK         0x01000000
#define BLEMATTERUNLOCK       0x02000000
#define BLEMATTEREXTLOCK      0x04000000
#define BLEMATTEREXTUNLOCK    0x05000000
#define BLEMATTERRESET        0x06000000

/* BLE QN9090 Pins */
#define BLE_UART                   USART0
#define BLE_UART_RST               kFC0_RST_SHIFT_RSTn
#define BLE_UART_CLKATTACH         kFRO12M_to_FLEXCOMM0
#define BLE_UART_CLKFREQ           12000000UL
#define BLE_UART_CLKSRC            kCLOCK_Flexcomm0
#define BLE_UART_IRQNUM            FLEXCOMM0_IRQn
#define BLE_UART_IRQHANDLER        FLEXCOMM0_IRQHandler

#define BLETXD_PORT                0u
#define BLETXD_PIN                 30u
#define BLETXD_FUNC                IOCON_FUNC1

#define BLERXD_PORT                0u
#define BLERXD_PIN                 29u
#define BLERXD_FUNC                IOCON_FUNC1

#define BLEIRQI_PORT               1u
#define BLEIRQI_PIN                5u
#define BLEIRQI_FUNC               IOCON_FUNC0
#define BLEPINTI_SRC               kINPUTMUX_GpioPort1Pin5ToPintsel

#define BLEIRQO_PORT               0u
#define BLEIRQO_PIN                31u
#define BLEIRQO_FUNC               IOCON_FUNC0
 
extern volatile uint32_t  g_BLETaskStatus;
 
extern void     APP_BLE_Init(void);
extern uint32_t APP_BLE_Task(void);

extern uint32_t APP_BLE_Printf(const char *formatString, ...);
extern volatile uint16_t  g_FingerDetectNum;
extern volatile uint16_t  g_FingerEnrollNum;

extern volatile uint8_t  g_UserIndexRecord;
extern volatile uint8_t  g_BLEFaceIDdeletNum;

#endif /* __APP_BLE_H__ */
