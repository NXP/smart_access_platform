#ifndef _BTL_BEP_H
#define _BTL_BEP_H 



#define VERSION_NUMBER "V3.2.0" //20210827

/*
SPI 写
*/
typedef int (*SpiSendByteFuncPtr)(unsigned char nByte);
/*
SPI 读
*/
typedef int (*SpiRevByteFuncPtr)(unsigned char *rx_buffer,unsigned int num);
/*
RST 脚控制
*/	
typedef void (*RstnSetFuncPtr)(unsigned char RST_LEVEL);
/*
CS 控制
*/
typedef void (*SpiCSSetFuncPtr)(unsigned char CS_LEVEL); 
/*
延时函数
*/
typedef void (*DelayFuncPtr)(unsigned int times);

#ifndef BTL_ALGO_STRUCT
#define BTL_ALGO_STRUCT
/*
安全等级枚举
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
指纹模板结构体
此结构体的定义的变量一定要初始化为0,例如 BL_TEMPLATE a={0};
*/
typedef struct BL_TEMPLATE_tag    
{
	unsigned char* pTemplateData;      //模板数据
	unsigned int   templateSize;       //模板数据的大小
  unsigned char  templateType;       //模板类型
}BL_TEMPLATE, *PBL_TEMPLATE;


/*
拓展指纹模板结构体，支持给每个模板指定不同的安全等级
*/
typedef struct BL_TEMPLATE_EX_tag
{
	int nFarAccepted;                //安全等级
	PBL_TEMPLATE pBlTemplate;        //模板数据
}BL_TEMPLATE_EX, *PBL_TEMPLATE_EX;

#endif


/*
工作模式枚举
*/
typedef enum 
{
	BTL_IDLE_MODE = 0,              //空闲模式
	BTL_ENROLLMENT_MODE = 1,        //注册模式
	BTL_VERIFY_MODE = 2,            //匹配模式
	BTL_GET_IMAGE_MODE = 3,         //采图模式     
	BTL_FINGER_DETECT_MODE = 4,     //手指检测模式
} btl_mode_t;   


/*
错误码枚举
*/ 
typedef enum btl_bep_rc_e{
	BTL_RC_OK = 0x00, 					  							// 成功 
	BTL_RC_FINGER_NO_UP = 0x01,     	  				// 手指没抬起  
	BTL_RC_FINGER_DETECT_ERR = 0x02,     			  // 未检测到手指按压
	BTL_RC_ENROLLMENT_ERR = 0x03,        			  // 注册失败  
	BTL_RC_VERIFY_FAIL = 0x04,           			  // 匹配失败 	
	BTL_RC_GET_IMAGE_FAIL = 0x05,        			  // 采集图像失败 
	BTL_RC_IMAGE_NG = 0x06,    		      			  // 图片质量差  		
	BTL_RC_SENSOR_INIT_ERR = 0x07,       			  // sensor初始化失败
	BTL_RC_EXTRACT_TEMPLATE_FAIL = 0x08, 			  // 抽取模板失败
	BTL_RC_CREATE_MULTI_TEMPLATE_FAIL = 0x09, 	// 合成多模板失败
	BTL_RC_IMAGE_AREA_ERR = 0x0A,	      			  // 图像有效面积太小
	BTL_RC_IMAGE_DRY_ERR = 0x0B,		      			// 手指太干
	BTL_RC_IMAGE_HUMIDITY_ERR = 0x0C,    			  // 手指太湿
	BTL_RC_DYNAMIC_UPDATE_ERR = 0x0D,    			  // 模板动态更新失败
	BTL_RC_SENSOR_ID_ERR = 0x0E,         			  // sensor id 错误
	BTL_RC_MALLOC_ERR = 0x0F,            			  // 内存申请失败
	BTL_RC_STATIC_PATTERN_ERR = 0x10,      		  // sensor 上有静态异物
	BTL_RC_CRACK_ERR = 0x11,               		  // sensor 上有裂纹
	BTL_RC_MULTI_FINGERS = 0x12,           		  // 同一个模板录入多个手指
	BTL_RC_RESET_ERR = 0xFC,      			        // sensor 异常复位
	BTL_RC_IMAGE_STATUS_ERR = 0xFD,  		        // 采图标志出错
	BTL_RC_DEVICE_ERR = 0xFE,      			        // 设备故障
	BTL_RC_PARAM_ERROR = 0xFF,              		// 参数错误
	BTL_RC_GENERAL_ERROR = 0x100,          			// 一般性错误
	BTL_RC_NOT_SUPPORT = 0x101,             		// 不支持该操作
	BTL_RC_METERIALS_ERROR = 0x102              // 料号错误
} btl_rc_t;



/*
初始化参数结构体 
*/
typedef struct{   	
	RstnSetFuncPtr rstn_set;  				// sensor复位
	SpiCSSetFuncPtr spi_cs_set; 			// SPI CS
	SpiSendByteFuncPtr spi_send_byte; // SPI字节发送
	SpiRevByteFuncPtr spi_rev_byte; 	// SPI字节接收  
	DelayFuncPtr delay_ms;  	        // 毫秒延时
	DelayFuncPtr delay_us;  	        // 微秒延时
	unsigned char *pImage; 		        // 用于存放图像数据的缓冲区地址，如果传入NULL，则由SDK内部自动申请
}BEP_INIT_PARAM;


/*
图像质量参数
*/
typedef struct{
	unsigned char nQuality;  //图像质量，范围[0,100]，0表示最差，100表示最好     
	unsigned char nArea;     //有效面百分比，范围[0,100]
	unsigned char nConditon; //干湿程度，范围[0,100]，50为最佳，往小表示手指较湿，往大表示手指较干
}BTL_IMAGE_QUALITY;


/*
图像质量阈值结构体
*/
typedef struct {
	unsigned char min_quality;	  //最低图像质量阈值
	unsigned char min_area;  	    //最低图像面积阈值
	unsigned char min_condition;	//最低干湿度阈值
	unsigned char max_condition;  //最高干湿度阈值
}BTL_IMAGE_QUALITY_CTRL;

/*
FD 周期设置，默认是200ms
*/
typedef enum
{
	BTL_100MS_3_3V,
	BTL_200MS_3_3V,
	BTL_300MS_3_3V,
}BTL_FREQUENCE_PARAM;


/*
可动态调整的配置
*/
typedef struct {
	int nFarAccepted;  								               	// 比对安全等级
	int update_FAR_level;                             // 更新模板时再次比对的安全等级
	int nEnrollCounts;  							              	// 录入次数
	int finger_detect_preriod;                        // 手指检测周期
	BTL_IMAGE_QUALITY_CTRL image_quality_for_enroll; 	// 注册时图像质量控制
	BTL_IMAGE_QUALITY_CTRL image_quality_for_verify; 	// 匹配和抽取模板时图像质量控制
	int max_crack_pixels;                             // 检测裂纹像素卡控，默认值为0
	unsigned short nVerifyTimeOut;                    // 比对超时时间，单位毫秒  
	unsigned char bForbidSamePos;                     // 是否禁止相同位置录入，0表示不禁止，1表示禁止，默认为禁止
	unsigned char FingerUpdetect;                     // 手指抬起检测功能控制，0表示关闭，1表示打开，默认1：打开
	unsigned char ImageEnhance;                       // 图像增强功能开关，0表示关闭，1表示打开
	unsigned char Dacp_min;                           // 灵敏度调整最小值
	unsigned char Dacp_max;                           // 灵敏度调整最大值
	unsigned char image_modify_control;               // bit0，图像修复开关，1表示打开; bit1，图像平滑开关，1表示打开
	unsigned char AGC_controler;                      // 增益自动调节控制器， 0表示关闭， else表示打开
	unsigned char re_image_cnt;                       // 图像不合格时尝试采图次数
	unsigned char static_pattern_score;               // 检测静态异物卡控分数，默认值为0,范围[0,100]
  unsigned char agc_aim_mean;                       // AGC目标灰度均值
} BTL_CONFIG; 

/*
驱动芯片类型
*/
typedef enum
{
	BTL_DRIVER_UNKNOWN = 0,
	BTL_DRIVER_NONE, 
	BTL_DRIVER_BF2080, 
	BTL_DRIVER_BF3080    
} btl_driver_type_t;  
/*
获取驱动芯片类型
*/
extern btl_driver_type_t btl_get_driver_type(void);

/*
获取可调参数结构体指针，必须在SDK初始化成功之后获取才有效,参数可随时调整
*/   	
extern BTL_CONFIG* btl_get_config(void);


/*
部分算法和SENSOR参数，初始化时调整
*/
typedef struct 
{
	//模板的最大字节数
	unsigned int max_template_size;    
	//是否锁定录入的模板不被替换更新，1表示锁定，0表示不锁定，默认为0(V3.0.3及之前版本默认为1)
	int	lock_templates_after_enroll;   
	//是否支持360度比对，1表示支持, 0表示不支持
	unsigned char support_360rotate;    
	//防止不同手指注册进同一个ID, 默认为0
		//0:不禁止,也不返回孤岛数  
		//1:禁止不同手指录入,且返回孤岛数   
		//2:最大孤岛录入,且返回孤岛数    
		//3:不禁止多手指录入,但返回孤岛数  	
	unsigned char prevent_enroll_multifingers;  
	//注册时判定为相同位置的最小偏移量
	unsigned char duplicate_min_distance; 
	//灵敏度调整参数
	unsigned char dacp_val;   
	//图像对比度调整参数1
	unsigned char reg31_val;       
	//图像对比度调整参数2
	unsigned char reg32_val;                    
}BTL_PARAM; 

	
/*
获取sensor ID
*/
extern unsigned int btl_get_sensor_id(void);
/*
获取SDK版本号
*/
extern char* btl_get_version(void);		
/*
sensor中断处理,用户需要将此函数放到指纹模组的INT脚中断处理函数中
*/		
extern void btl_sensor_int_handle(void);
/*
1毫秒定时处理，用户需要将此函数放到1毫秒定时器中
也可以定义n(n<=10)毫秒定时器，然后在定时中断服务程序中调用次函数n次
*/
extern void btl_timer_ms_handle(void); 
/*
初始化SDK
*/
extern btl_rc_t btl_bep_init(
		BEP_INIT_PARAM	*InitParam             //[in] 必选初始化参数
		);
/*
资源释放，在MCU非掉电、RAM不能保持的情况下需要用到，休眠前释放资源
*/
extern void btl_bep_uninit(void);


	
/* 
获取指纹传感器与算法相关参数
*/
extern BTL_PARAM btl_get_param(void);
/* 
参数设置函数指针，为了统一接口而创建，使用方法如下
btl_set_param = btl_set_param_hybrid;
*/
extern int (*btl_set_param)(const BTL_PARAM *); 

/*
配置指纹传感器与算法相关参数
*/
// 内存资源需求较低，模板提取最慢，模板搜索最快，拒真率较低
extern int btl_set_param_hybrid(const BTL_PARAM *param); 
//内存资源需求最大，模板提取较快，模板搜索较快，拒真率最低
extern int btl_set_param_cardo(const BTL_PARAM *param);   
// 内存资源需求最低，模板提取最快，模板搜索最慢，拒真率最高
extern int btl_set_param_spectral(const BTL_PARAM *param); 


/*
设置是否开启360度比对，在btl_set_param之后调用才有效
*/
extern int btl_set_feature_360(
	unsigned char bOpen360        //1表示开启，0表示关闭
		);
	
/*
获取当前工作模式
*/
extern btl_mode_t btl_get_work_mode(void);

/*
设置工作模式
返回值：
BTL_RC_OK —— 表示设置成功
other —— 表示设置失败
*/
extern  btl_rc_t btl_set_work_mode(btl_mode_t mode);

/*
检测手指按压、采图、预处理等，新接口，搭配btl_set_work_mode使用，替代旧的接口btl_finger_detect
替代方案如下：
1、需要切换模式的地方，使用btl_set_work_mode替换btl_finger_detect
2、需要检测手指和采图的地方，使用btl_finger_detect_ex替换btl_finger_detect
返回值：
	BTL_IDLE_MODE
		总是返回 BTL_RC_FINGER_DETECT_ERR
	BTL_ENROLLMENT_MODE
	BTL_VERIFY_MODE
	BTL_GET_IMAGE_MODE
		返回 BTL_RC_OK 表示采集到合格图像
	BTL_FINGER_DETECT_MODE
	  返回BTL_RC_OK 表示检测到手指按压
		返回BTL_RC_FINGER_DETECT_ERR 表示未检测到手指
*/
extern btl_rc_t btl_finger_detect_ex(void);

/*
工作模式切换、检测手指按压、采图、预处理等，旧接口
*/
extern btl_rc_t btl_finger_detect(btl_mode_t mode);

/*
注册指纹，最终获取的指纹数据需要用户主动释放
*/
extern btl_rc_t btl_enrollment(
		int* pAcceptedNum,                 //[out] 已录入次数，如果函数返回OK，但是次数没有增加，则表示按压位置重复
		int* island_num,                   //[out] 孤岛数，非必须，只有prevent_enroll_multifingers为非0的时候才会返回
		PBL_TEMPLATE pbtlMultiTemplates    //[out] 注册完成模板,当pAcceptedNum等于设置的录入次数时才有效，注意：用户一定要及时释放
		);	

/*
匹配，使用统一的安全等级
*/
extern btl_rc_t btl_verify(
		PBL_TEMPLATE blMultiTemplatesArray[],	//[in] 待匹配模板数组
		int nNbrOfMultiTemplates,             //[in] 数组大小
		int *pMatchIndex  									  //[out] 匹配成功的模板序号 		
		); 

/*
抽取模板，仅仅是返回模板的地址，模板缓存由SDK内部管理，用户不要去释放
*/		
extern btl_rc_t btl_extract_template(
		PBL_TEMPLATE* ppbtlTemplate              //[out] 返回抽取到的模板地址，用户不需要释放此模板，如需要备份，则需另外申请缓存拷贝过去
		);

/*
使用已经存在的多个子模板合成一个多模板，合成模板需要用户主动释放
*/		
extern btl_rc_t btl_create_multitemplate(
		const PBL_TEMPLATE pblTemplates[],   //[in] 待合并指纹子模板
		unsigned char nNbrOfTemplates,       //[in] 待合并指纹子模板数量
		int *island_num,                     //[out] 合并后的孤岛个数，2个孤岛即表示有两块区域没有连接到一起
		PBL_TEMPLATE pblMultiTemplates);     //[out] 合并后的多模板数组，注意：用户一定要及时释放，否则可能引起内存错乱等未知错误。


/*
动态更新
模板比对成功之后，调用此函数进行模板更新，
如果函数返回成功，则得到更新后的模板，
此模板替换Flash中的旧模板，然后用户需要释放
*/	
extern btl_rc_t btl_dynamic_update_templates(
		const PBL_TEMPLATE pblMultiTemplates,          //[in] 待更新指纹模板
		PBL_TEMPLATE pblUpdateMultiTemplates           //[out]更新后的指纹模板，更新成功即可存入flash中，需要用户释放
		);
		
	
/*
释放指纹模板数据所占缓存 
*/
extern void btl_delete_template(
	PBL_TEMPLATE pbtlTemplate          //[in] 待释放的指纹模板
);


/*
获取图像数据接口，返回图像数据buffer地址
*/  
extern unsigned char* btl_get_image_data(void);

/*
获取图像的宽度
*/   
extern int btl_get_image_width(void);

/*
获取图像的高度
*/   
extern int btl_get_image_height(void);

/*
获取图像质量
*/
extern BTL_IMAGE_QUALITY btl_get_image_quality(void);

/*
适用于小面阵sensor(如BF81060)，使用cardo算法的图像增强接口，使用方法如下
btl_Customized_imageEnhance_pointer = btl_cardo_ImageEnhance;
不受 btl_get_config()->ImageEnhance 控制
*/
extern int btl_cardo_ImageEnhance(unsigned char*image, int w, int h);
/*
支持传入外部图像增强接口，替换内部的图像增强
*/
extern int (*btl_Customized_imageEnhance_pointer)(unsigned char*, int, int);



#endif
