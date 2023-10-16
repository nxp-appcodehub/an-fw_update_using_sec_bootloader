/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sbl_api.h"
#include "memory.h"
#include "sbl_config.h"
#include "fsl_common.h"


#define MAX_RETRY_CNT   (3)

/* 0x00: no re-invoke called, 0x01: re-invoke called */

#ifdef __ICCARM__
uint32_t re_invoke_flag @ 0x2000CF00 = 0x00;
#else
uint32_t re_invoke_flag;
#endif

void reinvoke(void)
{
    re_invoke_flag = 0x01;  
    
    uint32_t addr = 0x00000000;
    
    uint32_t *vectorTable = (uint32_t*)addr;
    uint32_t sp = vectorTable[0];
    uint32_t s_stackPointer = 0;

    s_stackPointer = sp;

    // Change MSP and PSP
    __set_MSP(s_stackPointer);
    __set_PSP(s_stackPointer);
    
    SCB->VTOR = addr;
    
    // Jump to application
    int main(void);
    main();

    // Should never reach here.
    __NOP();
}

int sbl_nvm_write(sbl_nvm_t* ctx)
{
    memory_erase(BL_DATA_START, BL_DATA_SIZE);
    memory_write(BL_DATA_START, (void*)ctx, sizeof(sbl_nvm_t));
    return 0;
}

int sbl_nvm_init(sbl_nvm_t* ctx)
{
    memory_read(BL_SIZE, (void*)ctx, sizeof(sbl_nvm_t));
    if(ctx->marker != BL_DATA_MARKER)
    {
        //printf("bad param, re-init nvm\r\n");
        
        /* init the data */
        ctx->marker = BL_DATA_MARKER;
        ctx->update_flag = 0;
        ctx->update_retry_cnt = 0;
        sbl_nvm_write(ctx);
    }
    return 0;
}


void set_update_flag(void)
{
    sbl_nvm_t sbl_nvm;
    sbl_nvm.marker = BL_DATA_MARKER;
    sbl_nvm.update_flag = 1;
    sbl_nvm.update_retry_cnt = MAX_RETRY_CNT;
    sbl_nvm_write(&sbl_nvm);
}

void test(void)
{
    //printf("test\r\n");
}

#ifdef __ICCARM__
const sbl_api_t sbl_api @ 0x400 = 
#else
const sbl_api_t sbl_api __attribute__((section(".ARM.__at_0x400"))) = 
#endif
{
    reinvoke,
    set_update_flag,
    test,
};













