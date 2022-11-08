/*
 * Copyright (c) 2017 - 2021 , NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "stdint.h"
#include "rc663_config.h"
#include "hal_reg.h"
#include "rc663.h"
#include "status_code.h"
#include "iso14443_3a.h"

#include "app_printf.h"
#include "drv_nfcreader663.h"

#include "fsl_common.h"
#include "fsl_clock.h"
#include "fsl_gpio.h"
#include "fsl_spi.h"
#include "fsl_inputmux.h"
#include "fsl_iocon.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

#define NFCCMDBUFLEN   512
volatile uint8_t  g_rc663TxBuffer[NFCCMDBUFLEN];
#if RC663_USE_SPI
volatile uint8_t  g_rc663RxBuffer[NFCCMDBUFLEN];
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/
/**
 * @brief   NFC mS delay
 * @param   delay times / mS
 * @return  NULL
 */
volatile uint32_t s_delayNFCMsCnt1, s_delayNFCMsCnt2;
void RC663_Delayms(uint32_t times)
{
    for (s_delayNFCMsCnt1 = 0; s_delayNFCMsCnt1 < times; s_delayNFCMsCnt1++)
    {
        for (s_delayNFCMsCnt2 = 0; s_delayNFCMsCnt2 < (11000); s_delayNFCMsCnt2++)
        {
            ;
        }
    }
}

volatile uint32_t s_delayNFCUsCnt1, s_delayNFCUsCnt2;
void RC663_Delayus(uint32_t times)
{
    for (s_delayNFCUsCnt1 = 0; s_delayNFCUsCnt1 < times; s_delayNFCUsCnt1++)
    {
        for (s_delayNFCUsCnt2 = 0; s_delayNFCUsCnt2 < (10); s_delayNFCUsCnt2++)
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
void RC663_DataPrint(uint8_t* tx_buf, uint32_t txcnt)
{
    uint32_t i;
    for(i=0; i<txcnt; i++)
    {
        PRINTF("0x%x ",  *tx_buf);
        tx_buf++;
    }
    PRINTF("\r\n");
}

/**
 * @brief   RC663_ResetSet
 * @param   
 * @return  
 */
void RC663_ResetSet(uint8_t high)
{
    GPIO_PinWrite(GPIO, NFCRST_PORT, NFCRST_PIN, high);
}

/**
 * @brief   RC663_DeInit
 * 			disable i2c int and disable i2c clock
 * 			disable spi int and disable spi clock
 * @param   
 * @return  
 */
void RC663_DeInit(void)
{
}

#if RC663_SPI_SIM
/**
 * @brief   
 * @param   
 * @return  
 */
static uint8_t RC663_SPIGPIOSend(uint8_t data)
{
	uint8_t val;
    int i;

    val = 0x00;
    for(i=7; i>=0; i--)
    {
        if(((data>>i)&0x01) == 0x01)
        {
            GPIO_PinWrite(GPIO, NFCMO_PORT, NFCMO_PIN, 1 );
        }
        else
        {
            GPIO_PinWrite(GPIO, NFCMO_PORT, NFCMO_PIN, 0 );
        }
        RC663_Delayus(100);
        GPIO_PinWrite(GPIO, NFCSCK_PORT, NFCSCK_PIN, 0);
        RC663_Delayus(80);
        GPIO_PinWrite(GPIO, NFCSCK_PORT, NFCSCK_PIN, 1);
        RC663_Delayus(15);
        val = val << 1;
        if(GPIO_PinRead(GPIO, NFCMI_PORT, NFCMI_PIN) == 1)
        {
            val |= 0x01;
        }
        else
        {
            val &= 0xFE;
        }
    }
    
    GPIO_PinWrite(GPIO, NFCMO_PORT,  NFCMO_PIN,  1);
    GPIO_PinWrite(GPIO, NFCSCK_PORT, NFCSCK_PIN, 1);
    
	return val;
}
#endif

/**
 * @brief   
 * @param   
 * @return  
 */
void RC663_RegSet(uint8_t regAddr, uint8_t regVal)
{
    uint32_t i;
    g_rc663TxBuffer[0] = (regAddr<<1)&0xFE;
	g_rc663TxBuffer[1] = regVal;
    
    GPIO_PinWrite(GPIO, NFCSEL_PORT, NFCSEL_PIN, 0);
	for(i = 0; i < 2; i++)
	{
#if RC663_SPI_SIM
        g_rc663RxBuffer[i] = RC663_SPIGPIOSend(g_rc663TxBuffer[i]);
#else
        /* clear tx/rx errors and empty FIFOs */
        NFCREADER_SPI->FIFOCFG  |= SPI_FIFOCFG_EMPTYTX_MASK | SPI_FIFOCFG_EMPTYRX_MASK;
        NFCREADER_SPI->FIFOSTAT |= SPI_FIFOSTAT_TXERR_MASK | SPI_FIFOSTAT_RXERR_MASK;
        NFCREADER_SPI->FIFOWR    = 0x07300000|g_rc663TxBuffer[i];
        /* wait if TX FIFO of previous transfer is not empty */
        while ((NFCREADER_SPI->FIFOSTAT & SPI_FIFOSTAT_RXNOTEMPTY_MASK) == 0)
        {
            ;
        }
        g_rc663RxBuffer[i] = (NFCREADER_SPI->FIFORD)&0x000000FF; 
#endif
	}
    GPIO_PinWrite(GPIO, NFCSEL_PORT, NFCSEL_PIN, 1);
}

/**
 * @brief   
 * @param   
 * @return  
 */
void RC663_RegSeqWrite(uint8_t regAddr, uint8_t *buff, uint8_t num)
{
	uint8_t i;
    
    memset((void *)g_rc663TxBuffer, 0x00, NFCCMDBUFLEN);
    g_rc663TxBuffer[0] = (regAddr<<1)&0xFE;
	for(i=0; i<num; i++)
    {
		g_rc663TxBuffer[i+1] = *buff;
		buff++;
    }
    
    GPIO_PinWrite(GPIO, NFCSEL_PORT, NFCSEL_PIN, 0);
	for(i = 0; i < (num+1); i++)
	{  
#if RC663_SPI_SIM
        g_rc663RxBuffer[i] = RC663_SPIGPIOSend(g_rc663TxBuffer[i]);
#else
        /* clear tx/rx errors and empty FIFOs */
        NFCREADER_SPI->FIFOCFG  |= SPI_FIFOCFG_EMPTYTX_MASK | SPI_FIFOCFG_EMPTYRX_MASK;
        NFCREADER_SPI->FIFOSTAT |= SPI_FIFOSTAT_TXERR_MASK | SPI_FIFOSTAT_RXERR_MASK;
        NFCREADER_SPI->FIFOWR    = 0x07300000|g_rc663TxBuffer[i];
        /* wait if TX FIFO of previous transfer is not empty */
        while ((NFCREADER_SPI->FIFOSTAT & SPI_FIFOSTAT_RXNOTEMPTY_MASK) == 0)
        {
            ;
        }
        g_rc663RxBuffer[i] = (NFCREADER_SPI->FIFORD)&0x000000FF; 
#endif
	}
    GPIO_PinWrite(GPIO, NFCSEL_PORT, NFCSEL_PIN, 1);

    RC663_Delayus(1);
}

/**
 * @brief   
 * @param   
 * @return  
 */
void RC663_RegSeqRead(uint8_t regAddr, uint8_t *buff, uint8_t num)
{
	volatile uint32_t i;
    volatile uint8_t  g_rc663SPIDataTemp = 0;

    memset((void *)g_rc663TxBuffer, 0x00, NFCCMDBUFLEN);
	for(i=0;i<(num+1);i++)
	{
		g_rc663TxBuffer[i] = ((regAddr)<<1)|0x01;
	}
	g_rc663TxBuffer[i] = 0x00;

    GPIO_PinWrite(GPIO, NFCSEL_PORT, NFCSEL_PIN, 0);
	for(i = 0; i < (num+2); i++)
	{  
#if RC663_SPI_SIM
        Read[i] = RC663_SPIGPIOSend(g_rc663TxBuffer[i]);
#else
        /* clear tx/rx errors and empty FIFOs */
        NFCREADER_SPI->FIFOCFG  |= SPI_FIFOCFG_EMPTYTX_MASK | SPI_FIFOCFG_EMPTYRX_MASK;
        NFCREADER_SPI->FIFOSTAT |= SPI_FIFOSTAT_TXERR_MASK | SPI_FIFOSTAT_RXERR_MASK;
        NFCREADER_SPI->FIFOWR    = 0x07300000|g_rc663TxBuffer[i];
        /* wait if TX FIFO of previous transfer is not empty */
        while ((NFCREADER_SPI->FIFOSTAT & SPI_FIFOSTAT_RXNOTEMPTY_MASK) == 0) {
            ;
        }
        buff[i] = (NFCREADER_SPI->FIFORD)&0x000000FF; 
#endif
	}
    GPIO_PinWrite(GPIO, NFCSEL_PORT, NFCSEL_PIN, 1);
    
    for(i=0; i<(num+1); i++)
    {
        g_rc663SPIDataTemp = buff[i+1];
        buff[i]             = g_rc663SPIDataTemp; 
    }

#if NFCREADER_DEBUG
    PRINTF("-----------------------------------\r\n"); 
    PRINTF("RC663_RegSeqRead\r\n");
    PRINTF("Tx Buf: "); 
    RC663_DataPrint((uint8_t *)g_rc663TxBuffer, num+2); 
    PRINTF("Rx Buf: ");
    RC663_DataPrint(buff, num+2);
#endif
}

/**
 * @brief   
 * @param   
 * @return  
 */
uint8_t RC663_RegRead(uint8_t regAddr)
{
    uint32_t i;
    GPIO_PinWrite(GPIO, NFCSEL_PORT, NFCSEL_PIN, 0);
    memset((void *)g_rc663TxBuffer, 0x00, 8);
    g_rc663TxBuffer[0] = (regAddr << 1) | 0x01;

    GPIO_PinWrite(GPIO, NFCSEL_PORT, NFCSEL_PIN, 0);
	for(i = 0; i < 2; i++)
	{  
#if RC663_SPI_SIM
        g_rc663RxBuffer[i] = RC663_SPIGPIOSend(g_rc663TxBuffer[i]);
#else
        /* clear tx/rx errors and empty FIFOs */
        NFCREADER_SPI->FIFOCFG  |= SPI_FIFOCFG_EMPTYTX_MASK | SPI_FIFOCFG_EMPTYRX_MASK;
        NFCREADER_SPI->FIFOSTAT |= SPI_FIFOSTAT_TXERR_MASK | SPI_FIFOSTAT_RXERR_MASK;
        NFCREADER_SPI->FIFOWR    = 0x07300000|g_rc663TxBuffer[i];
        /* wait if TX FIFO of previous transfer is not empty */
        while ((NFCREADER_SPI->FIFOSTAT & SPI_FIFOSTAT_RXNOTEMPTY_MASK) == 0)
        {
            ;
        }
        g_rc663RxBuffer[i] = (NFCREADER_SPI->FIFORD)&0x000000FF; 
#endif
	}
    GPIO_PinWrite(GPIO, NFCSEL_PORT, NFCSEL_PIN, 1);
    
#if NFCREADER_DEBUG
    PRINTF("-----------------------------------\r\n"); 
    PRINTF("RC663_RegRead\r\n");
    PRINTF("Tx Buf: "); 
    RC663_DataPrint((uint8_t *)g_rc663TxBuffer, 2);    
    PRINTF("Rx Buf: ");
    RC663_DataPrint((uint8_t *)g_rc663RxBuffer, 2);
#else
    RC663_Delayus(1);
#endif
    return g_rc663RxBuffer[1];
}

/**
 * @brief   
 * @param   
 * @return  
 */
void RC663_PinInit(void) 
{
    volatile uint8_t  g_rc663Sak         = 0;
    volatile uint8_t  g_rc663Status      = 0;
    
	// 11U68 GPIO init
    gpio_pin_config_t   gpioPinConfig;
    gpioPinConfig.pinDirection = kGPIO_DigitalOutput;
    gpioPinConfig.outputLogic  = 1u; /* output high as default. */
    
    /* Enables the clock for the I/O controller.: Enable Clock. */
    CLOCK_EnableClock(kCLOCK_Iocon);
    /* Debug Pin Port Init */
	IOCON->PIO[NFCSEL_PORT][NFCSEL_PIN] = (NFCSEL_FUNC | IOCON_MODE_INACT  | IOCON_DIGITAL_EN );

#if RC663_SPI_SHARED

#else
	IOCON->PIO[NFCMO_PORT][NFCMO_PIN]   = (NFCMO_FUNC  | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN );
    IOCON->PIO[NFCMI_PORT][NFCMI_PIN]   = (NFCMI_FUNC  | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN );
    IOCON->PIO[NFCSCK_PORT][NFCSCK_PIN] = (NFCSCK_FUNC | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN );
#endif
    IOCON->PIO[NFCRST_PORT][NFCRST_PIN] = (NFCRST_FUNC | IOCON_MODE_PULLUP | IOCON_DIGITAL_EN );
    IOCON->PIO[NFCIRQ_PORT][NFCIRQ_PIN] = (NFCIRQ_FUNC | IOCON_MODE_INACT  | IOCON_DIGITAL_EN );
    /* Disables the clock for the I/O controller.: Disable Clock. To Save Power */
    CLOCK_DisableClock(kCLOCK_Iocon);

    GPIO_PinInit (GPIO, NFCSEL_PORT, NFCSEL_PIN, &gpioPinConfig);
    GPIO_PinWrite(GPIO, NFCSEL_PORT, NFCSEL_PIN, 1u);
    
// mfrc reset pin
    GPIO_PinInit (GPIO, NFCRST_PORT, NFCRST_PIN, &gpioPinConfig);
    GPIO_PinWrite(GPIO, NFCRST_PORT, NFCRST_PIN,  1u);

#if RC663_SPI_SIM
    GPIO_PinInit (GPIO, NFCMO_PORT,  NFCMO_PIN,  &gpioPinConfig);
    GPIO_PinWrite(GPIO, NFCMO_PORT,  NFCMO_PIN,  1u);
    
    GPIO_PinInit (GPIO, NFCSCK_PORT, NFCSCK_PIN, &gpioPinConfig);
    GPIO_PinWrite(GPIO, NFCSCK_PORT, NFCSCK_PIN, 1u);
    
    gpioPinConfig.pinDirection = kGPIO_DigitalInput;
    gpioPinConfig.outputLogic  = 1u; /* output high as default. */
    GPIO_PinInit (GPIO, NFCMI_PORT,  NFCMI_PIN,  &gpioPinConfig);
#else

#if RC663_SPI_SHARED
    
#else
    spi_master_config_t userConfig = {0};
    /* attach 12 MHz clock to NFC Reader's SPI */
    CLOCK_AttachClk(NFCREADER_SPI_CLKATTACH);
    /* reset FLEXCOMM for SPI */
    RESET_PeripheralReset(NFCREADER_SPI_RST);
    
    /*
     * userConfig.enableLoopback = false;
     * userConfig.enableMaster = true;
     * userConfig.polarity = kSPI_ClockPolarityActiveHigh;
     * userConfig.phase = kSPI_ClockPhaseFirstEdge;
     * userConfig.direction = kSPI_MsbFirst;
     * userConfig.baudRate_Bps = 500000U;
     */
    SPI_MasterGetDefaultConfig(&userConfig);
    userConfig.baudRate_Bps = NFCREADER_SPI_CLKFREQ;
    userConfig.sselNum = (spi_ssel_t)0;
    userConfig.sselPol = (spi_spol_t)kSPI_SpolActiveAllLow;
    SPI_MasterInit(NFCREADER_SPI, &userConfig, NFCREADER_SPI_CLKSOURCE);
#endif // endif RC663_SPI_SHARED
    
#endif // endif RC663_SPI_SIM

    PcdReset();
    
    g_rc663Sak    = RC663_RegRead(0x7F);
    g_rc663Status = PICC_PcdTypeA106Config();
    PRINTF("*** RCsak %x  RCstatus %x\r\n", g_rc663Sak, g_rc663Status);
}
