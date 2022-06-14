/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __APP_SPIFLASH_H__
#define __APP_SPIFLASH_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/
//#define SPIFLASH_DEBUG      0x1

/* Flash Control */
#define FLASH_SPI                  SPI8
#define FLASH_SPI_RST              kHSLSPI_RST_SHIFT_RSTn
#define FLASH_SPI_CLKATTACH        kFRO12M_to_HSLSPI
#define FLASH_SPI_CLKFREQ          12000000UL
#define FLASH_SPI_CLKSRC           kCLOCK_Flexcomm8

#define FLASHCS_PORT               0u
#define FLASHCS_PIN                20u
#define FLASHCS_FUNC               IOCON_FUNC0

#define FLASHMI_PORT               1u
#define FLASHMI_PIN                3u
#define FLASHMI_FUNC               IOCON_FUNC6

#define FLASHMO_PORT               0u
#define FLASHMO_PIN                26u
#define FLASHMO_FUNC               IOCON_FUNC9

#define FLASHCK_PORT               1u
#define FLASHCK_PIN                2u
#define FLASHCK_FUNC               IOCON_FUNC6

#define FLASH_SECTOR_SIZE          0x1000     // 4KByte - Sector
#define FLASH_SECTOR_NUM           128

#define FLASH_PAGE_SIZE            0x100      // 256Byte - Page

extern void spiflash_init(void);
extern void spiflash_sector_write(uint32_t sector, uint8_t *buf);
extern void spiflash_sector_read(uint32_t sector, uint8_t *buf);
extern void spiflash_chip_erase(void);
extern void spiflash_sector_erase(uint32_t sector);
extern void spiflash_readid(uint32_t* id);
extern void spiflash_test(void);

extern void spiflash_write_file(uint32_t position, uint8_t* buf, uint32_t len);
extern void spiflash_read_file(uint32_t position, uint8_t* buf, uint32_t len);

#endif

// End file __APP_SPIFLASH_H__
