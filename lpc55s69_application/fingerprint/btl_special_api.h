
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
	BTL_SENSOR_POWER_ON = 0,   //�򿪵�Դ
  BTL_SENSOR_POWER_OFF = 1,  //�رյ�Դ          
} btl_power_mode_t; 

typedef void (*BtlSensorPowerControlFuncPtr)(btl_power_mode_t mode);
	
/*
sensor��Դ���ƽӿڳ�ʼ��
*/
extern void btl_sensor_power_control_init(BtlSensorPowerControlFuncPtr PowerControl);





extern btl_rc_t btl_create_multitemplate_ex(
		const PBL_TEMPLATE pblTemplates[],   //[in] ���ϲ�ָ��ģ��
		unsigned char nNbrOfTemplates,       //[in] ���ϲ�ָ��ģ������
		unsigned char fix_num,               //[in] �̶����ɸ��µ�ģ������
		PBL_TEMPLATE pblMultiTemplates);     //[out] �ϲ���Ķ�ģ�����飬ע�⣺�û�һ��Ҫ��ʱ�ͷţ�������������ڴ���ҵ�δ֪����



		
		
		/*
ƥ��,֧��һ��һ��һ�Զ��ģ��ȶ�
*/
extern  btl_rc_t btl_verify_pair(
		PBL_TEMPLATE pblTemplate, //[in] ��ƥ��ģ��
		PBL_TEMPLATE blMultiTemplatesArray[], //[in] ��ƥ��ģ������
		int nNbrOfMultiTemplates, //[in] �����С
		int *pMatchIndex //[out] ƥ��ɹ���ģ����� 		
		);

		
		
		
				
/*
ƥ��,��ͬģ��ʹ�ò�ͬ�İ�ȫ�ȼ�
*/
extern btl_rc_t btl_verify_ex(
		PBL_TEMPLATE_EX blMultiTemplatesExArray,
		int nNbrOfMultiTemplates,
		int *pMatchIndex
		);


		
///*SENSOR��ָ���Ƶ��ö��*/
//typedef enum
//{
//	BTL_HIGH_FRE_MODE = 0, //��Ƶģʽ
//  BTL_LOW_FRE_MODE = 1,  //��Ƶģʽ(�͹���)            
//} btl_fre_mode_t;   

//		
///*
//ɨ��Ƶ�ʿ���
//*/
//extern btl_rc_t btl_fre_control(btl_fre_mode_t mode);



/***********************************************************************
 * @name    btl_get_similarity_score
 * @brief   ��ȡ����ģ������ģ������ƶȷ���
 
 * @param   multi_templates��[in] �Ѿ�ע���ģ��ָ��
 * @param   single_Template��[in] ��У��ģ��ָ��
 * @param   score��[out] �������ֵָ��
 * @retval  btl_rc_t
 **********************************************************************/
 
extern btl_rc_t btl_get_similarity_score(const PBL_TEMPLATE multi_templates, const PBL_TEMPLATE single_Template, unsigned short *score);

/***********************************************************************
 * @name    btl_decode_multiTemplates
 * @brief   �����ģ�壬������ģ������
 
 * @param   pblMultiTemplates: [in] ��ģ������
 * @param   pblSubTemplates: [out] ������ģ������
 * @param   nNbrOfSubtemplates: [out] ��ģ������
 * @retval  btl_rc_t
 **********************************************************************/
extern btl_rc_t btl_decode_multiTemplates(const PBL_TEMPLATE pblMultiTemplates, PBL_TEMPLATE** pblSubTemplates, unsigned char* nNbrOfSubtemplates);

/***********************************************************************
 * @name    btl_delete_subtemplates
 * @brief   �ͷ���ģ������ռ�õ���Դ
 
 * @param   pblSubTemplates: [in] ��ģ������
 * @param   nNbrOfSubtemplates: [in] ��ģ������
 * @retval  btl_rc_t
 **********************************************************************/
extern btl_rc_t btl_delete_subtemplates(PBL_TEMPLATE* pblSubTemplates, unsigned char nNbrOfSubtemplates);



/***********************************************************************
 * @name    btl_get_score_by_subtemplates
 * @brief   ��ģ�������Ƴ̶ȷ�
 
 * @param   pblSubTemplates:    [in] ��ģ������
 * @param   nNbrOfSubTemplates: [in] ��ģ������
 * @retval  pSimilarityScores:  [in/out] ����Ԫ�ظ�ΪnNbrOfSubTemplates�����飬���ڲ�������Ʒ�ֵ
 **********************************************************************/
extern btl_rc_t btl_get_score_by_subtemplates(PBL_TEMPLATE* pblSubTemplates, unsigned char nNbrOfSubTemplates,	unsigned short* pSimilarityScores);



/***********************************************************************
 * @name    btl_verify_speed
 * @brief   ����ģ�����ģ��ȶ�
 
 * @param   pblMultiTemplates:       [in] ��ģ������
 * @param   nNbrOfMultiTemplates:    [in] ��¼��ģ������	
 * @param   nFarAccepted:            [in] �ȶ԰�ȫ�ȼ�
 * @param   nMaxNbrOfSubTempaltes��  [in] ÿ����ģ���в���ȶԵ������ģ����
 * @param   pMatchIndex��            [out] ƥ��ɹ���ģ����ţ���0��ʼ��-1��ʾƥ��ʧ��
 * @retval  btl_rc_t
 **********************************************************************/
extern btl_rc_t  btl_verify_speed(
    const PBL_TEMPLATE* pblMultiTemplates,  //[in] ��¼���ģ������ 
    unsigned char nNbrOfMultiTemplates,     //[in] ��¼��ģ������	
    int nFarAccepted,                       //[in] FAR
    unsigned char  nMaxNbrOfSubTempaltes,   //[in] ÿ����ģ���в���ȶԵ������ģ����
    int* pMatchIndex                        //[out] ƥ��ɹ���ģ����ţ���0��ʼ��-1��ʾƥ��ʧ��
    );



/***********************************************************************
 * @name    btl_image_enhance
 * @brief   ͼ����ǿ

 * @retval  btl_rc_t
 **********************************************************************/
extern btl_rc_t btl_image_enhance(void);



/***********************************************************************
 * @name    btl_getBadLinePixelsNum
 * @brief   ��ȡ�������
 
 * @param   nBadPixelsNum:   [out] �������
 * @retval  int          ��  0��ʾ�ɹ���else��ʾʧ��
 **********************************************************************/			
extern int btl_getBadLinePixelsNum(int* nBadPixelsNum);  //[out]  �������



/***********************************************************************
 * @name    btl_computeSNR
 * @brief   ����ͼ���ź�����
 
 * @param   pSNR   :  [out] �����
 * @param   pSignal:  [out] �ź���
 * @param   pNoise :  [out] ������

 * @retval  int    �� 0��ʾ�ɹ���else��ʾʧ��
 **********************************************************************/
extern int btl_computeSNR( unsigned char* pSNR, unsigned char* pSignal, unsigned char* pNoise);




/***********************************************************************
 * @name    btl_get_cap
 * @brief  ��ȡͼ��ĵ���ֵ
 
 * @param   nCap   :  [out] ����ĵ���ֵ
 * @retval  btl_rc_t    �� BTL_RC_OK��ʾ�ɹ���else��ʾʧ��
 **********************************************************************/
extern btl_rc_t btl_get_cap(int* nCap);

/***********************************************************************
 * @name    btl_add_cap
 * @brief   ��ӵ���ֵ
 
 * @param   nCap   :  [in] ��Ҫ��ӵĵ���ֵ
 * @retval  btl_rc_t  �� BTL_RC_OK��ʾ�ɹ���else��ʾʧ��
 **********************************************************************/
extern btl_rc_t btl_add_cap(int nCap);

/***********************************************************************
 * @name    btl_finalize_cap
 * @brief   ��ȡ���ĵ���ֵ
 
 * @param   nMaxCap   :  [out] �����������ֵ
 * @retval  btl_rc_t  �� 0��ʾ�ɹ���else��ʾʧ��
 **********************************************************************/
extern btl_rc_t btl_finalize_cap(int* nMaxCap);



/***********************************************************************
 * @name    btl_force_image_without_agc
 * @brief   ǿ�Ʋ�ͼ������AGC����������ԭʼ���ݣ��������ڲ���������

 * @retval  btl_rc_t     �� BTL_RC_OK��ʾ�ɹ���else��ʾʧ��
 **********************************************************************/
extern btl_rc_t btl_force_image_without_agc(void);

/***********************************************************************
 * @name    btl_get_image_center_value
 * @brief   ��ȡͼ��48*48��������ĻҶȾ�ֵ�����������������
 * @param   nValue       :  [out]����ͼ��ҶȾ�ֵ
 * @retval  btl_rc_t     �� BTL_RC_OK��ʾ�ɹ���else��ʾʧ��
 **********************************************************************/
extern btl_rc_t btl_get_image_center_value(unsigned char* nValue);


/***********************************************************************
 * @name    btl_force_image_without_print
 * @brief   ǿ�ƶ�β�ͼ�����ϴ�ͼ�����ݣ���һ�廯ģ���������Բ�ͼ������
 
 * @param   times             :  [in] ��ͼ���������255��
 * @retval  unsigned char     �� ��ͼ����Ĵ���
 **********************************************************************/
extern unsigned char btl_force_image_without_print(unsigned char times);



/***********************************************************************
 * @name    btl_read_register
 * @brief   ��ȡĳ���Ĵ�����ֵ
 
 * @param   nRegAddr   :  [in] �Ĵ�����ַ
 * @param   pData      :  [out] ���ؼĴ���ֵ
 * @retval  btl_rc_t   �� BTL_RC_OK��ʾ�ɹ���else��ʾʧ��
 **********************************************************************/
extern btl_rc_t btl_read_register(unsigned char nRegAddr, unsigned char *pData);

/***********************************************************************
 * @name    btl_write_register
 * @brief   ����ĳ���Ĵ�����ֵ
 
 * @param   nRegAddr   :  [in] �Ĵ�����ַ
 * @param   nData      :  [in] �Ĵ���ֵ
 * @retval  btl_rc_t   �� BTL_RC_OK��ʾ�ɹ���else��ʾʧ��
 **********************************************************************/
extern btl_rc_t btl_write_register(unsigned char nRegAddr, unsigned char nData);

/***********************************************************************
 * @name    btl_imagePreprocessEx
 * @brief   ֧�ִ����ⲿͼ��Ԥ����ӿڣ���ԭʼͼ�����Ԥ������ͼ��ƽ����ͼ�������ȣ��˽ӿڲ���btl_get_config()->ImageEnhance����
 
 * @param   image   :  [in] ԭʼͼ������
 * @param   w       :  [in] ͼ���
 * @param   h       :  [in] ͼ���
 * @retval  int     �� 0��ʾ�ɹ���else��ʾʧ��
 **********************************************************************/
extern int (*btl_imagePreprocessEx)(unsigned char* image, int w, int h);

/***********************************************************************
 * @name    btl_get_base_image
 * @brief   �ɼ�BASEͼ�񣬹����ڲ�ͼ��BASEʹ��
 * @param   nDevThreshold     :  [in] ͼ�񷽲���ֵ���Ƽ�ֵ300
 * @param   nSnrThreshold     :  [in] ͼ���������ֵ���Ƽ�ֵ10
 * @param   nSignalThreshold  :  [in] ͼ���ź�����ֵ���Ƽ�ֵ20
 * @retval  btl_rc_t     
 ������BTL_RC_OK֮��ͨ��btl_get_image_data()��btl_get_image_width()��btl_get_image_height()�ӿڻ�ȡͼ������
 **********************************************************************/
extern btl_rc_t  btl_get_base_image(int nDevThreshold, unsigned char nSnrThreshold, unsigned char nSignalThreshold);

/***********************************************************************
 * @name    btl_minus_base_image
 * @brief   ԭʼͼ��ȥ����

 * @param   srcImg     :  [in] ԭʼͼ������
 * @param   pImageBase :  [in] BASEͼ��
 * @param   nWidth     :  [in] ͼ���
 * @param   nHeight    :  [in] ͼ���
 * @param   nBaseMean  :  [in] BASEͼ��ĻҶȾ�ֵ������0�����ڲ�����
 * @retval  btl_rc_t   ��
 **********************************************************************/
extern btl_rc_t btl_minus_base_image(unsigned char* srcImg, const unsigned char* pImageBase,  int nWidth, int nHeight, unsigned char nBaseMean);

/***********************************************************************
 * @name    blt_sensor_connected_status_test
 * @brief   Sensor����״̬��飬����SPI��RST��INT��Chip ID����

 * @retval  btl_scs_t   ��
 **********************************************************************/
extern btl_scs_t blt_sensor_connected_status_test(void);



/***********************************************************************
 * @name    btl_disable_finger_up_once
 * @brief   ����ȡ����ָ̧����

 * @retval  void
 **********************************************************************/
extern void btl_disable_finger_up_once(void);


/***********************************************************************
 * @name    btl_update_definite_templates
 * @brief   update definited template into multitamplate

 * @param   pblTemplate            :  [in] the new definited template
 * @param   pblMultiTemplates      :  [in] the old multitamplates
 * @param   pblUpdateMultiTemplates:  [out] the outcome
 * @retval  btl_rc_t   ��
 **********************************************************************/

extern int btl_update_definite_templates(const PBL_TEMPLATE pblTemplate, const PBL_TEMPLATE pblMultiTemplates, PBL_TEMPLATE pblUpdateMultiTemplates);

#endif


























