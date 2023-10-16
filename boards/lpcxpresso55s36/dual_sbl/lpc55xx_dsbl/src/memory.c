/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "memory.h"
#include "fsl_flash.h"
#include "fsl_flash_ffr.h"

#include <string.h>
#include "fsl_debug_console.h"

#define ALIGN_DOWN(x, a)    ((x) & -(a))
#define ALIGN_UP(x, a)      (-(-(x) & -(a)))

#ifndef ALIGN
/* Compiler Related Definitions */
#ifdef __arm__                         /* ARM Compiler */
    #define ALIGN(n)                    __attribute__((aligned(n)))
#elif defined (__IAR_SYSTEMS_ICC__)     /* for IAR Compiler */
    #define PRAGMA(x)                   _Pragma(#x)
    #define ALIGN(n)                    PRAGMA(data_alignment=n)
#elif defined (__GNUC__)                /* GNU GCC Compiler */
    #define ALIGN(n)                    __attribute__((aligned(n)))
#endif /* Compiler Related Definitions */
#endif


/* NB4: sector size: 32K, page size:512 */
#define CH_OK           (0)
#define CH_ERR          (1)
#define SECTOR_SIZE     (32*1024)

#define LIB_DEBUG

#if defined(LIB_DEBUG)
#include <stdio.h>
#define LIB_TRACE	printf
#else
#define LIB_TRACE(...)
#endif

/*
    FLASH ISP does not export FLASH_Read function, here we have to implment FLASH_Read 
*/

/* memory instance */
static flash_config_t flashInstance;


int memory_init(void)
{
    flashInstance.modeConfig.sysFreqInMHz = CLOCK_GetFreq(kCLOCK_CoreSysClk) / (1000*1000);
    if (FLASH_Init(&flashInstance) == kStatus_Success)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int memory_erase(uint32_t addr, uint32_t len)
{
    status_t ret;
    
    ret = FLASH_Erase(&flashInstance, addr, len, kFLASH_ApiEraseKey);
    return ret;
}

int memory_write(uint32_t start_addr, uint8_t *buf, uint32_t len)
{
    status_t ret;
    ALIGN(512) static uint8_t  tmp_buf[512];
    
    memcpy(tmp_buf, buf, len);
    
    /* for LPC55xx, safe protect, do not erase last sector */
    if((start_addr + len) > 512*1024)
    {
      return 1;
    }
    ret = FLASH_Program(&flashInstance, start_addr, tmp_buf, 512);
    return ret;
}


int memory_read(uint32_t addr, uint8_t *buf, uint32_t len)
{
    return FLASH_Read(&flashInstance, addr, buf, len);
}

int memory_copy(uint32_t to, uint32_t from, uint32_t len)
{
    int i;
    int ret;
    
    for(i=0; i<len / 512; i++)
    {
        ret = 0;
        ret |= memory_erase(to+512*i, 512);
        ret |= memory_write(to+512*i, (void*)(from+512*i), 512);
    }
    
    if(len % 512)
    {
        ret = 0;
        ret |= memory_erase(to+512*i, 512);
        ret |= memory_write(to+512*i, (void*)(from+512*i), len % 512);
    }
    
    return ret;
}

uint32_t FLASH_SetcorTest(uint32_t addr)
{
    uint32_t ret, i, j;
    uint8_t *p;
    ALIGN(512) static  uint8_t buf[512];
    
    LIB_TRACE("program addr:0x%X(%dKB) ...", addr, addr/1024);
    ret = memory_erase(addr, SECTOR_SIZE);
    
    for(i=0; i<sizeof(buf); i++)
    {
        buf[i] = i % 0xFF;
    }
    
    for(i=0; i<(SECTOR_SIZE/sizeof(buf)); i++)
    {
        ret += memory_write(addr + sizeof(buf)*i, buf, sizeof(buf));  
        if(ret)
        {
            LIB_TRACE("issue command failed\r\n");
            return CH_ERR;
        }
    }
    
    LIB_TRACE("varify addr:0x%X ...", addr);
    for(i=0; i<(SECTOR_SIZE/sizeof(buf)); i++)
    {
        memory_read(addr + sizeof(buf)*i, buf, sizeof(buf));
        p = buf;
        for(j=0; j<sizeof(buf); j++)
        {
            if(p[j] != (j%0xFF))
            {
                ret++;
                LIB_TRACE("ERR:[%d]:0x%02X ", i, *p); 
            }
        }
    }
    
    if(ret == 0)
    {
        LIB_TRACE("OK\r\n"); 
    }
    return ret;
}

uint32_t FLASH_Test(uint32_t addr, uint32_t len)
{
    int i, ret;

    for(i=0; i<(len/SECTOR_SIZE); i++)
    {
        ret = FLASH_SetcorTest(addr + i*SECTOR_SIZE);
        if(ret != CH_OK)
        {
            return ret;
        }
    }
    return ret;
}


