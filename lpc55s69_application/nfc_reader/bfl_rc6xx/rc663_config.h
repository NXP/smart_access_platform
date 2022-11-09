/*
 * A reference code for CLRC663/MFRC631/MFRC630
 */
 
#ifndef __MFRC663CONFIG_H__
#define __MFRC663CONFIG_H__


// timeout by reader chip timer, timeout = tm x 4.7 us
// extra software timeout guard
#define TIMEOUT_RC6xx_Timer_4p7_LoadProtocol			50			// 50x4.7us = 94us
#define TIMEOUT_sw_LoadProtocol										15	
#define TIMEOUT_RC6xx_Timer_4p7_RequestA					30
#define TIMEOUT_sw_RequestA												50
#define TIMEOUT_RC6xx_Timer_4p7_AntiCollisionA		50
#define TIMEOUT_sw_AntiCollisionA									50
#define TIMEOUT_RC6xx_Timer_4p7_SelectA						50
#define TIMEOUT_sw_SelectA												50
#define TIMEOUT_RC6xx_Timer_4p7_HaltA							212			// if PICC responds any modulation in 1ms => HALT failed
#define TIMEOUT_sw_HaltA													500			// larger than TIMEOUT_RC6xx_Timer_4p7_HaltA * 4.7us
#define TIMEOUT_RC6xx_Timer_4p7_RATS							1000
#define TIMEOUT_sw_RATS														500
#define TIMEOUT_RC6xx_Timer_4p7_DeSelect					1000
#define TIMEOUT_sw_DeSelect												500


#define TIMEOUT_RC6xx_Timer_4p7_ReadE2						6000
#define TIMEOUT_sw_ReadE2													3000
#define TIMEOUT_RC6xx_Timer_4p7_WriteE2						55000
#define TIMEOUT_sw_WriteE2												20000
#define TIMEOUT_RC6xx_Timer_4p7_SoftReset					50
#define TIMEOUT_sw_SoftReset											10

#define TIMEOUT_RC6xx_Timer_4p7_MifareRead				1000
#define TIMEOUT_sw_MifareRead											500
#define TIMEOUT_RC6xx_Timer_4p7_MifareWrite				1000
#define TIMEOUT_sw_MifareWrite										500
#define TIMEOUT_RC6xx_Timer_4p7_LoadKey						1000
#define TIMEOUT_sw_LoadKey												500
#define TIMEOUT_RC6xx_Timer_4p7_Authen						1000
#define TIMEOUT_sw_Authen													500
#define TIMEOUT_RC6xx_Timer_4p7_Value							1000
#define TIMEOUT_sw_Value													500


// RF reset time
#define TIME_RF_RESET															6

// drive mode for Type A 106kbps
#define RcvMode_TypeA106													0x89


// RF_ON Startup time
#define RF_ON_STARTUP_TIME_ms											6			// RF on time = 6ms, ISO defines 5.1ms min.
// FIFO
#define FIFO_DEFAULT															0x0		// fifo default = 512 bytes
// minimum signal level of Rx reception
#define RX_Min_Reception_Level										0x30	// format: (----xxxx)b, default RxThreshold = 0x3F
// minimum level of Rx phase shift
#define RX_Min_Phase_Shift_Level									0x0F	// format: (xxxx----)b, default RxThreshold = 0x3F
// set Rx gain
#define RX_Gain																		0x02	// format ------xx(bit1/0), default (10)b
// set High Pass Filter cut-off frequency
#define RX_HPCF																		0x08	// format ----xx--(bit3/2), default (10)b


#define IRQ_PIN_en																0			// enable IRQ pin -> IRQ will be SET when there is an irq

#define CLRC663_PLUS															1			

#endif              // __MFRC663CONFIG_H__
