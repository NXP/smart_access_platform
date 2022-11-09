/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "app.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
params_t g_UserCfg;    /* User Application Data Information */

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/**
 * @brief   Initialize System Configure Parameter
 * @param   NULL
 * @return  NULL
 */
void APP_SYS_InfoInit(uint8_t value)
{
    memcpy((void *)&g_UserCfg, (uint8_t *)(USER_CFGADDR), sizeof(params_t));    /* Init user configure data */
}


/**
 * @brief   
 * @param   
 * @return  
 */
void APP_SYS_InfoSave(void)
{
    flash_config_t flashInstance;
    uint32_t status;
    uint8_t *TempBuf = NULL;
    
    PRINTF("*** APP_SYS_InfoSave FLASH_Init ing\r\n");
    if (FLASH_Init(&flashInstance) != kStatus_Success)
    {
        PRINTF("!!! APP_SYS_InfoSave FLASH_Init Failed!\r\n");
        return;
    }
    __disable_irq();

    status = FLASH_Erase(&flashInstance, USER_CFGADDR, USER_CFGSIZE, kFLASH_ApiEraseKey);
    status = FLASH_VerifyErase(&flashInstance, USER_CFGADDR, USER_CFGSIZE);
    status = FLASH_Program(&flashInstance, USER_CFGADDR, (const void *)&g_UserCfg, USER_CFGSIZE);

    /* Verify programming by reading back from flash directly */
    for (uint32_t i = 0; i < (USER_CFGSIZE/4); i++)
    {
        status = *(volatile uint32_t *)(USER_CFGADDR + i * 4);
    }

    __enable_irq();
}

/**
 * @brief   
 * @param   
 * @return  
 */
void APP_SYS_InfoLoad(void)
{
    memcpy((void *)&g_UserCfg, (uint8_t *)(USER_CFGADDR), sizeof(params_t));    /* load system configure information */
}


/**
 * @brief   
 * @param   
 * @return  
 */
void APP_SYS_PWDSet(uint8_t num, uint32_t length, uint8_t *buf)
{
    uint32_t i;
    PRINTF("*** Config User Password %d in Flash, Len %d, PWD is ", num, length);
    memset(g_UserCfg.upwd_info[num], 0x00, 32);
    g_UserCfg.upwd_info[num][0] = 0xAA;
    g_UserCfg.upwd_info[num][1] = length;
    for(i=0; i<length; i++) {
        g_UserCfg.upwd_info[num][2+i] = buf[i];
        PRINTF("%x ", g_UserCfg.upwd_info[num][2+i]);
    }
    APP_SYS_InfoSave();
    PRINTF("*** Set Password Done\r\n");
}

/**
 * @brief   
 * @param   
 * @return  
 */
uint32_t APP_SYS_UserCreate(uint32_t length, uint8_t *buf)
{
    uint32_t i;
    uint32_t num;
    uint32_t user_index;

    num = APP_SYS_ValidUserFind(buf);
    if (num == 0xFFFFFFFF) /* exist user */
    {
    	return 0xFFFFFFFF;
    }
    else                  /* Create a new one */
    {
        memset(g_UserCfg.user_name[num], 0x00, 32);
        memset(g_UserCfg.card_info[num], 0x00, 32);
        memset(g_UserCfg.upwd_info[num], 0x00, 32);
        g_UserCfg.vizn_id[num] = 0xFF;
        g_UserCfg.fp_id[num] = 0xFF;

        g_UserCfg.valid[num] = 0xAA;
        memcpy(g_UserCfg.user_name[num], buf, length);
    }

    user_index = 0;
    for (i = 0; i < 50; i++)
    {
        if (g_UserCfg.valid[i] == 0xAA)
        {
            g_UserCfg.index[i] = user_index;
            user_index++;
        }
    }

    PRINTF("*** Config User Name %d in Flash, Len %d, PWD is ", num, length);
    APP_SYS_InfoSave();
    PRINTF("*** Create New User Done, User Cnt %d\r\n", user_index);
    return num;
}

/**
 * @brief
 * @param
 * @return
 */
void APP_SYS_UserDelete(uint8_t *buf)
{
    uint32_t i, num, user_index;

    num = APP_SYS_ValidUserFind(buf);
    if(num < 50)
    {
		g_UserCfg.valid[num] = 0xFF;
		g_UserCfg.card_info[num][0] = 0xFF;
		g_UserCfg.upwd_info[num][0] = 0xFF;
    }

    user_index = 0;
    for(i=0; i<50; i++)
    {
    	if(g_UserCfg.valid[i] == 0xAA)
    	{
    		g_UserCfg.index[i] = user_index;
    		user_index++;
    	}
    }

    APP_SYS_InfoSave();
    PRINTF("*** Delete User Done\r\n");
}

/**
 * @brief
 * @param
 * @return
 */
int areEqual(uint8_t *buf, uint8_t *buf2, int n, int m)
{
    if (n != m)
        return 0;

    for (int i = 0; i < n; i++)
        if (buf[i] != buf2[i])
            return 0;

    return 1;
}

/**
 * @brief
 * @param name of the user
 * @return first valid id
 */
uint32_t APP_SYS_ValidUserFind(uint8_t *buf)
{
    uint32_t i;
    char *kResult = NULL;

    for(i=0; i<50; i++)
    {
    	if(g_UserCfg.valid[i] == 0xAA)
    	{
    		int n = strlen((void *)(const char *)buf);
    		int m = strlen((void *)(const char *)&g_UserCfg.user_name[i][0]);

    		if (areEqual((const char *)buf, (const char *)&g_UserCfg.user_name[i][0], n, m))
    			return 0xFFFFFFFF;
    	}
    }

    for(i=0; i<50; i++)
    {
    	if(g_UserCfg.valid[i] != 0xAA)
    	{
    		return i;
    	}
    }
}

/**
 * @brief
 * @param
 * @return
 */
uint32_t APP_SYS_UserNameUpdate(uint8_t index, uint32_t len, uint8_t *buf)
{
    uint32_t i, j;
    char *kResult = NULL;

    if (APP_SYS_ValidUserFind((const char *) buf) == 0xFFFFFFFF)
    	return 0xFFFFFFFF;

    for(i=0; i<50; i++)
    {
    	if(g_UserCfg.valid[i] == 0xAA)
    	{
            if(g_UserCfg.index[i] == index)
            {
            	memset(&g_UserCfg.user_name[i], 0x00, 32);
            	memcpy(&g_UserCfg.user_name[i], buf, len);
            	APP_SYS_InfoSave();
            	return index;
            }
    	}
    }
    return 0xFFFFFFFF;
}

/**
 * @brief
 * @param
 * @return
 */
uint32_t APP_SYS_GetUserNameByIndex(uint8_t index, uint8_t *buf, uint32_t len)
{
    uint32_t size = 0;

    for (uint8_t i = 0; i < 50; i++)
    {
        if (g_UserCfg.valid[i] == 0xAA)
        {
            if (g_UserCfg.index[i] == index)
            {
                size = snprintf(buf, len, "%s", g_UserCfg.user_name[i]);
                break;
            }
        }
    }

    return size;
}

/**
 * @brief
 * @param
 * @return
 */
uint32_t APP_SYS_FP_ID_Update(uint8_t index, uint8_t fp_id)
{
    uint32_t i;
    uint32_t j;

    for (i = 0; i < 50; i++)
    {
        if (g_UserCfg.valid[i] == 0xAA)
        {
            if (g_UserCfg.index[i] == index)
            {
                g_UserCfg.fp_id[i] = fp_id;
                APP_SYS_InfoSave();
                return index;
            }
        }
    }
    return 0xFFFFFFFF;
}

/**
 * @brief
 * @param
 * @return
 */
uint32_t APP_SYS_FP_ID_Delete(uint8_t index)
{
    uint32_t i;
    uint32_t j;

    for (i = 0; i < 50; i++)
    {
        if (g_UserCfg.valid[i] == 0xAA)
        {
            if (g_UserCfg.index[i] == index)
            {
                if (g_UserCfg.fp_id[i] != 0xFF)
                {
                    FPS_TemplateDelete(g_UserCfg.fp_id[i]);
                    g_UserCfg.fp_id[i] = 0xFF;
                    APP_SYS_InfoSave();
                }
                return index;
            }
        }
    }
    return 0xFFFFFFFF;
}

/**
 * @brief
 * @param
 * @return
 */
uint32_t APP_SYS_VIZNIDUpdate(uint8_t index, uint8_t vizn_id)
{
    for (uint8_t i = 0; i < 50; i++)
    {
        if (g_UserCfg.valid[i] == 0xAA)
        {
            if (g_UserCfg.index[i] == index)
            {
                g_UserCfg.vizn_id[i] = vizn_id;
                APP_SYS_InfoSave();
                return index;
            }
        }
    }

    return 0xFFFFFFFF;
}

/**
 * @brief
 * @param
 * @return
 */
uint32_t APP_SYS_VIZNIDGet(uint8_t index, uint8_t *vizn_id)
{
    for (uint8_t i = 0; i < 50; i++)
    {
        if (g_UserCfg.valid[i] == 0xAA)
        {
            if (g_UserCfg.index[i] == index)
            {
                *vizn_id = g_UserCfg.vizn_id[i];
                return index;
            }
        }
    }

    return 0xFFFFFFFF;
}

uint32_t APP_SYS_UserPWDUpdate(uint8_t index, uint8_t len, uint8_t *buf)
{
    uint32_t i, j;
    char *kResult = NULL;

    for(i=0; i<50; i++)
    {
    	if(g_UserCfg.valid[i] == 0xAA)
    	{
            if(g_UserCfg.index[i] == index)
            {
            	memset(&g_UserCfg.upwd_info[i], 0x00, 32);
            	g_UserCfg.upwd_info[i][0] = 0xAA;
            	g_UserCfg.upwd_info[i][1] = len;
            	memcpy(&g_UserCfg.upwd_info[i][2], buf, len);
            	APP_SYS_InfoSave();
            	return index;
            }
    	}
    }
    return 0xFFFFFFFF;
}

uint32_t APP_SYS_UserNumDelete(uint8_t index)
{
    uint32_t i, j, user_index;

    for (i = 0; i < 50; i++)
    {
        if (g_UserCfg.valid[i] == 0xAA)
        {
            if (g_UserCfg.index[i] == index)
            {
                g_UserCfg.valid[i] = 0xFF;
                g_UserCfg.index[i] = 0xFF;

                memset(&g_UserCfg.user_name[i], 0xFF, 32);
                g_UserCfg.vizn_id[i] = 0xFF;

                memset(&g_UserCfg.card_info[i], 0xFF, 32);
                memset(&g_UserCfg.upwd_info[i], 0xFF, 32);
                if (g_UserCfg.fp_id[i] != 0xFF)
                {
                    FPS_TemplateDelete(g_UserCfg.fp_id[i]);
                    g_UserCfg.fp_id[i] = 0xFF;
                }

                for (j = i; j < 50 - 1; j++)
                {
                    g_UserCfg.valid[j] = g_UserCfg.valid[j + 1];
                    memcpy(&g_UserCfg.user_name[j], &g_UserCfg.user_name[j + 1], 32);
                    g_UserCfg.vizn_id[j] = g_UserCfg.vizn_id[j + 1];
                    memcpy(&g_UserCfg.card_info[j], &g_UserCfg.card_info[j + 1], 32);
                    memcpy(&g_UserCfg.upwd_info[j], &g_UserCfg.upwd_info[j + 1], 32);
                    g_UserCfg.fp_id[j] = g_UserCfg.fp_id[j + 1];
                }

                g_UserCfg.valid[j] = 0xFF;
                memset(&g_UserCfg.user_name[j], 0xFF, 32);
                g_UserCfg.vizn_id[j] = 0xFF;
                memset(&g_UserCfg.card_info[j], 0xFF, 32);
                memset(&g_UserCfg.upwd_info[j], 0xFF, 32);
                g_UserCfg.fp_id[j] = 0xFF;

                user_index = 0;
                for (j = 0; j < 50; j++)
                {
                    if (g_UserCfg.valid[j] == 0xAA)
                    {
                        g_UserCfg.index[j] = user_index;
                        user_index++;
                    }
                }

                APP_SYS_InfoSave();

                return index;
            }
        }
    }

    return 0xFFFFFFFF;
}

uint32_t APP_SYS_UserNumGet(void)
{
    static uint8_t i;
    static uint8_t s_UserNums = 0;
    s_UserNums = 0;
    for(i=0; i<50; i++)
    {
    	if(g_UserCfg.valid[i] == 0xAA)
    	{
    		s_UserNums++;
    	}
    }
    return s_UserNums;
}


uint32_t APP_SYS_UserInfoGet(uint8_t *buf)
{
    static uint8_t i;
    static uint32_t s_BufLen = 0;

    __disable_irq();
    s_BufLen = 0;
    for(i=0; i<50; i++)
    {
    	if(g_UserCfg.valid[i] == 0xAA)
    	{
    	    sprintf(&buf[s_BufLen], "%04d%s", g_UserCfg.index[i], g_UserCfg.user_name[i]);
    	   // sprintf(&buf[s_BufLen+4+32], "%04d", g_UserCfg.vizn_id[i]);
    	   // s_BufLen += 40;
    	    s_BufLen += 36;
    	}
    }
    __enable_irq();
    return s_BufLen;
}

/**
 * @brief
 * @param
 * @return
 */
void APP_SYS_Format(uint8_t reset)
{
    flash_config_t flashInstance;
    uint32_t status;
    uint32_t i, j;

    PRINTF("*** APP_SYS_Format FLASH_Init ing\r\n");
    if (FLASH_Init(&flashInstance) != kStatus_Success)
    {
        PRINTF("!!! APP_SYS_Format FLASH_Init Failed!\r\n");
    }
    
    uint8_t *TempBuf = malloc(32768);                                       /* Create a template data buffer */
    if(TempBuf == NULL)														/* allocate buffer failed */
    {
        PRINTF("!!! APP_SYS_Format Malloc TempBuf Buffer Failed!\r\n");
        return;
    }
#if 0
    memset((uint8_t *)TempBuf, 0x55, 32768);                                /* init template data buffer */
#else
    memset((uint8_t *)TempBuf, 0xFF, 32768);
#endif
    for(i=0; i<(FPTM_FPTSIZE/32768); i++)
    {
        status = FLASH_Erase(&flashInstance, FPTM_FPTADDR+i*32768, 32768, kFLASH_ApiEraseKey);  
        status = FLASH_VerifyErase(&flashInstance, FPTM_FPTADDR+i*32768, 32768);
        status = FLASH_Program(&flashInstance, FPTM_FPTADDR+i*32768, TempBuf, 32768);
        /* Verify programming by reading back from flash directly */
        for (uint32_t j = 0; j < (32768/4); j++)
        {
            status = *(volatile uint32_t *)(FPTM_FPTADDR+i*32768 + j * 4);
        } 
    }
    
    uint32_t last_addr = FPTM_FPTADDR+i*32768;
    uint32_t last_len = FPTM_FPTSIZE%32768;
    if(last_len)
    {
    	status = FLASH_Erase(&flashInstance, last_addr, last_len, kFLASH_ApiEraseKey);
		status = FLASH_VerifyErase(&flashInstance, last_addr, last_len);
		status = FLASH_Program(&flashInstance, last_addr, TempBuf, last_len);
    }
    
    status = FLASH_Erase(&flashInstance, USER_CFGADDR, USER_CFGSIZE, kFLASH_ApiEraseKey);  ;
    status = FLASH_VerifyErase(&flashInstance, USER_CFGADDR, USER_CFGSIZE);
    status = FLASH_Program(&flashInstance, USER_CFGADDR, TempBuf, USER_CFGSIZE);
    /* Verify programming by reading back from flash directly */
    for (j = 0; j < (USER_CFGSIZE/4); j++)
    {
        status = *(volatile uint32_t *)(USER_CFGADDR);
    }

    free(TempBuf);
    PRINTF("*** APP_SYS_Format Format Done\r\n");
    if(reset == 1)
    {
        NVIC_SystemReset();
    }
}
