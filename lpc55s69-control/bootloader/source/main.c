/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "app.h"
#include "app_printf.h"
#include "app_binupdate.h"
#include "app_spiflash.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * @brief	doCleanBoot
 * @param   Nothing
 * @return	Nothing
 */
void secondaryLoaderAppEntry(void)
{
	__ASM volatile ("cpsid i" : : : "memory");		/* __disable_irq */
	__ASM volatile ("LDR r0, =0x0");
	__ASM volatile ("LDR r0, [r0, #0]");			/* Reset stack pointer */
	__ASM volatile ("MOV sp, r0");

	__ASM volatile ("LDR r0, =0x20000000");			/* Must match SL_ADDRESS_APPCALLEDFL value only for LPC55S69 */

	__ASM volatile ("LDR r1, =0x1");
	__ASM volatile ("STRB r1, [r0]");				/* FLASH boot sets pushAppFlag to 1 */

    BOARD_BootClockFRO12M();						/* Basic System Initial */
	main();											/* Jump to main app */
}

/**
 * @brief	doCleanBoot
 * @param   Nothing
 * @return	Nothing
 */
typedef void (*iapfun)(void);
iapfun jump2app;
volatile uint32_t g_ImageAddr = 0x8000;
void clean_boot(void)
{
	/* Cleanup before going to app */
	/* Switch Main clock resource to FRO 12MHz */
	//BOARD_BootClockFRO12M();

	/* Boot Valid Application */
	/* Set Stack */
	__set_MSP(*(volatile uint32_t*)(g_ImageAddr));
	/* Set app entry point */
	jump2app =(iapfun)*(uint32_t*)(g_ImageAddr+4);

	/* enable interrupt */
	__ASM volatile ("cpsie i" : : : "memory");
    //__ASM volatile ("cpsid i" : : : "memory");		/* __disable_irq */
	/* Jump_To_Application = (pFunction) JumpAddress; */
	jump2app();
	/***** Control should never come here *****/
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Enables the clock for the I/O controller.: Enable Clock. */
    CLOCK_EnableClock(kCLOCK_Iocon);
    CLOCK_EnableClock(kCLOCK_InputMux);
    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_EnableClock(kCLOCK_Gpio1);

    BOARD_BootClockFRO12M();

    PRINT_UARTInit(115200);
    PRINTF("\r\n~~~LPC5500 ELOCK 2.0~~~\r\n");
    PRINTF("*** This is boot project\r\n");
    PRINTF("*** Boot version %d.%d\r\n", BOOT_VERSION_MAIN, BOOT_VERSION_MINI);
	PRINTF("*** Build Date %s %s\r\n", __DATE__, __TIME__);

    /* Enables the clock for the I/O controller.: Enable Clock. */
    CLOCK_EnableClock(kCLOCK_Iocon);
    /* Audio Binary Update Detected Pin */
    IOCON_PinMuxSet(IOCON, BOOT_ENTRY_PORT, BOOT_ENTRY_PIN, IOCON_PIO_FUNC(0) | IOCON_PIO_DIGIMODE_MASK);
    /* Disables the clock for the I/O controller.: Disable Clock. To Save Power */
    CLOCK_DisableClock(kCLOCK_Iocon);
    
    /* Initialize Detect Pin */
    gpio_pin_config_t   gpioPinConfig;
    gpioPinConfig.pinDirection = kGPIO_DigitalInput;
    gpioPinConfig.outputLogic  = 1u; /* input high as default. */

    GPIO_PinInit (GPIO, BOOT_ENTRY_PORT, BOOT_ENTRY_PIN,  &gpioPinConfig);
    
    if(GPIO_PinRead(GPIO, BOOT_ENTRY_PORT,  BOOT_ENTRY_PIN) == 0)
    {
        spiflash_init();
        //spiflash_test();
        
        binupdate_init();
        while(1)
        {
            binupdate_task();
        }
    }

//    extern volatile uint8_t *g_BinaryImage;
//    g_BinaryImage = 0x04000000;
//    spiflash_read_file(0, g_BinaryImage, 0x4000); /* Save binary into spi flash */
//    PRINTF("%x %x %x %x\r\n", g_BinaryImage[0], g_BinaryImage[1], g_BinaryImage[2], g_BinaryImage[3]);
    /* Need check Image is valid */
    /* Clean-up System */
    /* Goto Core0 Application Code */
    g_ImageAddr = 0x8000; /* Config Image Start Address */
    //PRINTF("*** Boot to Address 0x%x\r\n", g_ImageAddr);
    
    clean_boot(); /* Core0 Start @ 0x18000 */
    
    while (1)
    {

    }
}
