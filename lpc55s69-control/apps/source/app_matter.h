/*
 * app_matter.h
 *
 *  Created on: Apr 14, 2022
 *      Author: nxf65009
 */

#ifndef APP_MATTER_H_
#define APP_MATTER_H_

#include "stdint.h"

#define MATTERTASKIDLE           0x00000000
#define MATTERLOCK				 0x00000001
#define MATTERUNLOCK			 0x00000002

#define MATTER_BUFFER_SIZE       4096

#define MATTER_UART                   USART4
#define MATTER_UART_RST               kFC4_RST_SHIFT_RSTn
#define MATTER_UART_CLKATTACH         kMAIN_CLK_to_FLEXCOMM4 // kFRO12M_to_FLEXCOMM4 // kMAIN_CLK_to_FLEXCOMM4
#define MATTER_UART_CLKFREQ           CLOCK_GetFlexCommClkFreq(4U)
#define MATTER_UART_CLKSRC            kCLOCK_Flexcomm4
#define MATTER_UART_IRQNUM            FLEXCOMM4_IRQn
#define MATTER_UART_IRQHANDLER        FLEXCOMM4_IRQHandler

#define MATTERTXD_PORT                1u
#define MATTERTXD_PIN                 20u
#define MATTERTXD_FUNC                IOCON_FUNC5

#define MATTERRXD_PORT                1u
#define MATTERRXD_PIN                 21u
#define MATTERRXD_FUNC                IOCON_FUNC5

#define MATTERIRQI_PORT               1u
#define MATTERIRQI_PIN                30u
#define MATTERIRQI_FUNC               IOCON_FUNC0
#define MATTERPINTI_SRC               kINPUTMUX_GpioPort1Pin30ToPintsel

#define MATTERIRQO_PORT               1u
#define MATTERIRQO_PIN                19u
#define MATTERIRQO_FUNC               IOCON_FUNC0

extern volatile uint32_t  g_MATTERTaskStatus;

extern void     APP_MATTER_Init(void);
extern uint32_t APP_MATTER_Task(void);
extern uint32_t APP_MATTER_Printf(const char *formatString, ...);

#endif /* APP_MATTER_H_ */
