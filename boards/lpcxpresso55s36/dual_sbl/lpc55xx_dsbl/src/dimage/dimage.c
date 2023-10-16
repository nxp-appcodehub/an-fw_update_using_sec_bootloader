/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "dimage.h"
#include "crc32.h"
#include <string.h>
#include "memory.h"

/* image layout follow LPC54xxx Dual Enhenced Image */

#define DUAL_IMAGE_MAKRER               (0x0FFEB6B6)
#define HEADER_BLOCK_MARKER             (0xFEEDA5A5)
#define DUAL_IMAGE_MARKER_OFFSET        (0x24)
#define DUAL_IMAGE_HDR_ADDR             (DUAL_IMAGE_MARKER_OFFSET + 4)


void dump_hdr(ihdr_t *hdr)
{
    DIMAGE_TRACE("%-16s :0x%08X\r\n", "sheader_marker", hdr->header_marker);
    DIMAGE_TRACE("%-16s :0x%08X\r\n", "image_type", hdr->img_type);
    DIMAGE_TRACE("%-16s :0x%08X\r\n", "reserved", hdr->reserved);
    DIMAGE_TRACE("%-16s :%d\r\n",     "img_len", hdr->img_len);
    DIMAGE_TRACE("%-16s :0x%08X\r\n", "crc_value", hdr->crc_value);
    DIMAGE_TRACE("%-16s :0x%08X\r\n", "version", hdr->version);
}

/* get a image header from actual addr and load addr */
int image_get_hdr(uint32_t addr, uint32_t load_addr, ihdr_t *hdr)
{
    uint32_t hdr_addr;
    
    memory_read(addr + DUAL_IMAGE_HDR_ADDR, (uint8_t*)&hdr_addr, sizeof(hdr_addr));
    
    hdr_addr = hdr_addr - load_addr + addr;
    //DIMAGE_TRACE("dhr_addr:0x%X\r\n", hdr_addr);
    memory_read(hdr_addr, (uint8_t*)hdr, sizeof(ihdr_t));

    return hdr_addr;
}


/* do image crc checking */
static int _crc_check(uint32_t addr, uint32_t load_addr)
{
    int i;
    uint8_t buf[64];
    int ret;
    int crc_len, crc_start;
    uint32_t cal_crc, crc_offset;
    ihdr_t hdr;
    
    ret = 1;
    
    /* get image header */
    crc_offset = image_get_hdr(addr, load_addr, &hdr);
    
    if(hdr.header_marker == HEADER_BLOCK_MARKER)
    {
        switch(hdr.img_type)
        {
            case 0: /* need crc check */
                crc_offset = crc_offset -addr + sizeof(ihdr_t) - 2*sizeof(uint32_t);
                //DIMAGE_TRACE("crc_offset:0x%X\r\n", crc_offset);
                
                if(crc_offset > hdr.img_len)
                {
                    break;
                }
                
                crc32_init(&cal_crc);
                
                /* calcuate data before crc */
                crc_len = crc_offset;
                crc_start = addr;
                
                for(i=0; i<crc_len / sizeof(buf); i++)
                {
                    memory_read(crc_start + i*sizeof(buf), buf, sizeof(buf));
                    crc32_generate(&cal_crc, buf, sizeof(buf));
                }
                
                memory_read(crc_start + i*sizeof(buf), buf, crc_len % sizeof(buf));
                crc32_generate(&cal_crc, buf, crc_len % sizeof(buf));
                    
                /* calcuate data after crc */
                crc_len = hdr.img_len - crc_offset;
                crc_start = crc_offset + 4 + addr;
                  
                for(i=0; i<crc_len / sizeof(buf); i++)
                {
                    memory_read(crc_start + i*sizeof(buf), buf, sizeof(buf));
                    crc32_generate(&cal_crc, buf, sizeof(buf));
                }
                
                memory_read(crc_start + i*sizeof(buf), buf, crc_len % sizeof(buf));
                crc32_generate(&cal_crc, buf, crc_len % sizeof(buf));
                
                crc32_complete(&cal_crc);
                
                if(cal_crc == hdr.crc_value)
                {
                    ret = 0;
                }
                break;
            case 1: /* no crc check */
                ret = 0;
                break;
            default:
                break;
        }
    }
    return ret;
}

/* scan flash to find vailid image */
int image_scan(uint32_t start_addr, uint32_t load_addr, uint32_t len, uint32_t *image_addr, uint32_t max_image_cnt)
{
    uint32_t marker;
    int i;
    uint32_t image_cnt, addr;
    uint32_t hdr_addr;
    ihdr_t hdr;
    
    /* scan dual image marker and header */
    image_cnt = 0;
    
    for(i=0; i<len/sizeof(marker); i++)
    {
        memory_read(start_addr + i*sizeof(marker), (uint8_t*)&marker, sizeof(marker));

        if(marker == DUAL_IMAGE_MAKRER)
        {
            memory_read(start_addr + i*sizeof(marker) + 4, (uint8_t*)&hdr_addr, sizeof(hdr_addr));
            hdr_addr = hdr_addr - load_addr + start_addr;
            memory_read(hdr_addr, (uint8_t*)&hdr, sizeof(ihdr_t));
            if(hdr.header_marker == HEADER_BLOCK_MARKER)
            {
                addr = start_addr + i*sizeof(marker) - DUAL_IMAGE_MARKER_OFFSET;
                if(_crc_check(addr, load_addr) == 0)
                {
                    /* image found */
                    image_addr[image_cnt] = addr;
                    image_cnt++;
                    
                    if(max_image_cnt == image_cnt)
                    {
                        return image_cnt;
                    }
                }
                else
                {
                    DIMAGE_TRACE("crc check failed\r\n");
                }
            }
        }
    }
    return image_cnt;
}

