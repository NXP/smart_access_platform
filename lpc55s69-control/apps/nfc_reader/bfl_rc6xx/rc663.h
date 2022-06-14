/*
 * A reference code for CLRC663/MFRC631/MFRC630
 */


#ifndef __CLRC663_H__
#define __CLRC663_H__

//=================================================================================================
typedef struct
{
	uint8_t  cmd;                 // command
	char           status;              // status
	uint8_t  nBytesSent;          // bytes already sent
	uint8_t  nBytesToSend;        // bytes to be sent
	unsigned short nBytesReceived; 
	uint8_t  nBitsReceived;
	uint8_t  irqEn0;							// irq enable0
	uint8_t  irqEn1;							// irq enable1
	uint8_t  waitFor0;						// expected irq
	uint8_t  waitFor1;
	uint8_t  irq0;								// irq0
	uint8_t  irq1;								// irq1
	uint8_t  collPos;							// collision position
	uint8_t  doReceive;						// require data to return
} MfCmdInfo;

				 
extern MfCmdInfo MInfo;
extern volatile uint8_t RcDriverMode;
extern volatile unsigned short tmo;

//CLRC663
#define TIMEOUT_TIMER0				0x01
#define TIMEOUT_TIMER1				0x02
#define TIMEOUT_TIMER2				0x04
#define TIMEOUT_TIMER3				0x08
#define TIMEOUT_TIMER4				0x10


//CLRC663 Commands
#define PCD_IDLE              0x00
#define PCD_LPCD							0x01
#define PCD_MFLOADKEY					0x02
#define PCD_MFAUTHENT         0x03
#define PCD_ACKREQ						0x04
#define PCD_RECEIVE           0x05
#define PCD_TRANSMIT          0x06
#define PCD_TRANSCEIVE        0x07
#define PCD_WRITEE2						0x08
#define PCD_WRITEE2PAGE				0x09
#define PCD_READE2						0x0A
#define PCD_LOADREG						0x0C
#define PCD_LOADPROTOCOL			0x0D
#define PCD_LOADKEYE2					0x0E
#define PCD_STOREKEYE2				0x0F
#define PCD_GETRNR  		      0x1C
#define PCD_SOFTRESET         0x1F


//MF663 FIFO Size
#define DEF_FIFO_LENGTH       512           	   //FIFO size=512Bytes
#define MAXRLEN  							512

//CLRC663 registers
// Command
#define     CommandReg					0x00
// SAM configuration
#define     HostCtrlReg					0x01
// FIFO
#define     FIFOControlReg			0x02
#define     WaterLevelReg				0x03
#define     FIFOLengthReg				0x04
#define     FIFODataReg					0x05
// Interrupt
#define     IRQ0Reg							0x06    
#define     IRQ1Reg							0x07    
#define     IRQ0EnReg						0x08    
#define     IRQ1EnReg						0x09
// Contactless interface
#define     ErrorReg          	0x0A
#define     statusReg         	0x0B
#define     RxBitCtrlReg      	0x0C
#define     RxCollReg         	0x0D
// Timer configuration
#define     TControlReg					0x0E
#define     T0ControlReg				0x0F
#define     T0ReloadHiReg				0x10
#define     T0ReloadLoReg				0x11
#define     T0CounterValHiReg		0x12
#define     T0CounterValLoReg		0x13
#define     T1ControlReg				0x14
#define     T1ReloadHiReg				0x15
#define     T1ReloadLoReg				0x16
#define     T1CounterValHiReg		0x17
#define     T1CounterValLoReg		0x18
#define     T2ControlReg				0x19
#define     T2ReloadHiReg				0x1A
#define     T2ReloadLoReg				0x1B
#define     T2CounterValHiReg		0x1C
#define     T2CounterValLoReg		0x1D
#define     T3ControlReg				0x1E
#define     T3ReloadHiReg				0x1F
#define     T3ReloadLoReg				0x20
#define     T3CounterValHiReg		0x21
#define     T3CounterValLoReg		0x22
#define     T4ControlReg				0x23
#define     T4ReloadHiReg				0x24
#define     T4ReloadLoReg				0x25
#define     T4CounterValHiReg		0x26
#define     T4CounterValLoReg		0x27
// Transmitter
#define			DrvModeReg					0x28
#define			TxAmpReg						0x29
#define			TxConReg						0x2A
#define			TxlReg							0x2B
// CRC configuration
#define			TxCrcPresetReg			0x2C
#define			RxCrcConReg					0x2D
// Transmitter configuration
#define			TxDataNumReg				0x2E
#define			TxDATAModWidthReg		0x2F
#define			TxSym10BurstLenReg	0x30
#define			TxWaitCtrlReg				0x31
#define			TxWaitLoReg					0x32
// FrameCon
#define			FrameConReg					0x33
#define			RxSofDReg						0x34
#define			RxCtrlReg						0x35
#define			RxWaitReg						0x36
#define			RxThresholdReg			0x37
#define			RcvReg							0x38
#define			RxAnaReg						0x39
#define 		RFU1Reg							0x3A
// Clock configuration
#define			SerialSpeedReg			0x3B
#define			LFO_TrimmReg				0x3C
#define			PLL_CtrlReg					0x3D
#define			PLLDiv_OutReg				0x3E
// Low-power Card Detection
#define			LPCD_QMinReg				0x3F
#define			LPCD_QMaxReg				0x40
#define			LPCD_IMinReg				0x41
#define			LPCD_Result_IReg		0x42
#define			LPCD_Result_QReg		0x43
// Pin configuration
#define			PinEnReg						0x44
#define			PinOutReg						0x45
#define			PinInReg						0x46
#define			SigOutReg						0x47
// Protocol configuration
#define			TxBitModReg					0x48
#define 		RFU2Reg							0x49
#define			TxDataConReg				0x4A
#define			TxDataModReg				0x4B
#define			TxSymFreqReg				0x4C
#define			TxSym0HReg					0x4D
#define			TxSym0LReg					0x4E
#define			TxSym1HReg					0x4F
#define			TxSym1LReg					0x50
#define			TxSym2Reg						0x51
#define			TxSym3Reg						0x52
#define 		TxSym10LenReg				0x53
#define			TxSym32LenReg				0x54
#define			TxSym10BurstCtrlReg	0x55
#define			TxSym10ModReg				0x56
#define 		TxSym32ModReg				0x57
//	Receiver configuration
#define			RxBitModReg					0x58
#define			RxEofSymReg					0x59
#define			RxSyncValHReg				0x5A
#define			RxSyncValLReg				0x5B
#define			RxSyncModReg				0x5C
#define			RxModReg						0x5D
#define			RxCorrReg						0x5E
#define			FabCaliReg					0x5F
//	Version
#define			VersionReg					0x7F


/**************************************************************************************************
** Reset data buffer
***************************************************************************************************/
void ResetInfo(void);

/**************************************************************************************************
** IN:						_4p7us			number of 4.72us
** 								Set timeout, using timer0
** 								max. ~309.2ms
**************************************************************************************************/
void SetTimeOut4p7us(unsigned int _4p7us);

/**************************************************************************************************
** swicth on RF feild, and wait for RF_StartUp_time
**************************************************************************************************/
void PcdRfOn(void);

/**************************************************************************************************
** swicth off RF feild
**************************************************************************************************/
void PcdRfOff(void);

/**************************************************************************************************
** IN:					uint8_t ms								RF switch off time in ms
** Func:				switch off RF power for ms and restart the RF
**************************************************************************************************/
void PcdRfReset(uint8_t ms);

/**************************************************************************************************
** delay ms by PCD
**************************************************************************************************/
void PcdDelayMs(unsigned short ms);

/**************************************************************************************************
** General settings
**************************************************************************************************/
char PcdCommonConfig(void);

/**************************************************************************************************
** Set chip to specified protocol operation state
**************************************************************************************************/
char PcdLoadProtocol(uint8_t rx, uint8_t tx);

/**************************************************************************************************
** IN:					uint8_t cmd								command code
** 							uint8_t *ExchangeBuf      data in-out buffer
** Func:				implement chip command
**************************************************************************************************/
char  PcdCmd(uint8_t cmd,uint8_t *ExchangeBuf);

/**************************************************************************************************
** IN:					uint8_t addrH						address High byte (EEPROM byte address)
** 							uint8_t addrL						address Low byte
**							uint8_t len							number of bytes to be read
** OUT:					uint8_t *buf						output buffer
** FUNC:				read chip's EEPROM 
**************************************************************************************************/
char PcdReadE2(uint8_t addrH, uint8_t addrL, uint8_t len, uint8_t *buf);

/**************************************************************************************************
** IN:					uint8_t addrH						address High byte (EEPROM byte address)
** 							uint8_t addrL						address Low byte
** 							uint8_t data						data to be written
** FUNC:				write one byte into chip's EEPROM 
**************************************************************************************************/
char PcdWriteE2SingleByte(uint8_t addrH, uint8_t addrL, uint8_t data);

/**************************************************************************************************
** IN:					uint8_t addr						page address
** 							uint8_t *din						input data buffer
** FUNC:				write one page(64 bytes) into chip's EEPROM 
**************************************************************************************************/
char PcdWriteE2Page(uint8_t page_addr, uint8_t *din);

/**************************************************************************************************
** IN:					uint8_t len							length
** 							uint8_t *buf						output buffer
** FUNC:				generate random numbers 
**************************************************************************************************/
char PcdGetRnr(uint8_t len, uint8_t *buf);

/**************************************************************************************************
** void
** FUNC:				soft reset, all registers will set to default 
**************************************************************************************************/
char PcdSoftReset(void);


#endif              // __CLRC663_H__
