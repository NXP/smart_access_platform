/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <btl_sensor.h>
#include "btl_bep.h"
#include "fsl_iap.h"
#include "fsl_iap_ffr.h"

#include "lpc5500_fpslib.h"
#include "app_printf.h"
#include "app_fingerprint.h"
#include "app_syscfg.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint16_t   g_fingerEnrollId = 0; 
volatile uint8_t    g_fingerIndex = 0;
volatile uint8_t    g_fingerForbidSameFinger = 0; // 0:Allowed Record,1:Forbiden
volatile uint32_t   g_fingerTimerTicks = 0;
BEP_INIT_PARAM      g_fingerBEPDriverAPI;

volatile uint32_t   g_fingerSensorID = 0;

volatile uint8_t*   g_fingerImageBuf = NULL;      // 10752
BTL_PARAM           s_AlgSensorParam;
BL_TEMPLATE         blMultiTemplates;
BL_TEMPLATE         BtlTemplate;
volatile btl_rc_t   g_BTLResult = 0 ;

WORK_STRUCTURE      g_fingerWorkMode;
FP_OPT g_OptGlobal;

volatile uint8_t    Array[FPS_STORE_FINGERS] = {0};
BL_TEMPLATE  blMultiTemplatesArray[FPS_STORE_FINGERS]; 
PBL_TEMPLATE pblMultiTemplatesArray[FPS_STORE_FINGERS];

volatile uint32_t statusFlash;
flash_config_t  flashInstance;

volatile uint32_t pflashBlockBase = 0;
volatile uint32_t pflashTotalSize = 0;
volatile uint32_t pflashSectorSize = 0;
volatile uint32_t PflashPageSize = 0;
            
volatile uint32_t g_flashAddress = 0;

volatile uint8_t  g_FPFlashBuf[32768];

extern bool g_FpEnrollInterrupted;

/*******************************************************************************
 * Code
 ******************************************************************************/


/********************************************************
�������ܣ�g_OptGlobal�ṹ������ʼ��
���������sensor ID
���أ���
********************************************************/
void FPS_BTLGlobalInit(uint32_t SensorID, uint32_t ImageStorageAddr, uint32_t ImageStorageSize)
{
    switch(SensorID) {
        case 0x81072:
            g_OptGlobal.EnrollTimes = 12;
            g_OptGlobal.MaxEnrollTimes = 12;
            g_OptGlobal.FingerFlashSize = 16384;  
        break;
        case 0x81088:
            g_OptGlobal.EnrollTimes = 6;
            g_OptGlobal.MaxEnrollTimes = 12;
            g_OptGlobal.FingerFlashSize = 8192;  
        break;
        case 0x81160:				
        case 0x81192:
            g_OptGlobal.EnrollTimes = 3;
            g_OptGlobal.MaxEnrollTimes = 6;
            g_OptGlobal.FingerFlashSize = 4096; 
        break;
        case 0x82160:
        case 0x82192:
            g_OptGlobal.EnrollTimes = 3;
            g_OptGlobal.MaxEnrollTimes = 6;
            g_OptGlobal.FingerFlashSize = 4096;
        break;

        default:
            
        break;
    }
    
    g_OptGlobal.StoreFingers = FPS_STORE_FINGERS;
}


uint8_t FPS_SensorInit(fingersensor_initfunc_t func)
{
    btl_rc_t errRet;
    
    func.sensor_init();

    g_fingerBEPDriverAPI.delay_ms      = func.sensor_delayms;
    g_fingerBEPDriverAPI.delay_us      = func.sensor_delayus;
    g_fingerBEPDriverAPI.pImage        = (unsigned char *)g_fingerImageBuf;
    g_fingerBEPDriverAPI.rstn_set      = func.sensor_reset;
    g_fingerBEPDriverAPI.spi_cs_set    = func.sensor_select;
    g_fingerBEPDriverAPI.spi_rev_byte  = func.sensor_spirecv;
    g_fingerBEPDriverAPI.spi_send_byte = func.sensor_spisend;
    
    g_flashAddress = func.sensor_getaddress();
    
    errRet = btl_bep_init(&g_fingerBEPDriverAPI);
    
	if(errRet != BTL_RC_OK )
	{
#if USE_FINGERPRINT
		//errRet = blt_SensorConnectedStatusTest();
        FPS_PRINTF("errRet != BTL_RC_OK  %x %x\r\n", errRet);
        errRet = blt_SensorConnectedStatusTest();
        FPS_PRINTF("errRet = blt_SensorConnectedStatusTest  %x\r\n", errRet);
#endif
		return 0xFF;  /* Init Failed */
	}
    
    g_fingerSensorID = btl_get_sensor_id();
    FPS_BTLGlobalInit(g_fingerSensorID, func.sensor_flashaddr, func.sensor_flashsize);
#if 1
    s_AlgSensorParam = btl_get_param();
    btl_set_param_spectral(&s_AlgSensorParam);
#else
    s_AlgSensorParam = btl_get_param();
    s_AlgSensorParam.max_template_size = 4 * 1024 - 10; //4KB
//    if((g_fingerSensorID == 0x81088)||(g_fingerSensorID == 0x81072))
//    {
//        s_AlgSensorParam.algorithm_type = BTL_ALG_GOOD_BIOMETRIC_PERFORMANCE;
//    }
//    else
//    {
//        s_AlgSensorParam.algorithm_type = BTL_ALG_FAST_SPEED; //
        //
        btl_get_config()->Dacp_min = 0x70;
        btl_get_config()->Dacp_max = 0xff;
        s_AlgSensorParam.dacp_val  = 0x80;
        s_AlgSensorParam.reg31_val = 0x4f;
        s_AlgSensorParam.reg32_val = 0x33;
//    }  
    //s_AlgSensorParam.max_template_size = g_OptGlobal.FingerFlashSize-9; //
    //s_AlgSensorParam.support_360rotate = 1;   
//  s_AlgSensorParam.prevent_enroll_multifingers = 1;   	
   btl_set_param = btl_set_param_hybrid; //
   
    btl_set_param(&s_AlgSensorParam);  
    btl_get_config()->nFarAccepted   = BL_FAR_500000;
    btl_get_config()->nEnrollCounts  = g_OptGlobal.EnrollTimes;   	
    //btl_get_config()->bForbidSamePos = 1;	//
    //btl_get_config()->AGC_controler  = 1;
//	btl_get_config()->image_quality_for_verify.min_area = 70;
#endif
	/* Initiliaze parameters */
    blMultiTemplates.pTemplateData = 0;
    BtlTemplate.pTemplateData = NULL;    	
    /* Set FingerSensor Work Mode:Enrrol, Verify, Idle */
    g_fingerWorkMode.mode = BTL_IDLE_MODE;
    return 0;
}

/**
 * @brief   FPS_BTLIntHandler, GPIO interrupt Handler
 * @param   NULL
 * @return  NULL
 */
void FPS_IntHandler(void)
{
    btl_sensor_int_handle();  	
}

/**
 * @brief   FPS_BTLTickHandler, Should place to 1mS Timer handler
 * @param   NULL
 * @return  NULL
 */
void FPS_TickHandler(void)
{
    btl_timer_ms_handle();
    g_fingerTimerTicks++;
}

/**
 * @brief   FPS_GetSensorId, return sensors ID
 * @param   NULL
 * @return  sensor ID
 */
uint32_t FPS_SensorIdGet(void)
{
    return g_fingerSensorID;
}

/**
 * @brief   FPS_TemplateWrite
 * @param   template & ID
 * @return  status
 */
uint8_t FPS_TemplateWrite(PBL_TEMPLATE pblTemplate, int finger, int id)
{
	uint32_t flash_addr;
	// capture a empty image
	id = finger;

	flash_addr = g_flashAddress + (finger/8)*32768;  /* */

	if (finger < g_OptGlobal.StoreFingers) {
        uint8_t type        = pblTemplate->templateType;
        const uint8_t* data = pblTemplate->pTemplateData;
        uint32_t data_size  = pblTemplate->templateSize;
        uint8_t	*ptr = NULL;
        ptr= malloc(g_OptGlobal.FingerFlashSize);
        if (ptr == NULL)
        {
#if USE_FINGERPRINT
            FPS_PRINTF("failed : malloc(OptGlobal.FingerFlashSize)\r\n");
#endif
            return 0xF1;
        }
        ptr[0] = 0xf0;
        ptr[1] = type;
        ptr[2] = (uint8_t)data_size;
        ptr[3] = (uint8_t)(data_size>>8);
        ptr[4] = (uint8_t)(data_size>>16);
        ptr[5] = (uint8_t)(data_size>>24);
        ptr[6] = (uint8_t)(id);
        ptr[7] = (uint8_t)(id>>8);
        ptr[8] = 0x00; //Ȩ��ֵ������
        memcpy(&ptr[9],data,data_size);

        if (data_size > (g_OptGlobal.FingerFlashSize-9))
        {
#if USE_FINGERPRINT
            FPS_PRINTF("    template is too large %d for flash storage configuration of %d bytes\n", 
                    (int)data_size, (g_OptGlobal.FingerFlashSize-9));
            FPS_PRINTF("     either increase define STORE_SIZE or set -mt_size_bytes\n");
#endif
            free(ptr);
            return 0xF2;
        } 
        __disable_irq();
		if(g_OptGlobal.FingerFlashSize==2048)	
		{      
#if USE_FINGERPRINT
            FPS_PRINTF("!!! Not Support 2K Template\r\n");
#endif
		}
		else if(g_OptGlobal.FingerFlashSize==4096)
		{       
#if USE_FINGERPRINT
            FPS_PRINTF("!!! FLASH_Init ing, 4096, FMC Cache %x\r\n", SYSCON->FMCCR);
#endif

            memcpy((void *)g_FPFlashBuf, (uint8_t *)flash_addr, 32768);               /* ??Flash???Buffer */
            memcpy((void *)&g_FPFlashBuf[(finger%8)*4096], (uint8_t *)ptr, 4096);     /*  */

            if (FLASH_Init(&flashInstance) != kStatus_Success)
            {
#if USE_FINGERPRINT
                FPS_PRINTF("!!! FLASH_Init Failed!\r\n");
#endif
                return 0xF4;
            }

#if USE_FINGERPRINT
            FPS_PRINTF("!!! FLASH_Init OK!\r\n");
#endif
            statusFlash = FLASH_Erase(&flashInstance, flash_addr, 32768, kFLASH_ApiEraseKey);
#if USE_FINGERPRINT
//            verify_status(statusFlash);
            FPS_PRINTF("!!! FLASH_Erase Done!\r\n");
#endif  
//            statusFlash = FLASH_VerifyErase(&flashInstance, flash_addr, OptGlobal.FingerFlashSize);
            statusFlash = FLASH_VerifyErase(&flashInstance, flash_addr, 32768);
#if USE_FINGERPRINT
 //           verify_status(statusFlash);
#endif 
            if (statusFlash != kStatus_Success)
            {
#if USE_FINGERPRINT
                FPS_PRINTF("!!! FLASH Verify Erase Failed!\n");
#endif
                return 0xF5;
            }
#if USE_FINGERPRINT
            FPS_PRINTF("!!! FLASH_Program Addr 0x%x Size %d!\r\n", flash_addr, 32768);//OptGlobal.FingerFlashSize);
#endif
            statusFlash = FLASH_Program(&flashInstance, flash_addr, g_FPFlashBuf, 32768);//OptGlobal.FingerFlashSize);
#if USE_FINGERPRINT
            /* Verify programming by reading back from flash directly */
            FPS_PRINTF("!!! Read Addr 0x%x Size %d!\r\n", flash_addr, 32768); // OptGlobal.FingerFlashSize);
#endif
            for (uint32_t i = 0; i < (32768/4); i++)
            {
                statusFlash = *(volatile uint32_t *)(flash_addr + i * 4);
                //FPS_PRINTF("!!! Read Addr %x %x !\r\n", (flash_addr + i * 4), statusFlash);
            }
#if USE_FINGERPRINT
            FPS_PRINTF("!!! Exit!\r\n");    
#endif
		}
		else if(g_OptGlobal.FingerFlashSize==8192)		
		{
#if USE_FINGERPRINT
            FPS_PRINTF("!!! Not Support 8K Template\r\n");
#endif
		}
		else if(g_OptGlobal.FingerFlashSize==16384)		
		{
#if USE_FINGERPRINT
            FPS_PRINTF("!!! Not Support 16K Template\r\n");
#endif
        }
        free(ptr);
        __enable_irq();

#if 0		
        if (Sts != kStatus_FLASHIAP_Success)
        {
            return 0xF6;
        }
#endif 
#if USE_FINGERPRINT
        FPS_PRINTF("FLASH updated - template size %d, maximum is %d bytes\r\n", (int)data_size, g_OptGlobal.FingerFlashSize);
#endif
        return 0x00;
    }
    else {
        return 0xF7;
    }
}

uint8_t FPS_TemplateRead(PBL_TEMPLATE pblTemplate, int finger)
{
	volatile uint32_t flash_addr;
	volatile uint8_t  flag;
	volatile uint8_t  type;
	volatile uint32_t data_size;
	volatile uint32_t pTemplateData;
    
	flash_addr = g_flashAddress + ( finger * g_OptGlobal.FingerFlashSize ); 
    
	if (finger < g_OptGlobal.StoreFingers) 
	{
		flag = *(uint8_t*)(flash_addr);
		type = *(uint8_t*)(flash_addr+1);
		data_size  = *(uint32_t*)(flash_addr+2); 
		pTemplateData = flash_addr+9;
		
		if (flag != 0xf0)
		{
#if USE_FINGERPRINT
            FPS_PRINTF("Read finger %d Failed\r\n", finger);
#endif
			return 0xF1;
		}
		else 
		{
#if USE_FINGERPRINT
            FPS_PRINTF("Read finger %d Sucess\r\n", finger);
#endif
			pblTemplate->pTemplateData = (uint8_t *)pTemplateData;  
			pblTemplate->templateType = type;
			pblTemplate->templateSize = data_size;
			return 0x01;
		}		
	}
	else 
	{
		return 0xF2;
	}
}

/**
 * @brief FPS_TemplateDelete
 * @param   
 * @return  
 */
void FPS_TemplateDelete(uint8_t id, uint8_t num)
{  
    uint32_t addr;
	uint8_t  *pData;
	uint8_t  *pDataT;
	uint8_t  *pflash;
	uint32_t i;
	uint8_t  nID,cnt=0;
	float nSector; 
	
	if(num == 0) num = 1;
    
	nSector = (float)num*g_OptGlobal.FingerFlashSize;
	nSector = nSector/32768;
	if((nSector- (int)nSector) != 0)
		nSector = (int)nSector + 1;

	nID = id;
	pDataT = malloc(32768);
	do
	{
        addr = g_flashAddress + id * g_OptGlobal.FingerFlashSize/32768*32768;
		pflash = (uint8_t *)(g_flashAddress + id*g_OptGlobal.FingerFlashSize/32768*32768);  
		pData = pDataT;
		for(i=0;i<32768;i++)
		{
			*pData = *pflash;
			pData++;
			pflash++;
		}  		
		do
		{
			pData = pDataT;
			pData += (nID%(32768/g_OptGlobal.FingerFlashSize)*g_OptGlobal.FingerFlashSize); 	
			for(i=0;i<g_OptGlobal.FingerFlashSize;i++)
			{
				*pData= 0xff;
				pData++;
			}
			nID++;
			cnt++;
			if((cnt>=num)||(nID>=g_OptGlobal.StoreFingers))
				break;
		}while((nID%(32768/g_OptGlobal.FingerFlashSize)) != 0);
		
		__disable_irq();

        if (FLASH_Init(&flashInstance) != kStatus_Success)
        {
        }

        statusFlash = FLASH_Erase(&flashInstance, (uint32_t)addr, g_OptGlobal.FingerFlashSize, kFLASH_ApiEraseKey);
        statusFlash = FLASH_VerifyErase(&flashInstance, (uint32_t)addr, g_OptGlobal.FingerFlashSize);
        if (statusFlash != kStatus_Success)
        {
#if USE_FINGERPRINT
            FPS_PRINTF("*** FLASH Verify Erase Failed!\n");
#endif
        }
        /* Verify programming by reading back from flash directly */
        for (uint32_t i = 0; i < g_OptGlobal.FingerFlashSize; i++)
        {
            statusFlash = *(volatile uint32_t *)(addr + i * 4);
        }
        __enable_irq();	
        id = nID;   		
    }  while((cnt<num)&&(nID<g_OptGlobal.StoreFingers));
    free(pDataT);    
}

/**
 * @brief   
 * @param   
 * @return  
 */
uint8_t FPS_TemplateFindDelete(uint16_t id, uint16_t* finger_index)
{
	uint16_t finger;
	uint32_t flash_addr;
	uint8_t  flag;
	uint16_t Temp;
	finger_index[0] = FPS_STORE_FINGERS;
    
	for(finger=0;finger<FPS_STORE_FINGERS;finger++)
	{
		flash_addr = g_flashAddress + (finger*g_OptGlobal.FingerFlashSize);

		flag = *(uint8_t *)(flash_addr);
		Temp = *(uint16_t*)(flash_addr+6);
		if(flag==0xf0)
		{
			if(id == Temp)
			{
				*finger_index = finger;	//?????ID????
				return 1; //ID???
			}
		}
	}
	return 0;
}


/* Start Timer */
void FPS_TIMERStart(void)
{
	g_fingerTimerTicks = 0;
}

/* Return Timer count Values */
uint32_t FPS_TIMERGet(void)
{
    return (g_fingerTimerTicks);
}

uint8_t FPS_VerifyTask(uint16_t *finger_index, uint32_t *TimeCost)
{
    volatile uint32_t num_fingers, i, j;
    volatile btl_rc_t retValue;
    /* Finger detection + mode select function, used in main thread */
    FPS_TIMERStart();
    if(g_fingerWorkMode.mode == BTL_VERIFY_MODE)
    {
        retValue = btl_finger_detect(BTL_VERIFY_MODE);
        if( retValue == BTL_RC_OK )
        {
            num_fingers = 0;
            j = 0;
            memset( (void *)Array, 0, g_OptGlobal.StoreFingers );
            
            for (i = 0; i < g_OptGlobal.StoreFingers; i++)
            {
                if(FPS_TemplateRead(&blMultiTemplatesArray[num_fingers], i) == 0x01)
                {
                    pblMultiTemplatesArray[num_fingers] = &blMultiTemplatesArray[num_fingers];  					
                    num_fingers++;	
                    Array[j++] = i;					
                }
            }
            
            if(num_fingers >= 50)
            {
                btl_get_config()->nVerifyTimeOut = 1300; // 500ms
            }
            else if((g_fingerSensorID == 0x81088)||(g_fingerSensorID == 0x81072))
            {
                btl_get_config()->nVerifyTimeOut = 1300 + num_fingers*10;
            }
            else
            {
                btl_get_config()->nVerifyTimeOut = 300 + num_fingers*10;
            }
            FPS_TIMERStart();

#if USE_FINGERPRINT
            FPS_PRINTF("!!! Start Verify, num_fingers%d, Index %d \r\n", num_fingers, g_fingerWorkMode.MatchIndex);
#endif
            g_BTLResult = btl_verify(pblMultiTemplatesArray, num_fingers, &g_fingerWorkMode.MatchIndex);
            
            if(TimeCost != NULL)
            {
                *TimeCost = FPS_TIMERGet();            
            }
#if USE_FINGERPRINT
            FPS_PRINTF("Time %d\r\n", *TimeCost);
#endif             
            if( g_BTLResult == BTL_RC_OK && g_fingerWorkMode.MatchIndex >= 0)
            {
                *finger_index = *(uint16_t*)(g_flashAddress + (Array[g_fingerWorkMode.MatchIndex]*g_OptGlobal.FingerFlashSize)+6);
                if(g_UserCfg.fp_status[*finger_index] == 0xff)
				{
					return FPS_IMAGEIDLE;
				}
#ifdef DYNAMIC_UPDATE
                g_BTLResult = btl_dynamic_update_templates(blMultiTemplatesArray[g_fingerWorkMode.MatchIndex],&blMultiTemplates);
                if(g_BTLResult==BTL_RC_OK)
                {
                    if( find_template_tobe_del(Temp,&Offset) )
                    {
                        delete_template(Offset,1); //
                    }
                    flash_write_template(&blMultiTemplates, Offset, Temp);
                }
#endif                
                btl_delete_template(&blMultiTemplates);
#if USE_FINGERPRINT
                FPS_PRINTF("Verify OK, ID is %x, %dmS\r\n", *finger_index, *TimeCost);
#endif
                PRINTF("Verify OK, ID is %x, %dmS\r\n", *finger_index, *TimeCost);
                g_fingerWorkMode.mode = BTL_VERIFY_MODE;
                return FPS_IMAGEVALID;
            }
            else
            {
#if USE_FINGERPRINT
                FPS_PRINTF("Verify Failed Time %dmS\r\n", *TimeCost);
#endif
                PRINTF("Verify Failed Time %dmS\r\n", *TimeCost);
                return FPS_IMAGEILLEGAL;
            }
        }
    }
    
    /* false */
    return FPS_IMAGEIDLE;
}

uint8_t FPS_RegistTask(uint16_t *finger_index, uint16_t *finger_times, uint16_t *finger_total)
{
    volatile btl_rc_t ret;
    g_fingerEnrollId = *finger_index;
    g_fingerIndex = g_fingerEnrollId;
    if( g_fingerWorkMode.mode == BTL_ENROLLMENT_MODE )
    {
        if (g_FpEnrollInterrupted)
        {
            g_FpEnrollInterrupted = false;
            g_fingerWorkMode.AcceptedNum = 0;
            btl_delete_template(&blMultiTemplates);
            g_fingerWorkMode.mode = BTL_VERIFY_MODE;
            return FPS_REGFAILED;
        }

        ret = btl_finger_detect(BTL_ENROLLMENT_MODE);
        if(ret == BTL_RC_OK)
        {
            /* Check whether function pointer is located in non-secure memory */
            blMultiTemplates.pTemplateData = 0;
            int island_num = 0;
            g_BTLResult = btl_enrollment(&g_fingerWorkMode.AcceptedNum, &island_num, &blMultiTemplates);
            if(g_BTLResult == BTL_RC_OK)
            {
                if( g_fingerWorkMode.AcceptedNum >= (btl_get_config()->nEnrollCounts))
                {
#if USE_FINGERPRINT
                    FPS_PRINTF("!!! FP Flash_write_template Index %d, by Enroll ID %d\r\n", g_fingerIndex, g_fingerEnrollId);
#endif
                    if(FPS_TemplateWrite(&blMultiTemplates, g_fingerIndex, g_fingerEnrollId) == 0)				
                    {
#if USE_FINGERPRINT
                        FPS_PRINTF("!!! FP Flash write template done\r\n");
#endif
                        btl_delete_template(&blMultiTemplates);
                        g_fingerIndex = g_fingerEnrollId;
                        g_fingerWorkMode.mode = BTL_VERIFY_MODE;
                        return FPS_REGOK;
                    }
                    else
                    {
#if USE_FINGERPRINT
                        FPS_PRINTF("!!! FP Flash write template failed\r\n");
#endif
                        g_fingerWorkMode.AcceptedNum = 0;
                        btl_delete_template(&blMultiTemplates);
                        g_fingerWorkMode.mode = BTL_VERIFY_MODE;
                        return FPS_REGFAILED;
                    }
                }
                else // Enrolling
                { 		
#if USE_FINGERPRINT
                    FPS_PRINTF("!!! ENROLLING... AcceptedNum %d, nEnrollCounts %d\r\n", g_fingerWorkMode.AcceptedNum,btl_get_config()->nEnrollCounts);
                    FPS_PRINTF("!!! Pls Place Your Finger again\r\n");
#endif
                    if(finger_times != NULL)
                    {
                        *finger_times = g_fingerWorkMode.AcceptedNum;
                    }
                    if(finger_total != NULL)
                    {
                        *finger_total = btl_get_config()->nEnrollCounts;
                    }
                    return FPS_REGING;
                }
            }
            else {
#if USE_FINGERPRINT
                FPS_PRINTF("g_BTLResult is %x \r\n", g_BTLResult);
#endif
                return FPS_REGNOFINGER;
            }
        }
    }
    return FPS_REGIDLE;
}

uint8_t* FPS_GetImageTask(void)
{
    volatile btl_rc_t s_BTLResult = 0 ;

    s_BTLResult = btl_finger_detect(BTL_GET_IMAGE_MODE);
    if(g_BTLResult == BTL_RC_OK)
    {
        g_fingerImageBuf = btl_get_image_data();
        return (uint8_t *)g_fingerImageBuf;
    }
    else
    {
        return NULL;
    }
    return NULL;
}

void FPS_WorkModeSet(uint8_t data)
{
    g_fingerWorkMode.mode = data;
}

uint8_t FPS_WorkModeGet(void)
{
    return g_fingerWorkMode.mode;
}

uint8_t FPS_LPC55FlashFormat(uint32_t addr, uint32_t sectors)
{
    uint32_t i,j;
    uint8_t* data_buf;
    
    __disable_irq();

    if (FLASH_Init(&flashInstance) != kStatus_Success)
    {
        return 0xF1;
    }

    data_buf = malloc(32768);
    memset(data_buf, 0xAA, 32768);
    
    for(i=0; i<sectors; i++)
    {
        statusFlash = FLASH_Erase(&flashInstance, (uint32_t)addr, 32768, kFLASH_ApiEraseKey);
        statusFlash = FLASH_VerifyErase(&flashInstance, (uint32_t)addr, 32768);
        if (statusFlash != kStatus_Success)
        {
            free(data_buf);
            return 0xF2;
        }
        statusFlash = FLASH_Program(&flashInstance, addr, data_buf, 32768);//OptGlobal.FingerFlashSize);
        /* Verify programming by reading back from flash directly */
        for (j = 0; j < (32768/4); j++)
        {
            statusFlash = *(volatile uint32_t *)(addr + j * 4);
        }
        addr = addr + 32768;
    }
    free(data_buf);
    __enable_irq();	
    return 0;
}

// end file
