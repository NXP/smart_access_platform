/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __APP_SYSCFG_H__
#define __APP_SYSCFG_H__

#include "stdint.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
// parameters
typedef struct
{
    uint8_t   valid[50];           /* 0xAA is valid, 0xFF is un-valid */
    uint8_t   index[50];           /* Valid user index */
    uint8_t   user_name[50][32];   /* User Name */
    uint8_t   vizn_id[50];         /* User ID, VIZN ID */
    uint8_t   card_info[50][32];   /* 50 cards */
    uint8_t   upwd_info[50][32];   /* 50 user pinpad password */
    uint8_t   fp_id[50];           /* 25 fingerprint template ID */
    uint32_t  functions;           /* 0 means enabled, 1 for disable  */
    uint8_t   locktype;
}
params_t;

extern params_t g_UserCfg;

extern void APP_SYS_InfoInit(uint8_t value);
extern void APP_SYS_InfoLoad(void);
extern void APP_SYS_InfoSave(void);
extern void APP_SYS_Format(uint8_t reset);
extern void APP_SYS_PWDSet(uint8_t num, uint32_t length, uint8_t *buf);

extern uint32_t APP_SYS_UserCreate(uint32_t length, uint8_t *buf);
extern void     APP_SYS_UserUpdate(uint32_t length, uint8_t *buf);
extern uint32_t APP_SYS_ValidUserFind(uint8_t *buf);
extern uint32_t APP_SYS_UserCompare(uint32_t length, uint8_t *buf);
extern uint32_t APP_SYS_UserNumGet(void);
extern uint32_t APP_SYS_UserInfoGet(uint8_t *buf);
extern uint32_t APP_SYS_UserNameUpdate(uint8_t index, uint32_t len, uint8_t *buf);
extern uint32_t APP_SYS_UserPWDUpdate(uint8_t index, uint8_t len, uint8_t *buf);
extern uint32_t APP_SYS_UserNumDelete(uint8_t index);
extern uint32_t APP_SYS_VIZNIDUpdate(uint8_t index, uint8_t vizn_id);
extern uint32_t APP_SYS_VIZNIDGet(uint8_t index, uint8_t *vizn_id);
extern uint32_t APP_SYS_FP_ID_Update(uint8_t index, uint8_t fp_id);
extern uint32_t APP_SYS_FP_ID_Delete(uint8_t index);
extern uint32_t APP_SYS_GetUserNameByIndex(uint8_t index, uint8_t *buf, uint32_t len);

#endif /* __APP_SYSCFG_H__ */
