/*
 * A reference code for CLRC663/MFRC631/MFRC630
 */

#include <string.h>
#include "rc663_config.h"
#include "status_code.h"
#include "hal_reg.h"
#include "rc663.h"


MfCmdInfo MInfo;
volatile unsigned short tmo=0;				// extra software timeout
volatile uint8_t RcDriverMode;	// default for TypeA


/**************************************************************************************************
** Reset data buffer
***************************************************************************************************/
void ResetInfo(void)
{
  uint8_t i;
  uint8_t *BfrPtr = (uint8_t *)(&MInfo);

  for(i=0 ; i<sizeof(MfCmdInfo); i++)
    BfrPtr[i] = 0;
}

/**************************************************************************************************
** IN:						_4p7us			number of 4.72us
** 								Set timeout, using timer0
** 								max. ~309.2ms
**************************************************************************************************/
void SetTimeOut4p7us(unsigned int _4p7us)
{
	uint8_t h,l;
	
	l = (uint8_t)(_4p7us&0xff);
	RC663_RegSet(T0ReloadLoReg, l);
	h = (uint8_t)((_4p7us>>8)&0xff);
	RC663_RegSet(T0ReloadHiReg, h);
}

/**************************************************************************************************
** swicth on RF feild, and wait for RF_StartUp_time
**************************************************************************************************/
void PcdRfOn(void)
{
	RC663_RegSet(DrvModeReg, RcDriverMode);
	RC663_Delayms(RF_ON_STARTUP_TIME_ms);
}

/**************************************************************************************************
** swicth off RF feild
**************************************************************************************************/
void PcdRfOff(void)
{
	RC663_RegSet(DrvModeReg, 0x80);
}

/**************************************************************************************************
** IN:					uint8_t ms								RF switch off time in ms
** Func:				switch off RF power for ms and restart the RF
**************************************************************************************************/
void PcdRfReset(uint8_t ms)
{
	// RF OFF
	PcdRfOff();
	RC663_Delayms(ms);
	// RF ON
	PcdRfOn(); 
}

/**************************************************************************************************
** General settings
**************************************************************************************************/
char PcdCommonConfig(void)
{
	char status = 0;

	/* ------- Timer0 --------*/
	// timer0 is used as a timeout timer
	// timer0 can be started/stopped manually
	// timer0 will start automatically at end of transmission
	// timer0 will stop automatically after receiving first 4 bits
	// timer0 will NOT restart automatically - down to 0 and irq occurs
	// timer0 will use 211875Hz input clock - each count = 1/211875=4.72us
	// max time span is 65536/211875=309.3ms
	RC663_RegSet(T0ControlReg, 0x91);

	/* ------- others ----------*/
	// FIFO size = 512 bytes
	RC663_RegSet(FIFOControlReg, FIFO_DEFAULT);
	//RC663_RegSet(WaterLevelReg, 0xFF);
	// set minimum Rx reception level and minimum phase shift 
	RC663_RegSet(RxThresholdReg, ((RX_Min_Reception_Level & 0xF0)| (RX_Min_Phase_Shift_Level & 0x0F)));
	// set Rx gain
	RC663_RegSet(RxAnaReg, ((RX_Gain & 0x03) | (RX_HPCF & 0x0C)));

	return status;
}

/**************************************************************************************************
** Set chip to specified protocol operation state
**************************************************************************************************/
char PcdLoadProtocol(uint8_t rx, uint8_t tx)
{
	char status = 0;
	uint8_t buffer[2];
	
	ResetInfo();
	
	buffer[0] = rx;
	buffer[1] = tx;

	MInfo.nBytesToSend = 2;
	
	// reader ic's timer timeout
	SetTimeOut4p7us(TIMEOUT_RC6xx_Timer_4p7_LoadProtocol);
	// extra sw timeout
	tmo = TIMEOUT_sw_LoadProtocol;

	status = PcdCmd(PCD_LOADPROTOCOL, buffer);
	
	return status;	
}

/**************************************************************************************************
** IN:					uint8_t cmd               command code
** 						uint8_t *ExchangeBuf      data in-out buffer
** Func:				implement chip command
**************************************************************************************************/
char  PcdCmd(uint8_t cmd,uint8_t *ExchangeBuf)
{
	char status = MI_OK;
//	uint8_t doReceive = 0;
	uint8_t nbytes = 0;
	uint8_t nbits = 0;
	uint8_t getRegVal = 0;
	uint8_t coll;
	uint8_t i;
	
	// take following sequence to process a command
	// terminate any running command
	// clear interrupts, clear error flags, flush the FIFO
	// enable corresponding interrupts
	// prepare data and write into FIFO
	// write command into command register
	
	// terminate any running command
	RC663_RegSet(CommandReg, 0x0);
	// disable all irqs
	RC663_RegSet(IRQ1EnReg, 0xC0);
	RC663_RegSet(IRQ0EnReg, 0x0);
	// clear interrupts
	RC663_RegSet(IRQ0Reg, 0x7F);
	RC663_RegSet(IRQ1Reg, 0x7F);
	// clear error flags
	RC663_RegSet(ErrorReg, 0x0);
	// flush the FIFO, FIFO size set to 512 bytes
	RC663_RegSet(FIFOControlReg, 0x10);
	
	// record the command
	MInfo.cmd = cmd;
	
	switch(cmd)
	{
		case PCD_IDLE:
		case PCD_LOADPROTOCOL:
		case PCD_MFLOADKEY:
		case PCD_MFAUTHENT:
		case PCD_WRITEE2:
		case PCD_WRITEE2PAGE:
		case PCD_SOFTRESET:
			MInfo.irqEn0 = 0x12;			// enable idle irq / error irq
			MInfo.irqEn1 = 0x81;			// enable timer0 / IRQ pin = PP
			MInfo.waitFor0 = 0x10;		// wait for idle irq
			MInfo.waitFor1 = 0x01;		// wait for timer0 irq
	#if IRQ_PIN_en
			MInfo.irqEn1 |= 0x40;			// enable IRQ pin
	#endif
			MInfo.doReceive = 0;			// no return data in fifo
			break;
		case PCD_READE2:
		case PCD_GETRNR:
			MInfo.irqEn0 = 0x12;			// enable idle irq / error irq
			MInfo.irqEn1 = 0x81;			// enable timer0 / IRQ pin = PP
			MInfo.waitFor0 = 0x10;		// wait for idle irq
			MInfo.waitFor1 = 0x01;		// wait for timer0 irq
			MInfo.doReceive = 1;
			break;
		case PCD_TRANSMIT:					
			MInfo.irqEn0 = 0x0A;			// enable Tx irq / error irq
			MInfo.irqEn1 = 0x80;			// IRQ pin = PP, not inverted
			MInfo.waitFor0 = 0x08;		// wait for Tx irq
			MInfo.waitFor1 = 0x00;		
			MInfo.doReceive = 0;
			break;
		case PCD_TRANSCEIVE:
			MInfo.irqEn0 = 0x06;			// enable Rx irq / error irq
			MInfo.irqEn1 = 0x81;			// enable timer0 / IRQ pin = PP, not inverted
			MInfo.waitFor0 = 0x04;		// wait for rx irq
			MInfo.waitFor1 = 0x01;		// wait for timer0 irq
			MInfo.doReceive = 1;
			break;
		default:
			status = STATUS_UNSUPPORTED_COMMAND;
	}
	if(status == MI_OK)
	{
		// enable irqs
		// NOTE: IRQxEN register setting means the irq will be propagated to IRQ pin, or not
		RC663_RegSet(IRQ0EnReg, MInfo.irqEn0);
		RC663_RegSet(IRQ1EnReg, MInfo.irqEn1);
		
		// always wait for error irq
		MInfo.waitFor0 |= 0x02;
		
		// write data into FIFO
		WriteFIFO(ExchangeBuf, MInfo.nBytesToSend);
		
		// start the command
		RC663_RegSet(CommandReg, cmd);
		
		// manually start timer0 for non-Transceive command
		if((cmd != PCD_TRANSCEIVE) && (cmd != PCD_TRANSMIT))
		{
			RcModifyReg(TControlReg, 1, 0x11);
		}
//#ifdef FWT_TIMER	// mcu hardware timeout timer
//// manually start timer		
//#endif
		// wait for any enabled interrupt
		while(--tmo)
		{
			MInfo.irq0 = RC663_RegRead(IRQ0Reg);
			MInfo.irq1 = RC663_RegRead(IRQ1Reg);
			if(MInfo.irq0 & MInfo.waitFor0) 	// expected irq
				break;
			if(MInfo.irq1 & MInfo.waitFor1)		// timeout irq
				break;
			for(i=20; i>20; i--){;}
		}

		// manually stop timer0 for non-Transceive command
		if(cmd != PCD_TRANSCEIVE)
		{
			RcModifyReg(TControlReg, 0, 0x01);
		}
		if(!tmo)	// check sw timeout
		{
			status = STATUS_IO_TIMEOUT;
		}
		// check timeout from timer0
		if((MInfo.irq1 & 0x01))
		//if(((cmd != PCD_TRANSCEIVE) && (MInfo.irq1 & 0x01)) || ((cmd == PCD_TRANSCEIVE) && (MInfo.irq1 & 0x02)))
		{
			status = STATUS_IO_TIMEOUT;
		}
		else
		{
			// disable all interrupts
			RC663_RegSet(IRQ0EnReg, 0x00);
			RC663_RegSet(IRQ1EnReg, 0x80);
			
			// if there is any data to be received
			if(MInfo.doReceive && (status == MI_OK))
			{
				// complete bytes
				MInfo.nBytesReceived = RC663_RegRead(FIFOLengthReg);
				nbytes = RC663_RegRead(FIFOControlReg) & 0x03;
				if(nbytes & 0x01)
				{
					MInfo.nBytesReceived += 256;
				}
				if(nbytes & 0x02)
				{
					MInfo.nBytesReceived = 512;
				}

				// remaining bits (does not include complete bytes)
				nbits = RC663_RegRead(RxBitCtrlReg);
				MInfo.nBitsReceived = nbits & 0x07;
			}
			if(status == MI_OK)
			{
				getRegVal = RC663_RegRead(ErrorReg);
				// error info
				if(getRegVal)
				{
					if(getRegVal & 0x04)
					{
						// collision error
						status = STATUS_COLLISION_ERROR;
						// get collision position, MSB7 shall be 1 if collision is in valid range
						coll = RC663_RegRead(RxCollReg);
						if(coll&0x80)
						{
							// ISO14443 has a uid of max.32bit
							// ISO15693 has a uid of max.64bit
							if((coll&0x7F) < 64)
							{
								MInfo.collPos = coll & 0x3F;
							}
							else
							{
								status = STATUS_INVALID_COLLPOS;
							}
						}
						else
						{
							// invalid collision position - out of range
							status = STATUS_INVALID_COLLPOS;
						}						
					}
					else if(getRegVal & 0x01)
					{
						if(((MInfo.nBytesReceived == 0x01) || (MInfo.nBytesReceived == 0x00)) && (MInfo.nBitsReceived == 0x04))
						{   // CRC error AND only 4 bites received -> consider Mifare (N)ACK
							ExchangeBuf[0] = RC663_RegRead(FIFODataReg);
							//MInfo.nBytesReceived = 1;
							status = STATUS_ACK_SUPPOSED;   // (N)ACK
						}
						else
							status = STATUS_CRC_ERROR;      // CRC error
					}

					if(getRegVal & 0x80)
						status = STATUS_EEPROM_ERROR;				// EEPROM operation error
					else if(getRegVal & 0x40)
						status = STATUS_FIFO_WRITE_ERROR;		// write FIFO error					
					else if(getRegVal & 0x20)
						status = STATUS_BUFFER_OVERFLOW;		// FIFO overflow error
					else if(getRegVal & 0x10)
						status = STATUS_MINFRAME_ERROR;			// Min. Frame error
					else if(getRegVal & 0x08)
						status = STATUS_NODATA_ERROR;				// No data in FIFO while required					
					else if(getRegVal & 0x02)
						status = STATUS_PROTOCOL_ERROR;			// protocol error

					// in case of error, clear it before IRQ 
					RC663_RegSet(ErrorReg, 0);
				}
				else
				{
					if(((MInfo.nBytesReceived == 0x01) || (MInfo.nBytesReceived == 0x00)) && (MInfo.nBitsReceived == 0x04))
					{   // CRC error AND only 4 bites received -> consider Mifare (N)ACK
						ExchangeBuf[0] = RC663_RegRead(FIFODataReg);
						//MInfo.nBytesReceived = 1;
						status = STATUS_ACK_SUPPOSED;   // (N)ACK
					}
				}

				// read FIFO and set parameters
				if(MInfo.doReceive && (status != STATUS_ACK_SUPPOSED))
				{
					ReadFIFO(ExchangeBuf, MInfo.nBytesReceived);
					// last byte incomplete
					if(MInfo.nBitsReceived && MInfo.nBytesReceived)
						MInfo.nBytesReceived --;
				}
			}
		}
	}
	return status;
}


/**************************************************************************************************
** IN:					uint8_t addrH						address High byte (EEPROM byte address)
** 							uint8_t addrL						address Low byte
**							uint8_t len							number of bytes to be read
** OUT:					uint8_t *buf						output buffer
** FUNC:				read chip's EEPROM 
**************************************************************************************************/
char PcdReadE2(uint8_t addrH, uint8_t addrL, uint8_t len, uint8_t *buf)
{
	char status = STATUS_SUCCESS;
	uint8_t buffer[256];
	
	buffer[0] = addrH;
	buffer[1] = addrL;
	buffer[2] = len;

	ResetInfo();
	MInfo.nBytesToSend   = 3;

	// reader ic's timer timeout
	SetTimeOut4p7us(TIMEOUT_RC6xx_Timer_4p7_ReadE2);
	// extra sw timeout
	tmo = TIMEOUT_sw_ReadE2;
	
	status = PcdCmd(PCD_READE2, buffer);

	if(status == STATUS_SUCCESS)
	{
		memcpy(buf, buffer, len);
	}
	
	return status;	
}

/**************************************************************************************************
** IN:					uint8_t addrH						address High byte (EEPROM byte address)
** 							uint8_t addrL						address Low byte
** 							uint8_t data						data to be written
** FUNC:				write one byte into chip's EEPROM 
**************************************************************************************************/
char PcdWriteE2SingleByte(uint8_t addrH, uint8_t addrL, uint8_t data)
{
	char  status = STATUS_SUCCESS;
	uint8_t buffer[4];
	uint8_t dt;

	buffer[0] = addrH;
	buffer[1] = addrL;
	buffer[2] = data;
	
	ResetInfo();
	MInfo.nBytesToSend = 3;
	
	SetTimeOut4p7us(TIMEOUT_RC6xx_Timer_4p7_WriteE2);
	tmo = TIMEOUT_sw_WriteE2;	
	
	status = PcdCmd(PCD_WRITEE2, buffer);
	
	if(status == STATUS_SUCCESS)
	{
		
		status = PcdReadE2(addrH, addrL, 1, &dt);
		if(!status)
		{
			if(data != dt)
			{
				status = STATUS_EEPROM_WRITE_ERROR;
			}
		}
	}

	return status;
}

/**************************************************************************************************
** IN:					uint8_t addr						page address
** 							uint8_t *din						input data buffer
** FUNC:				write one page(64 bytes) into chip's EEPROM 
**************************************************************************************************/
char PcdWriteE2Page(uint8_t page_addr, uint8_t *din)
{
	char  status = STATUS_SUCCESS;
	uint8_t buffer[72];
	uint8_t addrL, addrH;
	unsigned short tmp;
	
	buffer[0] = page_addr;
	memcpy(&buffer[1], din, 64);
		
	ResetInfo();
	MInfo.nBytesToSend   = 65;
	
	SetTimeOut4p7us(TIMEOUT_RC6xx_Timer_4p7_WriteE2);
	tmo = TIMEOUT_sw_WriteE2;	
	
	status = PcdCmd(PCD_WRITEE2PAGE, buffer);
	
	if(status == STATUS_SUCCESS)
	{
		tmp = page_addr<<6;
		addrL = tmp%256;
		addrH = tmp/256;
		
		status = PcdReadE2(addrH, addrL, 64, buffer);
		if(!status)
		{
			if(memcmp(din, buffer, 64) != 0)
			{
				status = STATUS_EEPROM_WRITE_ERROR;
			}
		}
	}	
	return status;
}

/**************************************************************************************************
** IN:					uint8_t len							length
** 							uint8_t *buf						output buffer
** FUNC:				generate random numbers 
**************************************************************************************************/
char PcdGetRnr(uint8_t len, uint8_t *buf)
{
	char  status = STATUS_SUCCESS;
	uint8_t i=0;
	
	RC663_RegSet(CommandReg, 0);
	RC663_RegSet(CommandReg, PCD_GETRNR);
	
	while(i<len)
	{
		i = RC663_RegRead(FIFOLengthReg);
	}
	RC663_RegSet(CommandReg, 0);
	for(i=0; i<len; i++)
	{
		buf[i] = RC663_RegRead(FIFODataReg);
	}
	
	return status;
}

/**************************************************************************************************
** void
** FUNC:				soft reset, all registers will set to default 
**************************************************************************************************/
char PcdSoftReset(void)
{
	char  status = STATUS_SUCCESS;
	uint8_t buffer[4];
		
	ResetInfo();
	
	SetTimeOut4p7us(TIMEOUT_RC6xx_Timer_4p7_WriteE2);
	tmo = TIMEOUT_sw_WriteE2;	
	
	status = PcdCmd(PCD_SOFTRESET, buffer);	
	
	return status;
}
