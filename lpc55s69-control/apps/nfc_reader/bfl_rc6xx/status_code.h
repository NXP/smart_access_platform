/*
 * A reference code for CLRC663/MFRC631/MFRC630
 */

 
#ifndef __STATUS_CODE_H__
#define __STATUS_CODE_H__

//=================================================================================================
#define MI_OK                           (char)(0)
#define STATUS_SUCCESS                  (char)(0)			// 操作成功

#define STATUS_IO_TIMEOUT								(char)(-1)		// 无应答错误
#define STATUS_CRC_ERROR								(char)(-2)		// CRC校验错误
#define STATUS_PARITY_ERROR							(char)(-3)		// 奇偶校验错误
#define STATUS_BITCOUNT_ERROR						(char)(-4)		// 接收位计数器错误
#define STATUS_FRAMING_ERROR						(char)(-5)		// 帧错误
#define STATUS_COLLISION_ERROR					(char)(-6)		// 位冲突错误
#define STATUS_MINFRAME_ERROR						(char)(-7)		// Min. Frame error, less than 4 bits received
#define STATUS_ACCESS_DENIED						(char)(-8)		// 写禁止
#define STATUS_BUFFER_OVERFLOW					(char)(-9)		// BUF溢出错误
#define STATUS_PROTOCOL_ERROR						(char)(-10)		// 通信协议有误
#define STATUS_EEPROM_ERROR					    (char)(-11)		// EEPROM operation error
#define STATUS_FIFO_WRITE_ERROR         (char)(-12)		// FIFO写错误
#define STATUS_USERBUFFER_FULL          (char)(-13)		// 用户缓冲区满
#define STATUS_INVALID_PARAMETER        (char)(-14)		// 参数错误
#define STATUS_UNSUPPORTED_PARAMETER    (char)(-15)		// 无效参数
#define STATUS_UNSUPPORTED_COMMAND      (char)(-16)		// 无效命令
#define STATUS_INTERFACE_ERROR          (char)(-17)		// 主机接口错误
#define STATUS_INVALID_FORMAT           (char)(-18)		// 无效格式, CRC或者奇偶错误
#define STATUS_INTERFACE_NOT_ENABLED    (char)(-19)   // 接口未激活
#define STATUS_AUTHENT_ERROR            (char)(-20)   // 验证错误
#define STATUS_ACK_SUPPOSED             (char)(-21)   // NACK
#define STATUS_BLOCKNR_NOT_EQUAL        (char)(-22)		// 通信块错误
#define STATUS_INVALID_COLLPOS	       	(char)(-23)   // 目标死锁
#define STATUS_TARGET_SET_TOX        	 	(char)(-24)  	// 目标发送草食
#define STATUS_TARGET_RESET_TOX      		(char)(-25)  
#define STATUS_WRONG_UID_CHECKBYTE   	 	(char)(-26)  	// 目标UID检测错误
#define STATUS_WRONG_HALT_FORMAT     	 	(char)(-27)		// 挂起格式错误
#define STATUS_ID_ALREADY_IN_USE       	(char)(-28)   // ID已被使用
#define STATUS_INSTANCE_ALREADY_IN_USE 	(char)(-29)   // INSTANCE 已被使用
#define STATUS_ID_NOT_IN_USE          	(char)(-30)   // 指定的ID不存在
#define STATUS_NO_ID_AVAILABLE         	(char)(-31)   // 无空闲ID可用
#define STATUS_OTHER_ERROR              (char)(-32)		// 其他错误
#define STATUS_INSUFFICIENT_RESOURCES   (char)(-33)   // 系统资源不足
#define STATUS_INVALID_DEVICE_STATE    	(char)(-34)   // 驱动错误
#define STATUS_NODATA_ERROR        	    (char)(-35)   // NO data in FIFO while data is required
#define STATUS_INIT_ERROR								(char)(-36)   // 初始化错误
#define STATUS_NOBITWISEANTICOLL				(char)(-37)
#define STATUS_SERNR_ERROR							(char)(-38)
#define STATUS_NY_IMPLEMENTED						(char)(-39)
#define STATUS_ACCESS_TIMEOUT						(char)(-40)

#define	STATUS_BYTE_COUNT_ERROR					(char)(-50)		// error on number_of_bytes
#define	STATUS_OVERFLOW_UNDERFLOW				(char)(-51)		// overflow or underflow at value operation

#define STATUS_ATS_ERROR			    			(char)(-60)   // ATS错误
#define STATUS_FSC_ERROR			    			(char)(-61)		// RFU FSC(I)
#define STATUS_DRI_DSI_ERROR						(char)(-65)		// DRI/DSI not in range
#define STATUS_PPSS_ERROR								(char)(-66)   // PPSS error
#define STATUS_BLOCK_FSD_ERROR					(char)(-67)		// sending more data than FSC
#define STATUS_BLOCK_FORMAT_ERROR       (char)(-68)   // 分组帧格式错误
#define STATUS_PCB_ERROR								(char)(-69)		// PCB error
#define STATUS_CID_ERROR  			    		(char)(-70)   // PPSS错误
#define STATUS_NAD_ERROR								(char)(-71)		// NAD error
#define STATUS_BLOCK_NUMBER_ERROR				(char)(-71)		// Block Number error
#define STATUS_DESELECT_ERROR						(char)(-71)		// deselect error, failed to deselect

	
#define STATUS_DESFIRE_COM_ERROR				(char)(-94)		// general Desfire error
#define STATUS_DESIFRE_READ_ERROR				(char)(-95)		// read general error
#define STATUS_DESIFRE_READ_LEN_ERROR		(char)(-96)		// read length error
#define STATUS_DESIFRE_READ_CRC_ERROR		(char)(-97)		// read crc error	

#define STATUS_DESIFRE_WRITE_ERROR			(char)(-98)		// write general error
#define STATUS_DESIFRE_WRITE_LEN_ERROR	(char)(-99)		// write length error
#define STATUS_DESIFRE_WRITE_CRC_ERROR	(char)(-100)	// write crc error	

#define STATUS_EEPROM_WRITE_ERROR				(char)(-80)   // EEPROM operation error
//=======================================================================================
#endif          // __STATUS_CODE_H__

