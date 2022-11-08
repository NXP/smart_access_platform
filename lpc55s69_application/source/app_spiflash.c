/*
 * Copyright (c) 2017 - 2018 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "app.h"

#include "fsl_clock.h"
#include "fsl_spi.h"
#include "app_spiflash.h"

#define SPIFLASH_CS_LOW()      GPIO_PinWrite(GPIO, FLASHCS_PORT,  FLASHCS_PIN,  0u)
#define SPIFLASH_CS_HIGH()     GPIO_PinWrite(GPIO, FLASHCS_PORT,  FLASHCS_PIN,  1u)

#define W25QXX_DUMMY_BYTE      0xA5

/**
 * @brief   spiflash_delayus() --flash operation delays.
 * @param   times   : Desired delay time / uS
 * @return  return NULL.
 */
volatile uint32_t g_SPIFlashDelayUs1, g_SPIFlashDelayUs2;
void spiflash_delayus(uint32_t times)
{
    for (g_SPIFlashDelayUs1 = 0; g_SPIFlashDelayUs1 < times; g_SPIFlashDelayUs1++)
    {
        for (g_SPIFlashDelayUs2 = 0; g_SPIFlashDelayUs2 < 12; g_SPIFlashDelayUs2++)
        {
            ;
        }
    }
}

/**
 * @brief   
 * @param   
 * @return  
 */
uint8_t spiflash_write_reg(uint8_t reg)
{
    /* clear tx/rx errors and empty FIFOs */
    FLASH_SPI->FIFOCFG  |= SPI_FIFOCFG_EMPTYTX_MASK | SPI_FIFOCFG_EMPTYRX_MASK;
    FLASH_SPI->FIFOSTAT |= SPI_FIFOSTAT_TXERR_MASK | SPI_FIFOSTAT_RXERR_MASK;
    FLASH_SPI->FIFOWR    = 0x07300000|reg;
    /* wait if TX FIFO of previous transfer is not empty */
    while ((FLASH_SPI->FIFOSTAT & SPI_FIFOSTAT_RXNOTEMPTY_MASK) == 0)
    {
        ;
    }
    return (FLASH_SPI->FIFORD)&0x000000FF;
}

/**
 * @brief   
 * @param   
 * @return  
 */
uint16_t spiflash_write_data(uint16_t data)
{
    /* clear tx/rx errors and empty FIFOs */
    FLASH_SPI->FIFOCFG  |= SPI_FIFOCFG_EMPTYTX_MASK | SPI_FIFOCFG_EMPTYRX_MASK;
    FLASH_SPI->FIFOSTAT |= SPI_FIFOSTAT_TXERR_MASK | SPI_FIFOSTAT_RXERR_MASK;
    FLASH_SPI->FIFOWR    = 0x0F300000|data;
    /* wait if TX FIFO of previous transfer is not empty */
    while ((FLASH_SPI->FIFOSTAT & SPI_FIFOSTAT_RXNOTEMPTY_MASK) == 0)
    {
        ;
    }
    return (FLASH_SPI->FIFORD)&0x0000FFFF;
}


/**
 * @brief   
 * @param   
 * @return  
 */
uint16_t spiflash_read_data(void)
{
    /* clear tx/rx errors and empty FIFOs */
    FLASH_SPI->FIFOCFG  |= SPI_FIFOCFG_EMPTYTX_MASK | SPI_FIFOCFG_EMPTYRX_MASK;
    FLASH_SPI->FIFOSTAT |= SPI_FIFOSTAT_TXERR_MASK | SPI_FIFOSTAT_RXERR_MASK;
    FLASH_SPI->FIFOWR    = 0x0F3000FF;
    /* wait if TX FIFO of previous transfer is not empty */
    while ((FLASH_SPI->FIFOSTAT & SPI_FIFOSTAT_RXNOTEMPTY_MASK) == 0)
    {
        ;
    }
    return (FLASH_SPI->FIFORD)&0x0000FFFF;
}

/**
 * @brief   
 * @param   
 * @return  
 */
void spiflash_readid(uint32_t* id)
{
    uint8_t  temp[3];
    
    SPIFLASH_CS_LOW();
    spiflash_delayus(100);
    spiflash_write_reg(0x9F);
    temp[0] = spiflash_write_reg(W25QXX_DUMMY_BYTE);
    temp[1] = spiflash_write_reg(W25QXX_DUMMY_BYTE);
    temp[2] = spiflash_write_reg(W25QXX_DUMMY_BYTE);
    SPIFLASH_CS_HIGH();
    * id = (temp[0] << 16) | (temp[1] << 8) | temp[2];
    spiflash_delayus(10);
}

/**
 * @brief   
 * @param   
 * @return  
 */
void spiflash_write_enable(void)
{
    SPIFLASH_CS_LOW();
    spiflash_delayus(100);
    spiflash_write_reg(0x06);
    SPIFLASH_CS_HIGH();
    spiflash_delayus(5);
}

/**
 * @brief   
 * @param   
 * @return  
 */
void spiflash_write_disable(void)
{
    SPIFLASH_CS_LOW();
    spiflash_delayus(100);
    spiflash_write_reg(0x04);
    SPIFLASH_CS_HIGH();
    spiflash_delayus(5);
}

/**
 * @brief   
 * @param   
 * @return  
 */
void spiflash_write_waitend(void)
{
    uint8_t reg;
    SPIFLASH_CS_LOW();
    spiflash_delayus(10);
    spiflash_write_reg(0x05);
    do 
    {
        reg = spiflash_write_reg(W25QXX_DUMMY_BYTE);
        spiflash_delayus(100);
    }
    while( (reg&0x01) == 0x01 );
    SPIFLASH_CS_HIGH();
    spiflash_delayus(5);
}

/**
 * @brief   
 * @param   
 * @return  
 */
void spiflash_sector_erase(uint32_t sector)
{
    spiflash_delayus(10);
#ifdef SPIFLASH_DEBUG
    PRINTF("Flash EraseSector %x Begin...\r\n", sector);
#endif
    spiflash_write_waitend();
    sector = sector * FLASH_SECTOR_SIZE;
    spiflash_write_enable();
    SPIFLASH_CS_LOW();
    spiflash_delayus(100);
    spiflash_write_reg(0x20);
  
    spiflash_write_reg(sector>>16);
    spiflash_write_reg(sector>>8);
    spiflash_write_reg(sector);
    SPIFLASH_CS_HIGH();
    spiflash_delayus(5);
    spiflash_write_waitend();
#ifdef SPIFLASH_DEBUG
    PRINTF("Flash EraseSector done\r\n");
#endif
    spiflash_delayus(120000);
}

/**
 * @brief   
 * @param   
 * @return  
 */
void spiflash_chip_erase(void)
{
    spiflash_delayus(5);
#ifdef SPIFLASH_DEBUG
    PRINTF("Flash Chip Erase Begin...\r\n");
#endif
    spiflash_write_waitend();
    spiflash_write_enable();
    SPIFLASH_CS_LOW();
    spiflash_delayus(100);
    spiflash_write_reg(0xC7);
    SPIFLASH_CS_HIGH();
    
    spiflash_write_waitend();
#ifdef SPIFLASH_DEBUG
    PRINTF("Flash Chip Erase Done\r\n");
#endif
    spiflash_delayus(5);
}

/**
 * @brief   
 * @param   
 * @return  
 */
void spiflash_page_read(uint32_t page, uint8_t *buf)
{
    uint32_t i;
    page = page * FLASH_PAGE_SIZE;
#ifdef SPIFLASH_DEBUG
    PRINTF("Flash Chip Read Begin 0x%x...\r\n", page);
#endif
    SPIFLASH_CS_LOW();
    spiflash_delayus(100);
    
    spiflash_write_reg(0x0B);
    spiflash_write_reg(page>>16);
    spiflash_write_reg(page>>8);
    spiflash_write_reg(page);
  
    spiflash_write_reg(0x00);        /* Dummy */
    
    for(i=0; i<(FLASH_PAGE_SIZE); i++)
    {
        *buf = spiflash_write_reg(0xFF);
        buf++;
    }
    SPIFLASH_CS_HIGH();
#ifdef SPIFLASH_DEBUG
    PRINTF("Flash Read Page Done\r\n");
#endif
}

/**
 * @brief   
 * @param   
 * @return  
 */
void spiflash_page_write(uint32_t page, uint8_t *buf)
{
    uint32_t i;
    spiflash_delayus(5);
    page = page * FLASH_PAGE_SIZE;
#ifdef SPIFLASH_DEBUG
    PRINTF("Flash Page Write Begin 0x%x...\r\n", page);
#endif
    spiflash_write_waitend();
    spiflash_write_enable();

    SPIFLASH_CS_LOW();
    spiflash_delayus(100);
    spiflash_write_reg(0x02);
    spiflash_write_reg(page>>16);
    spiflash_write_reg(page>>8);
    spiflash_write_reg(page);
  
    for(i=0; i<(FLASH_PAGE_SIZE); i++)
    {
        spiflash_write_reg(*buf);
        buf++;
    }

    SPIFLASH_CS_HIGH();
    spiflash_delayus(5);
    spiflash_write_waitend();
#ifdef SPIFLASH_DEBUG
    PRINTF("Flash Write Page Done\r\n");
#endif
}



/**
 * @brief   
 * @param   
 * @return  
 */
void spiflash_sector_read(uint32_t sector, uint8_t *buf)
{
    uint32_t i;
#ifdef SPIFLASH_DEBUG
    PRINTF("Flash Sector Read Begin 0x%d...\r\n", sector);
#endif
    sector = sector*16;
    for(i=0; i<16; i++)
    {
        spiflash_page_read(sector, buf);
        sector++;
        buf += FLASH_PAGE_SIZE;
    }
#ifdef SPIFLASH_DEBUG
    PRINTF("Flash Sector Read Done\r\n");
#endif
}

/**
 * @brief   
 * @param   
 * @return  
 */
void spiflash_sector_write(uint32_t sector, uint8_t *buf)
{
    uint32_t i;
#ifdef SPIFLASH_DEBUG
    PRINTF("Flash Write Sector Begin 0x%x...\r\n", sector);
#endif
    sector = sector*16;
    for(i=0; i<16; i++)
    {
        spiflash_page_write(sector, buf);
        sector++;
        buf += FLASH_PAGE_SIZE;
    }
#ifdef SPIFLASH_DEBUG
    PRINTF("Flash Write Sector Done\r\n");
#endif
}

/**
 * @brief   capt_delayus() -- Captouch sensor read delays.
 * @param   times   : Desired delay time / uS
 * @return  return NULL.
 */
void spiflash_init(void)
{
    uint32_t id;
    /* Initialize MCU SPI interface */
    gpio_pin_config_t   gpioPinConfig;
    
    /* Enables the clock for the I/O controller.: Enable Clock. */
    CLOCK_EnableClock(kCLOCK_Iocon);
    /* SPI */
    IOCON_PinMuxSet(IOCON, FLASHCS_PORT, FLASHCS_PIN, FLASHCS_FUNC | IOCON_PIO_DIGIMODE_MASK);
    IOCON_PinMuxSet(IOCON, FLASHMI_PORT, FLASHMI_PIN, FLASHMI_FUNC | IOCON_MODE_PULLUP | IOCON_PIO_DIGIMODE_MASK);
    IOCON_PinMuxSet(IOCON, FLASHMO_PORT, FLASHMO_PIN, FLASHMO_FUNC | IOCON_MODE_PULLUP | IOCON_PIO_DIGIMODE_MASK);
    IOCON_PinMuxSet(IOCON, FLASHCK_PORT, FLASHCK_PIN, FLASHCK_FUNC | IOCON_MODE_PULLUP | IOCON_PIO_DIGIMODE_MASK);
    
    /* Disables the clock for the I/O controller.: Disable Clock. To Save Power */
    CLOCK_DisableClock(kCLOCK_Iocon);
    
    gpioPinConfig.pinDirection = kGPIO_DigitalOutput;
    gpioPinConfig.outputLogic  = 1u; /* output high as default. */

//    GPIO_PinInit (GPIO, FLASHPWR_PORT, FLASHPWR_PIN, &gpioPinConfig);
//    GPIO_PinWrite(GPIO, FLASHPWR_PORT, FLASHPWR_PIN, 1u);
    
    GPIO_PinInit (GPIO, FLASHCS_PORT,  FLASHCS_PIN,  &gpioPinConfig);
    GPIO_PinWrite(GPIO, FLASHCS_PORT,  FLASHCS_PIN,  1u);
    
    spi_master_config_t userConfig = {0};
    /* attach 12 MHz clock to NFC Reader's SPI */
    CLOCK_AttachClk(FLASH_SPI_CLKATTACH);
    /* reset FLEXCOMM for SPI */
    RESET_PeripheralReset(FLASH_SPI_RST);
    
    SPI_MasterGetDefaultConfig(&userConfig);
    userConfig.baudRate_Bps = 6000000UL;
    userConfig.sselNum = (spi_ssel_t)0;
    userConfig.sselPol = (spi_spol_t)kSPI_SpolActiveAllLow;
    SPI_MasterInit(FLASH_SPI, &userConfig, FLASH_SPI_CLKFREQ);
    //GPIO_PinWrite(GPIO, FLASHPWR_PORT, FLASHPWR_PIN, 0u);
    
    /* Open SPI Flash Power */
    spiflash_readid(&id);
#ifdef SPIFLASH_DEBUG
    PRINTF("Flash ID is %x\r\n", id);
#endif
    PRINTF("Flash ID is %x\r\n", id);
}

/**
 * @brief   capt_delayus() -- Captouch sensor read delays.
 * @param   times   : Desired delay time / uS
 * @return  return NULL.
 */
void spiflash_test(void)
{
    uint8_t *TestBuf = 0;
    TestBuf = malloc(4096);
#ifdef SPIFLASH_DEBUG
    if(TestBuf != NULL)
    {
        PRINTF("FLASH Malloc Test Buffer Done\r\n");
    }
    else
    {
        PRINTF("FLASH Malloc Test Buffer Failed\r\n");
    }
#endif
    memset(&TestBuf[0],    0xA5, 1024);
    memset(&TestBuf[1024], 0x5A, 1024);
    memset(&TestBuf[2048], 0xCC, 1024);
    memset(&TestBuf[3072], 0xDD, 1024);
    
    spiflash_sector_erase(0);
    spiflash_sector_write(0, TestBuf);
    memset(TestBuf, 0x00, 4096);
    spiflash_sector_read(0, TestBuf);
    
    free(TestBuf);
}

/**
 * @brief   capt_delayus() -- Captouch sensor read delays.
 * @param   times   : Desired delay time / uS
 * @return  return NULL.
 */
void spiflash_write_file(uint32_t position, uint8_t* buf, uint32_t len)
{
    uint32_t i;
    for(i=0; i<(len/FLASH_SECTOR_SIZE+1); i++)
    {
        spiflash_sector_erase(position);
        spiflash_sector_write(position, buf);
        position++;
        buf += FLASH_SECTOR_SIZE;
    }
}

/**
 * @brief   capt_delayus() -- Captouch sensor read delays.
 * @param   times   : Desired delay time / uS
 * @return  return NULL.
 */
void spiflash_read_file(uint32_t position, uint8_t* buf, uint32_t len)
{
    uint32_t i;
    for(i=0; i<(len/FLASH_SECTOR_SIZE+1); i++)
    {
        spiflash_sector_read(position, buf);
        position++;
        buf += FLASH_SECTOR_SIZE;
    }
}
