#ifndef _BTL_BEP_H
#define _BTL_BEP_H 



#define VERSION_NUMBER "V3.2.0" //20210827

/*
SPI д
*/
typedef int (*SpiSendByteFuncPtr)(unsigned char nByte);
/*
SPI ��
*/
typedef int (*SpiRevByteFuncPtr)(unsigned char *rx_buffer,unsigned int num);
/*
RST �ſ���
*/	
typedef void (*RstnSetFuncPtr)(unsigned char RST_LEVEL);
/*
CS ����
*/
typedef void (*SpiCSSetFuncPtr)(unsigned char CS_LEVEL); 
/*
��ʱ����
*/
typedef void (*DelayFuncPtr)(unsigned int times);

#ifndef BTL_ALGO_STRUCT
#define BTL_ALGO_STRUCT
/*
��ȫ�ȼ�ö��
*/
typedef enum far_level_e
{
	BL_FAR_1 = 0,
	BL_FAR_2 = 1,
	BL_FAR_5 = 2,
	BL_FAR_10 = 3,
	BL_FAR_20 = 4,
	BL_FAR_50 = 5,
	BL_FAR_100 = 6,
	BL_FAR_200 = 7,
	BL_FAR_500 = 8,
	BL_FAR_1000 = 9,
	BL_FAR_2K = 10,
	BL_FAR_5000 = 11,
	BL_FAR_10000 = 12,
	BL_FAR_20K = 13,
	BL_FAR_50000 = 14,
	BL_FAR_100000 = 15,
	BL_FAR_200K = 16,
	BL_FAR_500000 = 17,
	BL_FAR_1000000 = 18,
	BL_FAR_2M = 19,
	BL_FAR_5M = 20,
	BL_FAR_10M = 21,
	BL_FAR_20M = 22,
	BL_FAR_50M = 23,
	BL_FAR_100M = 24,
	BL_FAR_200M = 25,
	BL_FAR_500M = 26,
	BL_FAR_1000M = 27,
	BL_FAR_Inf = 28
} far_level_t;

/*
ָ��ģ��ṹ��
�˽ṹ��Ķ���ı���һ��Ҫ��ʼ��Ϊ0,���� BL_TEMPLATE a={0};
*/
typedef struct BL_TEMPLATE_tag    
{
	unsigned char* pTemplateData;      //ģ������
	unsigned int   templateSize;       //ģ�����ݵĴ�С
  unsigned char  templateType;       //ģ������
}BL_TEMPLATE, *PBL_TEMPLATE;


/*
��չָ��ģ��ṹ�壬֧�ָ�ÿ��ģ��ָ����ͬ�İ�ȫ�ȼ�
*/
typedef struct BL_TEMPLATE_EX_tag
{
	int nFarAccepted;                //��ȫ�ȼ�
	PBL_TEMPLATE pBlTemplate;        //ģ������
}BL_TEMPLATE_EX, *PBL_TEMPLATE_EX;

#endif


/*
����ģʽö��
*/
typedef enum 
{
	BTL_IDLE_MODE = 0,              //����ģʽ
	BTL_ENROLLMENT_MODE = 1,        //ע��ģʽ
	BTL_VERIFY_MODE = 2,            //ƥ��ģʽ
	BTL_GET_IMAGE_MODE = 3,         //��ͼģʽ     
	BTL_FINGER_DETECT_MODE = 4,     //��ָ���ģʽ
} btl_mode_t;   


/*
������ö��
*/ 
typedef enum btl_bep_rc_e{
	BTL_RC_OK = 0x00, 					  							// �ɹ� 
	BTL_RC_FINGER_NO_UP = 0x01,     	  				// ��ָû̧��  
	BTL_RC_FINGER_DETECT_ERR = 0x02,     			  // δ��⵽��ָ��ѹ
	BTL_RC_ENROLLMENT_ERR = 0x03,        			  // ע��ʧ��  
	BTL_RC_VERIFY_FAIL = 0x04,           			  // ƥ��ʧ�� 	
	BTL_RC_GET_IMAGE_FAIL = 0x05,        			  // �ɼ�ͼ��ʧ�� 
	BTL_RC_IMAGE_NG = 0x06,    		      			  // ͼƬ������  		
	BTL_RC_SENSOR_INIT_ERR = 0x07,       			  // sensor��ʼ��ʧ��
	BTL_RC_EXTRACT_TEMPLATE_FAIL = 0x08, 			  // ��ȡģ��ʧ��
	BTL_RC_CREATE_MULTI_TEMPLATE_FAIL = 0x09, 	// �ϳɶ�ģ��ʧ��
	BTL_RC_IMAGE_AREA_ERR = 0x0A,	      			  // ͼ����Ч���̫С
	BTL_RC_IMAGE_DRY_ERR = 0x0B,		      			// ��ָ̫��
	BTL_RC_IMAGE_HUMIDITY_ERR = 0x0C,    			  // ��ָ̫ʪ
	BTL_RC_DYNAMIC_UPDATE_ERR = 0x0D,    			  // ģ�嶯̬����ʧ��
	BTL_RC_SENSOR_ID_ERR = 0x0E,         			  // sensor id ����
	BTL_RC_MALLOC_ERR = 0x0F,            			  // �ڴ�����ʧ��
	BTL_RC_STATIC_PATTERN_ERR = 0x10,      		  // sensor ���о�̬����
	BTL_RC_CRACK_ERR = 0x11,               		  // sensor ��������
	BTL_RC_MULTI_FINGERS = 0x12,           		  // ͬһ��ģ��¼������ָ
	BTL_RC_RESET_ERR = 0xFC,      			        // sensor �쳣��λ
	BTL_RC_IMAGE_STATUS_ERR = 0xFD,  		        // ��ͼ��־����
	BTL_RC_DEVICE_ERR = 0xFE,      			        // �豸����
	BTL_RC_PARAM_ERROR = 0xFF,              		// ��������
	BTL_RC_GENERAL_ERROR = 0x100,          			// һ���Դ���
	BTL_RC_NOT_SUPPORT = 0x101,             		// ��֧�ָò���
	BTL_RC_METERIALS_ERROR = 0x102              // �ϺŴ���
} btl_rc_t;



/*
��ʼ�������ṹ�� 
*/
typedef struct{   	
	RstnSetFuncPtr rstn_set;  				// sensor��λ
	SpiCSSetFuncPtr spi_cs_set; 			// SPI CS
	SpiSendByteFuncPtr spi_send_byte; // SPI�ֽڷ���
	SpiRevByteFuncPtr spi_rev_byte; 	// SPI�ֽڽ���  
	DelayFuncPtr delay_ms;  	        // ������ʱ
	DelayFuncPtr delay_us;  	        // ΢����ʱ
	unsigned char *pImage; 		        // ���ڴ��ͼ�����ݵĻ�������ַ���������NULL������SDK�ڲ��Զ�����
}BEP_INIT_PARAM;


/*
ͼ����������
*/
typedef struct{
	unsigned char nQuality;  //ͼ����������Χ[0,100]��0��ʾ��100��ʾ���     
	unsigned char nArea;     //��Ч��ٷֱȣ���Χ[0,100]
	unsigned char nConditon; //��ʪ�̶ȣ���Χ[0,100]��50Ϊ��ѣ���С��ʾ��ָ��ʪ�������ʾ��ָ�ϸ�
}BTL_IMAGE_QUALITY;


/*
ͼ��������ֵ�ṹ��
*/
typedef struct {
	unsigned char min_quality;	  //���ͼ��������ֵ
	unsigned char min_area;  	    //���ͼ�������ֵ
	unsigned char min_condition;	//��͸�ʪ����ֵ
	unsigned char max_condition;  //��߸�ʪ����ֵ
}BTL_IMAGE_QUALITY_CTRL;

/*
FD �������ã�Ĭ����200ms
*/
typedef enum
{
	BTL_100MS_3_3V,
	BTL_200MS_3_3V,
	BTL_300MS_3_3V,
}BTL_FREQUENCE_PARAM;


/*
�ɶ�̬����������
*/
typedef struct {
	int nFarAccepted;  								               	// �ȶ԰�ȫ�ȼ�
	int update_FAR_level;                             // ����ģ��ʱ�ٴαȶԵİ�ȫ�ȼ�
	int nEnrollCounts;  							              	// ¼�����
	int finger_detect_preriod;                        // ��ָ�������
	BTL_IMAGE_QUALITY_CTRL image_quality_for_enroll; 	// ע��ʱͼ����������
	BTL_IMAGE_QUALITY_CTRL image_quality_for_verify; 	// ƥ��ͳ�ȡģ��ʱͼ����������
	int max_crack_pixels;                             // ����������ؿ��أ�Ĭ��ֵΪ0
	unsigned short nVerifyTimeOut;                    // �ȶԳ�ʱʱ�䣬��λ����  
	unsigned char bForbidSamePos;                     // �Ƿ��ֹ��ͬλ��¼�룬0��ʾ����ֹ��1��ʾ��ֹ��Ĭ��Ϊ��ֹ
	unsigned char FingerUpdetect;                     // ��ָ̧���⹦�ܿ��ƣ�0��ʾ�رգ�1��ʾ�򿪣�Ĭ��1����
	unsigned char ImageEnhance;                       // ͼ����ǿ���ܿ��أ�0��ʾ�رգ�1��ʾ��
	unsigned char Dacp_min;                           // �����ȵ�����Сֵ
	unsigned char Dacp_max;                           // �����ȵ������ֵ
	unsigned char image_modify_control;               // bit0��ͼ���޸����أ�1��ʾ��; bit1��ͼ��ƽ�����أ�1��ʾ��
	unsigned char AGC_controler;                      // �����Զ����ڿ������� 0��ʾ�رգ� else��ʾ��
	unsigned char re_image_cnt;                       // ͼ�񲻺ϸ�ʱ���Բ�ͼ����
	unsigned char static_pattern_score;               // ��⾲̬���￨�ط�����Ĭ��ֵΪ0,��Χ[0,100]
  unsigned char agc_aim_mean;                       // AGCĿ��ҶȾ�ֵ
} BTL_CONFIG; 

/*
����оƬ����
*/
typedef enum
{
	BTL_DRIVER_UNKNOWN = 0,
	BTL_DRIVER_NONE, 
	BTL_DRIVER_BF2080, 
	BTL_DRIVER_BF3080    
} btl_driver_type_t;  
/*
��ȡ����оƬ����
*/
extern btl_driver_type_t btl_get_driver_type(void);

/*
��ȡ�ɵ������ṹ��ָ�룬������SDK��ʼ���ɹ�֮���ȡ����Ч,��������ʱ����
*/   	
extern BTL_CONFIG* btl_get_config(void);


/*
�����㷨��SENSOR��������ʼ��ʱ����
*/
typedef struct 
{
	//ģ�������ֽ���
	unsigned int max_template_size;    
	//�Ƿ�����¼���ģ�岻���滻���£�1��ʾ������0��ʾ��������Ĭ��Ϊ0(V3.0.3��֮ǰ�汾Ĭ��Ϊ1)
	int	lock_templates_after_enroll;   
	//�Ƿ�֧��360�ȱȶԣ�1��ʾ֧��, 0��ʾ��֧��
	unsigned char support_360rotate;    
	//��ֹ��ͬ��ָע���ͬһ��ID, Ĭ��Ϊ0
		//0:����ֹ,Ҳ�����عµ���  
		//1:��ֹ��ͬ��ָ¼��,�ҷ��عµ���   
		//2:���µ�¼��,�ҷ��عµ���    
		//3:����ֹ����ָ¼��,�����عµ���  	
	unsigned char prevent_enroll_multifingers;  
	//ע��ʱ�ж�Ϊ��ͬλ�õ���Сƫ����
	unsigned char duplicate_min_distance; 
	//�����ȵ�������
	unsigned char dacp_val;   
	//ͼ��Աȶȵ�������1
	unsigned char reg31_val;       
	//ͼ��Աȶȵ�������2
	unsigned char reg32_val;                    
}BTL_PARAM; 

	
/*
��ȡsensor ID
*/
extern unsigned int btl_get_sensor_id(void);
/*
��ȡSDK�汾��
*/
extern char* btl_get_version(void);		
/*
sensor�жϴ���,�û���Ҫ���˺����ŵ�ָ��ģ���INT���жϴ�������
*/		
extern void btl_sensor_int_handle(void);
/*
1���붨ʱ�����û���Ҫ���˺����ŵ�1���붨ʱ����
Ҳ���Զ���n(n<=10)���붨ʱ����Ȼ���ڶ�ʱ�жϷ�������е��ôκ���n��
*/
extern void btl_timer_ms_handle(void); 
/*
��ʼ��SDK
*/
extern btl_rc_t btl_bep_init(
		BEP_INIT_PARAM	*InitParam             //[in] ��ѡ��ʼ������
		);
/*
��Դ�ͷţ���MCU�ǵ��硢RAM���ܱ��ֵ��������Ҫ�õ�������ǰ�ͷ���Դ
*/
extern void btl_bep_uninit(void);


	
/* 
��ȡָ�ƴ��������㷨��ز���
*/
extern BTL_PARAM btl_get_param(void);
/* 
�������ú���ָ�룬Ϊ��ͳһ�ӿڶ�������ʹ�÷�������
btl_set_param = btl_set_param_hybrid;
*/
extern int (*btl_set_param)(const BTL_PARAM *); 

/*
����ָ�ƴ��������㷨��ز���
*/
// �ڴ���Դ����ϵͣ�ģ����ȡ������ģ��������죬�����ʽϵ�
extern int btl_set_param_hybrid(const BTL_PARAM *param); 
//�ڴ���Դ�������ģ����ȡ�Ͽ죬ģ�������Ͽ죬���������
extern int btl_set_param_cardo(const BTL_PARAM *param);   
// �ڴ���Դ������ͣ�ģ����ȡ��죬ģ���������������������
extern int btl_set_param_spectral(const BTL_PARAM *param); 


/*
�����Ƿ���360�ȱȶԣ���btl_set_param֮����ò���Ч
*/
extern int btl_set_feature_360(
	unsigned char bOpen360        //1��ʾ������0��ʾ�ر�
		);
	
/*
��ȡ��ǰ����ģʽ
*/
extern btl_mode_t btl_get_work_mode(void);

/*
���ù���ģʽ
����ֵ��
BTL_RC_OK ���� ��ʾ���óɹ�
other ���� ��ʾ����ʧ��
*/
extern  btl_rc_t btl_set_work_mode(btl_mode_t mode);

/*
�����ָ��ѹ����ͼ��Ԥ����ȣ��½ӿڣ�����btl_set_work_modeʹ�ã�����ɵĽӿ�btl_finger_detect
����������£�
1����Ҫ�л�ģʽ�ĵط���ʹ��btl_set_work_mode�滻btl_finger_detect
2����Ҫ�����ָ�Ͳ�ͼ�ĵط���ʹ��btl_finger_detect_ex�滻btl_finger_detect
����ֵ��
	BTL_IDLE_MODE
		���Ƿ��� BTL_RC_FINGER_DETECT_ERR
	BTL_ENROLLMENT_MODE
	BTL_VERIFY_MODE
	BTL_GET_IMAGE_MODE
		���� BTL_RC_OK ��ʾ�ɼ����ϸ�ͼ��
	BTL_FINGER_DETECT_MODE
	  ����BTL_RC_OK ��ʾ��⵽��ָ��ѹ
		����BTL_RC_FINGER_DETECT_ERR ��ʾδ��⵽��ָ
*/
extern btl_rc_t btl_finger_detect_ex(void);

/*
����ģʽ�л��������ָ��ѹ����ͼ��Ԥ����ȣ��ɽӿ�
*/
extern btl_rc_t btl_finger_detect(btl_mode_t mode);

/*
ע��ָ�ƣ����ջ�ȡ��ָ��������Ҫ�û������ͷ�
*/
extern btl_rc_t btl_enrollment(
		int* pAcceptedNum,                 //[out] ��¼������������������OK�����Ǵ���û�����ӣ����ʾ��ѹλ���ظ�
		int* island_num,                   //[out] �µ������Ǳ��룬ֻ��prevent_enroll_multifingersΪ��0��ʱ��Ż᷵��
		PBL_TEMPLATE pbtlMultiTemplates    //[out] ע�����ģ��,��pAcceptedNum�������õ�¼�����ʱ����Ч��ע�⣺�û�һ��Ҫ��ʱ�ͷ�
		);	

/*
ƥ�䣬ʹ��ͳһ�İ�ȫ�ȼ�
*/
extern btl_rc_t btl_verify(
		PBL_TEMPLATE blMultiTemplatesArray[],	//[in] ��ƥ��ģ������
		int nNbrOfMultiTemplates,             //[in] �����С
		int *pMatchIndex  									  //[out] ƥ��ɹ���ģ����� 		
		); 

/*
��ȡģ�壬�����Ƿ���ģ��ĵ�ַ��ģ�建����SDK�ڲ������û���Ҫȥ�ͷ�
*/		
extern btl_rc_t btl_extract_template(
		PBL_TEMPLATE* ppbtlTemplate              //[out] ���س�ȡ����ģ���ַ���û�����Ҫ�ͷŴ�ģ�壬����Ҫ���ݣ������������뻺�濽����ȥ
		);

/*
ʹ���Ѿ����ڵĶ����ģ��ϳ�һ����ģ�壬�ϳ�ģ����Ҫ�û������ͷ�
*/		
extern btl_rc_t btl_create_multitemplate(
		const PBL_TEMPLATE pblTemplates[],   //[in] ���ϲ�ָ����ģ��
		unsigned char nNbrOfTemplates,       //[in] ���ϲ�ָ����ģ������
		int *island_num,                     //[out] �ϲ���Ĺµ�������2���µ�����ʾ����������û�����ӵ�һ��
		PBL_TEMPLATE pblMultiTemplates);     //[out] �ϲ���Ķ�ģ�����飬ע�⣺�û�һ��Ҫ��ʱ�ͷţ�������������ڴ���ҵ�δ֪����


/*
��̬����
ģ��ȶԳɹ�֮�󣬵��ô˺�������ģ����£�
����������سɹ�����õ����º��ģ�壬
��ģ���滻Flash�еľ�ģ�壬Ȼ���û���Ҫ�ͷ�
*/	
extern btl_rc_t btl_dynamic_update_templates(
		const PBL_TEMPLATE pblMultiTemplates,          //[in] ������ָ��ģ��
		PBL_TEMPLATE pblUpdateMultiTemplates           //[out]���º��ָ��ģ�壬���³ɹ����ɴ���flash�У���Ҫ�û��ͷ�
		);
		
	
/*
�ͷ�ָ��ģ��������ռ���� 
*/
extern void btl_delete_template(
	PBL_TEMPLATE pbtlTemplate          //[in] ���ͷŵ�ָ��ģ��
);


/*
��ȡͼ�����ݽӿڣ�����ͼ������buffer��ַ
*/  
extern unsigned char* btl_get_image_data(void);

/*
��ȡͼ��Ŀ��
*/   
extern int btl_get_image_width(void);

/*
��ȡͼ��ĸ߶�
*/   
extern int btl_get_image_height(void);

/*
��ȡͼ������
*/
extern BTL_IMAGE_QUALITY btl_get_image_quality(void);

/*
������С����sensor(��BF81060)��ʹ��cardo�㷨��ͼ����ǿ�ӿڣ�ʹ�÷�������
btl_Customized_imageEnhance_pointer = btl_cardo_ImageEnhance;
���� btl_get_config()->ImageEnhance ����
*/
extern int btl_cardo_ImageEnhance(unsigned char*image, int w, int h);
/*
֧�ִ����ⲿͼ����ǿ�ӿڣ��滻�ڲ���ͼ����ǿ
*/
extern int (*btl_Customized_imageEnhance_pointer)(unsigned char*, int, int);



#endif
