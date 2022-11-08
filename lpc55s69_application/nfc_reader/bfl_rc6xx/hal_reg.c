/*
* Hardware related file, need to be re-write according to the selected MCU
*	
*/

// C Standard Lib includes
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "rc663_config.h"
#include "hal_reg.h"
#include "rc663.h"
#include "status_code.h"
#include "iso14443_3a.h"

#include "drv_nfcreader663.h"

/**************************************************************************************************
** IN:			    uint8_t regAddr			Register address
**							uint8_t ModifyVal		Register value to be modified: 1=set / 0=clear
**							uint8_t MaskByte		mask byte value
** OUT:					-
**************************************************************************************************/
void RcModifyReg(uint8_t RegAddr,uint8_t ModifyVal,uint8_t MaskByte)
{
    uint8_t RegVal;

    RegVal = RC663_RegRead(RegAddr);
    if(ModifyVal)
    {
        RegVal |= MaskByte;
    }
    else
    {
        RegVal &= (~MaskByte);
    }
    RC663_RegSet(RegAddr, RegVal);
}


/**************************************************************************************************
** IN:			    uint8_t *DataBfr		Output data buffer
**							uint8_t Len					number of bytes to be read from FIFO
**************************************************************************************************/
void ReadFIFO(uint8_t *DataBfr,uint8_t Len)
{
	RC663_RegSeqRead(FIFODataReg, DataBfr, Len);
}

/**************************************************************************************************
** IN:			    uint8_t *DataBfr		Input data buffer
**							uint8_t Len					number of bytes to be written
**************************************************************************************************/
void WriteFIFO(uint8_t *DataBfr,uint8_t Len)
{
	RC663_RegSeqWrite(FIFODataReg, DataBfr, Len);
}

/**************************************************************************************************
** Reset the chip
** 			   _______  
** _______|  1ms  |____________
**************************************************************************************************/
void PcdReset(void)
{
    // GPIO = HIGH
    RC663_ResetSet(1);
    // delay 1ms appr.
    RC663_Delayms(10);
    // GPIO = LOW
    RC663_ResetSet(0);
    // delay 1ms appr.
    RC663_Delayms(10);
}

/**************************************************************************************************
** Set chip to powerdown mode
**************************************************************************************************/
void PcdStop(void)
{	
    // Rc663 Power down
    RC663_ResetSet(1);
    // disable I2C
    RC663_DeInit();
}

// end file
