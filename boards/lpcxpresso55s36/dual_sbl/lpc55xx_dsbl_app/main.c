/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "board.h"
#include "fsl_flash.h"
#include "fsl_flash_ffr.h"
#include "fsl_common.h"
#include "pin_mux.h"

#include <stdio.h>

typedef struct
{
    void (*reinvoke)(void);
    void (*set_update_flag)(void);
    void (*test)(void);
}sbl_api_t;

static sbl_api_t *sbl_api = (sbl_api_t*)(0x400);

int main()
{
    /* Init board hardware. */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);
    
    BOARD_InitPins();
    BOARD_BootClockFROHF96M();
    BOARD_InitDebugConsole();
    
    printf("i am golden image\r\n");
        
    uint32_t version = *(uint32_t*)(0x10000 + 0x140 + 5*sizeof(uint32_t));
    
    printf("hello, gloden image running: Version:%d\r\n", version);
    
   // sbl_api->set_update_flag();
    sbl_api->reinvoke();
    
    while (1)
    {

    }
}

void HardFault_Handler(void)
{
    printf("HardFault_Handler from app\r\n");
    while(1);
}

