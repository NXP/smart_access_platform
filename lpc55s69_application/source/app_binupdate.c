/*
 * Copyright (c) 2017 - 2018 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
#include "stdint.h" 
#include "stdio.h"
#include "string.h"
#include "ctype.h"
#include "app_binupdate.h"
#include "app_printf.h"

volatile uint8_t  g_DebugRecvBuf[DEBUG_BUFFER_SIZE];
volatile uint16_t g_DebugrxIndex = 0;                   /* Index of the memory to save new arrived data. */
volatile uint8_t *g_DebugCmdCmp  = NULL;     
volatile uint8_t  g_DebugCmdStatus = 0;
volatile uint32_t g_BinaryNum = 0;
volatile uint32_t g_UpdateTickCnt = 0;

volatile uint8_t *g_BinaryImage = NULL;
volatile uint32_t g_BinaryCnt = 0;

/**
 * @brief   Clean BLE Task Status
 * @param   NULL
 * @return  NULL
 */
static void debugstatus_clean(void)
{
    g_DebugrxIndex = 0;
    memset((uint8_t*)g_DebugRecvBuf, 0x00, DEBUG_BUFFER_SIZE);
    g_DebugCmdStatus = 0;
}

/**
 * @brief   
 * @param   
 * @return  
 */
void binupdate_init(void)
{
    g_DebugrxIndex   = 0;
    g_DebugCmdStatus = 0;
    g_BinaryNum = 0;
}

/**
 * @brief   
 * @param   
 * @return  
 */
uint8_t binupdate_task(void)
{
    uint32_t i;
    volatile uint8_t *g_DebugCmdCmp  = NULL;       

    if(g_DebugCmdStatus == 1)
    {
        strupr((char *)g_DebugRecvBuf);
        g_DebugCmdCmp = (volatile uint8_t *)strstr((const char *)g_DebugRecvBuf, (const char *)"UPDATEBIN");
        if(g_DebugCmdCmp != NULL)
        {
            g_BinaryCnt = 0;
            g_BinaryNum = 0;
            g_BinaryImage = 0x04000000;
            memset((void *)g_BinaryImage, 0x00, 0x4000);
            g_DebugCmdStatus = 2;
            PRINTF("OK");
            
            for(g_BinaryNum=0; g_BinaryNum<RECV_FILE_NUMS; g_BinaryNum++)
            {
                while(g_UpdateTickCnt != 0)
                {
                    g_UpdateTickCnt--;
                }
                spiflash_write_file(g_BinaryNum*4, g_BinaryImage, 0x4000); /* Save binary into spi flash */
                g_BinaryImage = 0x04000000;
                memset((void *)g_BinaryImage, 0x00, 0x4000);
                PRINTF("Cnt %d  ", g_BinaryNum);
                g_BinaryCnt = 0;
                g_UpdateTickCnt = 0x700000*6;
                PRINTF("TimeOut\r\n");
            }
        }
        
        debugstatus_clean();
    }

    if(g_BinaryNum >= RECV_FILE_NUMS)
    {
    	return 0;
    }
    return 1;
}
