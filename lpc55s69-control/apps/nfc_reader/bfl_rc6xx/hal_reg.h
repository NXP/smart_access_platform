/*
* Hardware related file, need to be re-write according to the selected MCU	
*/
#include "stdint.h"

void i2c_init(void);

extern char PcdConfig(void);

extern void RC663_Delayms(uint32_t time);


/**************************************************************************************************
** IN:			    uint8_t regAddr			Register address
** 							uint8_t * buff			in-data address
** 							uint8_t num					number of bytes to be written
** FUNC:				MCU related(low level) write register
**************************************************************************************************/
void iic_Write_reg(uint8_t reg_add, uint8_t *buff, uint8_t num);


/**************************************************************************************************
** IN:			    uint8_t reg_add			Register address
**  						uint8_t *Read				buffer address of data output
** 							uint8_t num					number of bytes to be read
** FUNC:				MCU related(low level) read register
**************************************************************************************************/
void iic_Read_reg(uint8_t reg_add, uint8_t *Read, uint8_t num);


/**************************************************************************************************
** IN:			    uint8_t regAddr			Register address
** OUT:					register value
** FUNC:				read data from register
**************************************************************************************************/
uint8_t  RC663_RegRead(uint8_t regAddr);


/**************************************************************************************************
** IN:			    uint8_t regAddr			Register address
**							uint8_t rVal				Register value to be written
** FUNC:				write data into register
**************************************************************************************************/
void RC663_RegSet(uint8_t regAddr,uint8_t regVal);


/**************************************************************************************************
** IN:			    uint8_t regAddr			Register address
**							uint8_t ModifyVal		Register value to be modified: 1=set / 0=clear
**							uint8_t MaskByte		mask byte value
** OUT:					-
**************************************************************************************************/
void RcModifyReg(uint8_t RegAddr,uint8_t ModifyVal,uint8_t MaskByte);


/**************************************************************************************************
** IN:			    uint8_t *DataBfr		Output data buffer
**							uint8_t Len					number of bytes to be read from FIFO
**************************************************************************************************/
void ReadFIFO(uint8_t *DataBfr,uint8_t Len);


/**************************************************************************************************
** IN:			    uint8_t *DataBfr		Input data buffer
**							uint8_t Len					number of bytes to be written
**************************************************************************************************/
void WriteFIFO(uint8_t *DataBfr,uint8_t Len);


/**************************************************************************************************
** init GPIOs connected to RC663 
**************************************************************************************************/
void mcu_pin_init(void) ;


/**************************************************************************************************
** Reset the chip
** _______   1ms   __________
**        |_______|
**************************************************************************************************/
void PcdReset(void);


/**************************************************************************************************
** Set chip to powerdown mode
**************************************************************************************************/
void PcdStop(void);

extern void RC663_RegSeqRead(uint8_t regAddr, uint8_t *buff, uint8_t num);
extern void RC663_RegSeqWrite(uint8_t regAddr, uint8_t *buff, uint8_t num);
extern void RC663_DeInit(void);
extern void RC663_ResetSet(uint8_t high);


