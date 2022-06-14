/*
 * A reference code for CLRC663/MFRC631/MFRC630
 */

#include <string.h>
#include "rc663_config.h"
#include "hal_reg.h"
#include "rc663.h"
#include "status_code.h"

#include "iso14443_3a.h"


/**************************************************************************************************
** FUNC:				set chip to work at ISO14443 TypeA 106kbps protocol
**************************************************************************************************/
uint8_t PICC_PcdTypeA106Config(void)
{
	uint8_t status = MI_OK;

	status = PcdCommonConfig();
	if(status)
		return status;
	
	// set driver mode
	RcDriverMode = RcvMode_TypeA106;
	
	// config chip for TypeA@106kbps
	status = PcdLoadProtocol(0x0, 0x0);
	// RF reset
	PcdRfReset(TIME_RF_RESET);

	// these registers needs to be set in case higher baudrate is used before the config
	// for applications which work only at 106kbps, these settings can be skipped
	// 0x2F, ModWidth for Miller modulation
	RC663_RegSet(TxDATAModWidthReg, 0x27);
	// receiver baudrate
	RC663_RegSet(RxCtrlReg, 0x04);
	RC663_RegSet(TxSymFreqReg, 0x40);
	RC663_RegSet(TxDataConReg, 0x04);
	RC663_RegSet(RxBitModReg, 0x02);
	RC663_RegSet(RxSyncValLReg, 0x01);
	RC663_RegSet(RxModReg, 0x08);	
	
	return status;
}

/**************************************************************************************************
** IN:					uint8_t req_code          REQALL=0x52��REQIDLE=0x26
** OUT:			    uint8_t *atq              ATQA
**************************************************************************************************/
uint8_t PICC_RequestA(uint8_t req_code, uint8_t *atq)
{
	uint8_t status = STATUS_SUCCESS;
	uint8_t buffer[8];	
    
	// deavtivate Crypto1
	RcModifyReg(statusReg, 0, 0x20);
	// bits after collision does not change
	RC663_RegSet(RxBitCtrlReg, 0x80);
	// disable TxCRC and RxCRC
	RC663_RegSet(TxCrcPresetReg, 0x18);
	RC663_RegSet(RxCrcConReg, 0x18);
	// enable Tx/Rx parity
	RC663_RegSet(FrameConReg, 0xCF);		
	// send only 7 bits
	RC663_RegSet(TxDataNumReg, 0x0F);
	
	ResetInfo();
	buffer[0] = req_code;
	MInfo.nBytesToSend = 1;
    
	SetTimeOut4p7us(TIMEOUT_RC6xx_Timer_4p7_RequestA);
	tmo = TIMEOUT_sw_RequestA;
    
	status = PcdCmd(PCD_TRANSCEIVE, buffer);
	if(status == STATUS_SUCCESS || status == STATUS_COLLISION_ERROR)
	{
		if(MInfo.nBytesReceived != 2 || MInfo.nBitsReceived != 0x00)
		{
			status = STATUS_BITCOUNT_ERROR;
		}
		else
		{
			//status = MI_OK;
			memcpy(atq, buffer, 2);	// output ATQA even with collision(s)
		}
	}
	// reset "bits to be sent in last byte"
	RC663_RegSet(TxDataNumReg, 0x08);
	return status;
}
/**************************************************************************************************
** IN:					uint8_t sel_code          cascade level
**              uint8_t bitcount          known bit count
**              uint8_t *snr        			unkown part of uid
** OUT:					uint8_t *snr        			uid
**************************************************************************************************/
uint8_t PICC_CascAnticollA(uint8_t sel_code, uint8_t bitcount, volatile uint8_t *snr)
{
	uint8_t status = STATUS_SUCCESS;
	uint8_t buffer[8];	
	uint8_t i;
	uint8_t complete = 0;                // indicatates whether uid complete: 0=not complete
	uint8_t nbits    = 0;
	uint8_t nbytes   = 0;
	uint8_t byteOffset;

	// bits after collision are set to 0
	RC663_RegSet(RxBitCtrlReg, 0x0);
	// disable TxCRC and RxCRC
	RC663_RegSet(TxCrcPresetReg, 0x18);
	RC663_RegSet(RxCrcConReg, 0x18);
	// enable Tx/Rx parity
	RC663_RegSet(FrameConReg, 0xCF);
	
	/* NOTE: MInfo.collPos = 0 means collision at 1st bit */
	
	while(!complete && (status == STATUS_SUCCESS))
	{
		// bitcount will not exceed 32 unless there is protocol error
		if(bitcount > 32)
		{
			status = STATUS_INVALID_PARAMETER;
			continue;
		}
		nbits = bitcount % 8;
		nbytes = bitcount / 8;

		memset(buffer, 0, 7);
		ResetInfo();
		
		buffer[0] = sel_code;
		// NVB : complete bytes + bits
		buffer[1] = 0x20 + (nbytes << 4) + nbits;
		if(nbits)
		{
			nbytes++;
		}
		for(i=0; i<nbytes; i++)
		{
			buffer[2+i] = snr[i];  
		}
		// valid bits to be sent in last byte
		RC663_RegSet(TxDataNumReg, (0x08 | nbits));
		// bit position for the first received bit
		// bits after collision are marked as 0
		RC663_RegSet(RxBitCtrlReg, (nbits << 4));

		MInfo.nBytesToSend   = nbytes + 2;

		SetTimeOut4p7us(TIMEOUT_RC6xx_Timer_4p7_AntiCollisionA);
		tmo = TIMEOUT_sw_AntiCollisionA;

		status = PcdCmd(PCD_TRANSCEIVE, buffer);

		if(status == STATUS_COLLISION_ERROR || status == STATUS_SUCCESS)
		{
			byteOffset = 0;
			
			if(nbits)
			{// the last transmitting byte = the first receiving byte
				snr[nbytes - 1] |= buffer[0];
				byteOffset++;
			}
			// copy received data 
			for(i=0;i<(4-nbytes);i++)
			{
				snr[nbytes + i] = buffer[i + byteOffset];
			}
			if(status == STATUS_COLLISION_ERROR)
			{
				bitcount = bitcount + MInfo.collPos - nbits + 1;
				status = STATUS_SUCCESS;
				if(bitcount > 40)
				{
					status = STATUS_BITCOUNT_ERROR;
					continue;
				}				
			} 
			else
			{
				if((snr[0] ^ snr[1] ^ snr[2] ^ snr[3]) != buffer[i + byteOffset])
				{
					status = STATUS_WRONG_UID_CHECKBYTE;
					continue;
				}
				complete=1;
			}
		}
	}

	RC663_RegSet(TxDataNumReg, 0x08);
	RC663_RegSet(RxBitCtrlReg, 0x80);
	return status;
}
/**************************************************************************************************
** IN:			    uint8_t sel_code      cascade level
**              uint8_t *snr					UID
** OUT:					uint8_t *sak 					SAK
**************************************************************************************************/
uint8_t PICC_CascSelectA(uint8_t sel_code, volatile uint8_t *snr,  uint8_t *sak)
{
	uint8_t status = STATUS_SUCCESS;
	uint8_t buffer[8];	

	// enable Tx/Rx CRC
	RC663_RegSet(TxCrcPresetReg, 0x19);
	RC663_RegSet(RxCrcConReg, 0x19);
	// enable Tx/Rx parity
	RC663_RegSet(FrameConReg, 0xCF);

	buffer[0] = sel_code;                    
	buffer[1] = 0x70;

	buffer[2] = snr[0];                 
	buffer[3] = snr[1];
	buffer[4] = snr[2];
	buffer[5] = snr[3];
    
	buffer[6] = (uint8_t)(snr[0] ^ snr[1] ^ snr[2] ^ snr[3]);   // BCC

	ResetInfo();
	MInfo.nBytesToSend = 7;

	SetTimeOut4p7us(TIMEOUT_RC6xx_Timer_4p7_SelectA);
	tmo = TIMEOUT_sw_SelectA;

	status = PcdCmd(PCD_TRANSCEIVE, buffer);

	if(status == STATUS_SUCCESS)
	{
		if(MInfo.nBytesReceived == 1 && MInfo.nBitsReceived == 0)
		{
			*sak = buffer[0];
		}
		else
		{
			status = STATUS_BITCOUNT_ERROR;
		}
	}
	return status;
}
/**************************************************************************************************
** void
**************************************************************************************************/
uint8_t PICC_HaltA(void)
{
	uint8_t status = STATUS_SUCCESS;
	uint8_t buffer[2];
		
//	// enable Tx/Rx CRC
//	RC663_RegSet(TxCrcPresetReg, 0x19);
//	RC663_RegSet(RxCrcConReg, 0x19);
//	// enable Tx/Rx parity
//	RC663_RegSet(FrameConReg, 0xCF);
	
	buffer[0] = 0x50;
	buffer[1] = 0x0;

	ResetInfo();
	MInfo.nBytesToSend   = 2;
	
	SetTimeOut4p7us(TIMEOUT_RC6xx_Timer_4p7_HaltA);
	tmo = TIMEOUT_sw_HaltA;
	
	status = PcdCmd(PCD_TRANSCEIVE, buffer);

	if((MInfo.irq0 & 0x08) && (status == STATUS_IO_TIMEOUT))	// transmit successfully and with no response in 1ms
		status = STATUS_SUCCESS;
	
	return status;
}

/**************************************************************************************************
** IN:						uint8_t reqa				Request type, 0x26 or 0x52
** OUT:						uint8_t *atqa				ATQA buffer
** 								uint8_t *uid				UID buffer
**								uint8_t *sak				SAK address
**								uint8_t *uid_size		UID size, can be 4 or 7, do not implement 10
** FUNC:					Active one single card
**************************************************************************************************/
uint8_t PICC_CardActiveA(uint8_t req, uint8_t *atqa, uint8_t *uid, uint8_t *uid_size, uint8_t *sak)
{
	uint8_t status = MI_OK;
	*uid_size = 4;
				
	status = PICC_RequestA(req, atqa);
	if(status == MI_OK)
	{
        status = PICC_CascAnticollA(0x93, 0, uid);
        if (status == MI_OK) 
        {
            status = PICC_CascSelectA(0x93, uid, sak);
            if (status == MI_OK)  
            {
                if(*sak & 0x04)	
                {
                    // bit3 of SAK - UID not complete
                    *uid=*(uid+1);
                    *(uid+1)=*(uid+2);
                    *(uid+2)=*(uid+3);
                    status = PICC_CascAnticollA(0x95, 0, uid+3); // level 2
                    if(status == MI_OK) 
                    {
                        status = PICC_CascSelectA(0x95, uid+3, sak);	// level 2
                    }
                    *uid_size = 7;
                }
            }
        }
	}
	return status;
}

// end file
