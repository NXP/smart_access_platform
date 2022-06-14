/*
 * A reference code for CLRC663/MFRC631/MFRC630
 *
 */
#ifndef __ISO14443_3A_H__
#define __ISO14443_3A_H__

#include "stdint.h"

/**************************************************************************************************
** FUNC:				set chip to work at ISO14443 TypeA 106kbps protocol
**************************************************************************************************/
uint8_t PICC_PcdTypeA106Config(void);

/**************************************************************************************************
** IN:					uint8_t req_code          REQALL=0x52¡¢REQIDLE=0x26
** OUT:			    uint8_t *atq              ATQA
**************************************************************************************************/
uint8_t PICC_RequestA(uint8_t req_code, uint8_t *atq);

/**************************************************************************************************
** IN:					uint8_t sel_code          cascade level
**              uint8_t bitcount          known bit count
**              uint8_t *snr        			unkown part of uid
** OUT:					uint8_t *snr        			uid
**************************************************************************************************/
uint8_t PICC_CascAnticollA(uint8_t sel_code, uint8_t bitcount, volatile uint8_t *snr);

/**************************************************************************************************
** IN:			    uint8_t sel_code      cascade level
**              uint8_t *snr					UID
** OUT:					uint8_t *sak 					SAK
**************************************************************************************************/
uint8_t PICC_CascSelectA(uint8_t sel_code, volatile uint8_t *snr,  uint8_t *sak);

/**************************************************************************************************
** void
**************************************************************************************************/
uint8_t PICC_HaltA(void);

/**************************************************************************************************
** IN:						uint8_t reqa				Request type, 0x26 or 0x52
** OUT:						uint8_t *atqa				ATQA buffer
** 								uint8_t *uid				UID buffer
**								uint8_t *sak				SAK address
**								uint8_t *uid_size		UID size, can be 4 or 7, do not implement 10
** FUNC:					Active one single card
**************************************************************************************************/
uint8_t PICC_CardActiveA(uint8_t req, uint8_t *atqa, uint8_t *uid, uint8_t *uid_size, uint8_t *sak);


#endif
