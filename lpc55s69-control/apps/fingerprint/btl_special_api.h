
#ifndef H_BTL_SPECTRAL_API_H
#define H_BTL_SPECTRAL_API_H

#include "btl_bep.h"


typedef enum {
	BTL_SCS_OK = 0,           
  BTL_SCS_SPI_ERR, 
	BTL_SCS_ID_ERR,
	BTL_SCS_RST_ERR,
	BTL_SCS_INT_ERR,
} btl_scs_t; 


typedef enum {
	BTL_SENSOR_POWER_ON = 0,   //打开电源
  BTL_SENSOR_POWER_OFF = 1,  //关闭电源          
} btl_power_mode_t; 

typedef void (*BtlSensorPowerControlFuncPtr)(btl_power_mode_t mode);
	
/*
sensor电源控制接口初始化
*/
extern void btl_sensor_power_control_init(BtlSensorPowerControlFuncPtr PowerControl);





extern btl_rc_t btl_create_multitemplate_ex(
		const PBL_TEMPLATE pblTemplates[],   //[in] 待合并指纹模板
		unsigned char nNbrOfTemplates,       //[in] 待合并指纹模板数量
		unsigned char fix_num,               //[in] 固定不可更新的模板数量
		PBL_TEMPLATE pblMultiTemplates);     //[out] 合并后的多模板数组，注意：用户一定要及时释放，否则可能引起内存错乱等未知错误。



		
		
		/*
匹配,支持一对一或一对多个模板比对
*/
extern  btl_rc_t btl_verify_pair(
		PBL_TEMPLATE pblTemplate, //[in] 待匹配模板
		PBL_TEMPLATE blMultiTemplatesArray[], //[in] 待匹配模板数组
		int nNbrOfMultiTemplates, //[in] 数组大小
		int *pMatchIndex //[out] 匹配成功的模板序号 		
		);

		
		
		
				
/*
匹配,不同模板使用不同的安全等级
*/
extern btl_rc_t btl_verify_ex(
		PBL_TEMPLATE_EX blMultiTemplatesExArray,
		int nNbrOfMultiTemplates,
		int *pMatchIndex
		);


		
///*SENSOR手指检测频率枚举*/
//typedef enum
//{
//	BTL_HIGH_FRE_MODE = 0, //高频模式
//  BTL_LOW_FRE_MODE = 1,  //低频模式(低功耗)            
//} btl_fre_mode_t;   

//		
///*
//扫描频率控制
//*/
//extern btl_rc_t btl_fre_control(btl_fre_mode_t mode);



/***********************************************************************
 * @name    btl_get_similarity_score
 * @brief   获取单个模板与多个模板的相似度分数
 
 * @param   multi_templates：[in] 已经注册多模板指针
 * @param   single_Template：[in] 待校验模板指针
 * @param   score：[out] 输出分数值指针
 * @retval  btl_rc_t
 **********************************************************************/
 
extern btl_rc_t btl_get_similarity_score(const PBL_TEMPLATE multi_templates, const PBL_TEMPLATE single_Template, unsigned short *score);

/***********************************************************************
 * @name    btl_decode_multiTemplates
 * @brief   解码多模板，返回子模板数组
 
 * @param   pblMultiTemplates: [in] 多模板数据
 * @param   pblSubTemplates: [out] 返回子模板数组
 * @param   nNbrOfSubtemplates: [out] 子模板数量
 * @retval  btl_rc_t
 **********************************************************************/
extern btl_rc_t btl_decode_multiTemplates(const PBL_TEMPLATE pblMultiTemplates, PBL_TEMPLATE** pblSubTemplates, unsigned char* nNbrOfSubtemplates);

/***********************************************************************
 * @name    btl_delete_subtemplates
 * @brief   释放子模板数组占用的资源
 
 * @param   pblSubTemplates: [in] 子模板数组
 * @param   nNbrOfSubtemplates: [in] 子模板数量
 * @retval  btl_rc_t
 **********************************************************************/
extern btl_rc_t btl_delete_subtemplates(PBL_TEMPLATE* pblSubTemplates, unsigned char nNbrOfSubtemplates);



/***********************************************************************
 * @name    btl_get_score_by_subtemplates
 * @brief   子模板间的相似程度分
 
 * @param   pblSubTemplates:    [in] 子模板数组
 * @param   nNbrOfSubTemplates: [in] 子模板数量
 * @retval  pSimilarityScores:  [in/out] 传入元素个为nNbrOfSubTemplates的数组，由内部填充相似分值
 **********************************************************************/
extern btl_rc_t btl_get_score_by_subtemplates(PBL_TEMPLATE* pblSubTemplates, unsigned char nNbrOfSubTemplates,	unsigned short* pSimilarityScores);



/***********************************************************************
 * @name    btl_verify_speed
 * @brief   单个模板与多模板比对
 
 * @param   pblMultiTemplates:       [in] 多模板数据
 * @param   nNbrOfMultiTemplates:    [in] 已录入模板数量	
 * @param   nFarAccepted:            [in] 比对安全等级
 * @param   nMaxNbrOfSubTempaltes：  [in] 每个多模板中参与比对的最大子模板数
 * @param   pMatchIndex：            [out] 匹配成功的模板序号，从0开始，-1表示匹配失败
 * @retval  btl_rc_t
 **********************************************************************/
extern btl_rc_t  btl_verify_speed(
    const PBL_TEMPLATE* pblMultiTemplates,  //[in] 已录入多模板数组 
    unsigned char nNbrOfMultiTemplates,     //[in] 已录入模板数量	
    int nFarAccepted,                       //[in] FAR
    unsigned char  nMaxNbrOfSubTempaltes,   //[in] 每个多模板中参与比对的最大子模板数
    int* pMatchIndex                        //[out] 匹配成功的模板序号，从0开始，-1表示匹配失败
    );



/***********************************************************************
 * @name    btl_image_enhance
 * @brief   图像增强

 * @retval  btl_rc_t
 **********************************************************************/
extern btl_rc_t btl_image_enhance(void);



/***********************************************************************
 * @name    btl_getBadLinePixelsNum
 * @brief   获取坏点个数
 
 * @param   nBadPixelsNum:   [out] 坏点个数
 * @retval  int          ：  0表示成功，else表示失败
 **********************************************************************/			
extern int btl_getBadLinePixelsNum(int* nBadPixelsNum);  //[out]  坏点个数



/***********************************************************************
 * @name    btl_computeSNR
 * @brief   计算图像信号噪声
 
 * @param   pSNR   :  [out] 信噪比
 * @param   pSignal:  [out] 信号量
 * @param   pNoise :  [out] 噪声量

 * @retval  int    ： 0表示成功，else表示失败
 **********************************************************************/
extern int btl_computeSNR( unsigned char* pSNR, unsigned char* pSignal, unsigned char* pNoise);




/***********************************************************************
 * @name    btl_get_cap
 * @brief  获取图像的电容值
 
 * @param   nCap   :  [out] 输出的电容值
 * @retval  btl_rc_t    ： BTL_RC_OK表示成功，else表示失败
 **********************************************************************/
extern btl_rc_t btl_get_cap(int* nCap);

/***********************************************************************
 * @name    btl_add_cap
 * @brief   添加电容值
 
 * @param   nCap   :  [in] 需要添加的电容值
 * @retval  btl_rc_t  ： BTL_RC_OK表示成功，else表示失败
 **********************************************************************/
extern btl_rc_t btl_add_cap(int nCap);

/***********************************************************************
 * @name    btl_finalize_cap
 * @brief   获取最大的电容值
 
 * @param   nMaxCap   :  [out] 输出的最大电容值
 * @retval  btl_rc_t  ： 0表示成功，else表示失败
 **********************************************************************/
extern btl_rc_t btl_finalize_cap(int* nMaxCap);



/***********************************************************************
 * @name    btl_force_image_without_agc
 * @brief   强制采图，不做AGC调整，保留原始数据，可以用于测试灵敏度

 * @retval  btl_rc_t     ： BTL_RC_OK表示成功，else表示失败
 **********************************************************************/
extern btl_rc_t btl_force_image_without_agc(void);

/***********************************************************************
 * @name    btl_get_image_center_value
 * @brief   获取图像48*48中心区域的灰度均值，量产检测灵敏度用
 * @param   nValue       :  [out]返回图像灰度均值
 * @retval  btl_rc_t     ： BTL_RC_OK表示成功，else表示失败
 **********************************************************************/
extern btl_rc_t btl_get_image_center_value(unsigned char* nValue);


/***********************************************************************
 * @name    btl_force_image_without_print
 * @brief   强制多次采图，不上传图像数据，供一体化模组量产测试采图功耗用
 
 * @param   times             :  [in] 采图次数，最多255次
 * @retval  unsigned char     ： 采图出错的次数
 **********************************************************************/
extern unsigned char btl_force_image_without_print(unsigned char times);



/***********************************************************************
 * @name    btl_read_register
 * @brief   获取某个寄存器的值
 
 * @param   nRegAddr   :  [in] 寄存器地址
 * @param   pData      :  [out] 返回寄存器值
 * @retval  btl_rc_t   ： BTL_RC_OK表示成功，else表示失败
 **********************************************************************/
extern btl_rc_t btl_read_register(unsigned char nRegAddr, unsigned char *pData);

/***********************************************************************
 * @name    btl_write_register
 * @brief   设置某个寄存器的值
 
 * @param   nRegAddr   :  [in] 寄存器地址
 * @param   nData      :  [in] 寄存器值
 * @retval  btl_rc_t   ： BTL_RC_OK表示成功，else表示失败
 **********************************************************************/
extern btl_rc_t btl_write_register(unsigned char nRegAddr, unsigned char nData);

/***********************************************************************
 * @name    btl_imagePreprocessEx
 * @brief   支持传入外部图像预处理接口，对原始图像进行预处理，如图像平滑，图像减底噪等，此接口不受btl_get_config()->ImageEnhance限制
 
 * @param   image   :  [in] 原始图像数据
 * @param   w       :  [in] 图像宽
 * @param   h       :  [in] 图像高
 * @retval  int     ： 0表示成功，else表示失败
 **********************************************************************/
extern int (*btl_imagePreprocessEx)(unsigned char* image, int w, int h);

/***********************************************************************
 * @name    btl_get_base_image
 * @brief   采集BASE图像，供后期采图减BASE使用
 * @param   nDevThreshold     :  [in] 图像方差阈值，推荐值300
 * @param   nSnrThreshold     :  [in] 图像信噪比阈值，推荐值10
 * @param   nSignalThreshold  :  [in] 图像信号量阈值，推荐值20
 * @retval  btl_rc_t     
 ：返回BTL_RC_OK之后，通过btl_get_image_data()、btl_get_image_width()、btl_get_image_height()接口获取图像数据
 **********************************************************************/
extern btl_rc_t  btl_get_base_image(int nDevThreshold, unsigned char nSnrThreshold, unsigned char nSignalThreshold);

/***********************************************************************
 * @name    btl_minus_base_image
 * @brief   原始图像去底噪

 * @param   srcImg     :  [in] 原始图像数据
 * @param   pImageBase :  [in] BASE图像
 * @param   nWidth     :  [in] 图像宽
 * @param   nHeight    :  [in] 图像高
 * @param   nBaseMean  :  [in] BASE图像的灰度均值，传入0则由内部计算
 * @retval  btl_rc_t   ：
 **********************************************************************/
extern btl_rc_t btl_minus_base_image(unsigned char* srcImg, const unsigned char* pImageBase,  int nWidth, int nHeight, unsigned char nBaseMean);

/***********************************************************************
 * @name    blt_sensor_connected_status_test
 * @brief   Sensor连接状态检查，包括SPI、RST、INT和Chip ID核验

 * @retval  btl_scs_t   ：
 **********************************************************************/
extern btl_scs_t blt_sensor_connected_status_test(void);



/***********************************************************************
 * @name    btl_disable_finger_up_once
 * @brief   单次取消手指抬起检测

 * @retval  void
 **********************************************************************/
extern void btl_disable_finger_up_once(void);


/***********************************************************************
 * @name    btl_update_definite_templates
 * @brief   update definited template into multitamplate

 * @param   pblTemplate            :  [in] the new definited template
 * @param   pblMultiTemplates      :  [in] the old multitamplates
 * @param   pblUpdateMultiTemplates:  [out] the outcome
 * @retval  btl_rc_t   ：
 **********************************************************************/

extern int btl_update_definite_templates(const PBL_TEMPLATE pblTemplate, const PBL_TEMPLATE pblMultiTemplates, PBL_TEMPLATE pblUpdateMultiTemplates);

#endif


























