/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "board.h"
#include "fsl_usart.h"
#include "fsl_flash.h"
#include "fsl_flash_ffr.h"
#include "fsl_common.h"
#include "pin_mux.h"

#include "memory.h"
#include "dimage.h"
#include "mcuboot.h"
#include "sbl_api.h"
#include "sbl_config.h"

/* mcuboot instance */
static mcuboot_t mcuboot;

int sbl_nvm_init(sbl_nvm_t* ctx);
int sbl_nvm_write(sbl_nvm_t* ctx);
extern bool re_invoke_flag;

static int mcuboot_send(uint8_t *buf, uint32_t len)
{
    USART_WriteBlocking(USART0, buf, len);
    return 0;
}

void JumpToImage(uint32_t addr)
{
    uint32_t *vectorTable = (uint32_t*)addr;
    uint32_t sp = vectorTable[0];
    uint32_t pc = vectorTable[1];
    
    typedef void(*app_entry_t)(void);

    uint32_t s_stackPointer = 0;
    uint32_t s_applicationEntry = 0;
    app_entry_t s_application = 0;

    s_stackPointer = sp;
    s_applicationEntry = pc;
    s_application = (app_entry_t)s_applicationEntry;

    // Change MSP and PSP
    __set_MSP(s_stackPointer);
    __set_PSP(s_stackPointer);
    
    SCB->VTOR = addr;
    
    // Jump to application
    s_application();

    // Should never reach here.
    __NOP();
}

static void mcuboot_reset(void)
{
    /* delay for a while to wait mcuboot send respond packet */
    NVIC_SystemReset();
}

static void mcuboot_jump(uint32_t addr, uint32_t arg, uint32_t sp)
{
    /* clean up */
    USART_DisableInterrupts(USART0, kUSART_RxLevelInterruptEnable | kUSART_RxErrorInterruptEnable);
    
    DIMAGE_TRACE("dsbl: boot @ 0x%08X\r\n", addr);
    
    JumpToImage(addr);
}

static void mcuboot_complete(void)
{
    sbl_nvm_t sbl_nvm;
    sbl_nvm.marker = BL_DATA_MARKER;
    sbl_nvm.update_flag = 0;
    sbl_nvm.update_retry_cnt = 0;
    sbl_nvm_write(&sbl_nvm);
}

/* do dual image policy and boot application if everything ok */
static int image_check_and_boot(void)
{
    /*
        gaddr: golden image addr
        baddr: backup image addr 
        gimage_cnt: how many golden image in golden area
        bimage_cnt: how many backup image in backup area
    */
  
    int gimage_cnt, bimage_cnt, len;
    uint32_t gaddr, baddr;
    ihdr_t ghdr, bhdr;
    
    /* scan a image in golden region */
    DIMAGE_TRACE("scan golden region...\r\n");
    gimage_cnt = image_scan(GOLDEN_REGION_START, GOLDEN_REGION_START, 512, &gaddr, 1);
    if(gimage_cnt)
    {
        DIMAGE_TRACE("image found: 0x%08X\r\n", gaddr);
        image_get_hdr(gaddr, GOLDEN_REGION_START, &ghdr);
        dump_hdr(&ghdr);
    }
    
    /* scan a image in backup region */
    DIMAGE_TRACE("scan backup region...\r\n");
    bimage_cnt = image_scan(BACKUP_REGION_START, GOLDEN_REGION_START, 512, &baddr, 1);
    if(bimage_cnt)
    {
        DIMAGE_TRACE("image found: 0x%08X\r\n", baddr);
        image_get_hdr(baddr, GOLDEN_REGION_START, &bhdr);
        dump_hdr(&bhdr);
    }

    if((!gimage_cnt) && (bimage_cnt))
    {
        DIMAGE_TRACE("golen image bad, backup ok\r\n");
        
        /* copy to golen image and boot */
        (bhdr.img_type == 0x00000001)?(len = BACKUP_REGION_LEN):(len = bhdr.img_len + 4);
        memory_copy(GOLDEN_REGION_START, baddr, len);
        mcuboot_jump(GOLDEN_REGION_START, 0, 0);
    }
    
    if(gimage_cnt && (!bimage_cnt))
    {
        DIMAGE_TRACE("golen image ok, no backup image, boot\r\n");
        
        mcuboot_jump(GOLDEN_REGION_START, 0, 0);
    }
    
    if(gimage_cnt && bimage_cnt)
    {
        if(ghdr.version < bhdr.version)
        {
            DIMAGE_TRACE("backup image version higer, copy to golden area\r\n");
            
            /* copy backup into golden */
            (bhdr.img_type == 0x00000001)?(len = BACKUP_REGION_LEN):(len = bhdr.img_len + 4);
            memory_copy(GOLDEN_REGION_START, baddr, len);
        }
        
        DIMAGE_TRACE("golden image has same or higher version then backup, boot golen image\r\n");
        
        mcuboot_jump(GOLDEN_REGION_START, 0, 0);
    }
    
    return 1;
}


int main(void)
{
    sbl_nvm_t sbl_nvm;
    
    /* Init board hardware. */
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* enable clock for GPIO*/
    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_EnableClock(kCLOCK_Gpio1);
    
    BOARD_InitPins();
    BOARD_BootClockFROHF96M();
    BOARD_InitDebugConsole();
    
    DIMAGE_TRACE("CoreClock:%dHz\r\n", CLOCK_GetFreq(kCLOCK_CoreSysClk));
    
    /* config ISP pin */
    gpio_pin_config_t led_config = { kGPIO_DigitalInput, 0, };
    GPIO_PortInit(GPIO, 0);
    GPIO_PinInit(GPIO, 0, 17, &led_config);
    
    memory_init();
    
    sbl_nvm_init(&sbl_nvm);

    /* if update_rey cnt > MAX time, clear update flag */
    if(sbl_nvm.update_flag)
    {
        sbl_nvm.update_retry_cnt--;
        if(!sbl_nvm.update_retry_cnt)
        {
            sbl_nvm.update_flag = 0;
        }
        sbl_nvm_write(&sbl_nvm);
    }
    
    if(GPIO_PinRead(GPIO, 0, 17) == 1 && (re_invoke_flag == 0x00) && sbl_nvm.update_flag != 1)
    {
        DIMAGE_TRACE("boot application...\r\n");
        /* do not enter bootloader */
        image_check_and_boot();
        
        /* if a successful boot, code will neven run here */
    }
    
    DIMAGE_TRACE("enter dual bootloader\r\n");

    USART_EnableInterrupts(USART0, kUSART_RxLevelInterruptEnable | kUSART_RxErrorInterruptEnable);
    EnableIRQ(FLEXCOMM0_IRQn);
    
    /* config and init the mcuboot */
    mcuboot.op_send = mcuboot_send;
    mcuboot.op_reset = mcuboot_reset;
    mcuboot.op_jump = mcuboot_jump;
    mcuboot.op_complete = mcuboot_complete;
    
    mcuboot.op_mem_erase = memory_erase;
    mcuboot.op_mem_write = memory_write;
    mcuboot.op_mem_read = memory_read;
    
    mcuboot.cfg_flash_start = BACKUP_REGION_START;
    mcuboot.cfg_flash_size = BACKUP_REGION_LEN;
    mcuboot.cfg_ram_start = 0x20000000;
    mcuboot.cfg_ram_size = 128*1024;
    mcuboot.cfg_device_id = 0x12345678;
    mcuboot.cfg_uuid = 0x87654321;
    
    mcuboot_init(&mcuboot);

    
    while(1)
    {
        mcuboot_proc(&mcuboot);
    }
}

void FLEXCOMM0_IRQHandler(void)
{
    uint8_t c;

    if ((kUSART_RxFifoNotEmptyFlag | kUSART_RxError) & USART_GetStatusFlags(USART0))
    {
        c = USART_ReadByte(USART0);

        /* feed recv data into mcuboot */
        mcuboot_recv(&mcuboot, &c, 1);
    }
}

void HardFault_Handler(void)
{
    printf("HardFault_Handler from sbl\r\n");
    while(1);
}


