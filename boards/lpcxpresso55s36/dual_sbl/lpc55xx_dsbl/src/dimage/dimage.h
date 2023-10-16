/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __DIMAGE_H__
#define __DIMAGE_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>

#define DIMAGE_DEBUG

#if defined(DIMAGE_DEBUG)
#include <stdio.h>
#define DIMAGE_TRACE	printf
#else
#define DIMAGE_TRACE(...)
#endif

/* generate image via: ./image_generator.exe -s ./MDK/lpc54608/se_image.bin ./se_image_crc.bin */

typedef struct
{
	uint32_t header_marker;						/*!< Image header marker should always be set to 0xFEEDA5A5 */
    uint32_t img_type;                          /*!< Image check type, with or without optional CRC */
    uint32_t reserved;                          /*!< Reserved word; must be 0 */
	uint32_t img_len;                           /*!< Image length or the length of image CRC check should be done. */
	uint32_t crc_value;                         /*!< CRC value  */
    uint32_t version;                           /*!< Image version for multi-image support */
}ihdr_t;



void dump_hdr(ihdr_t *hdr);
int image_scan(uint32_t start_addr, uint32_t load_addr, uint32_t len, uint32_t *image_addr, uint32_t max_image_cnt);
int image_get_hdr(uint32_t addr, uint32_t load_addr, ihdr_t *hdr);


#ifdef __cplusplus
}
#endif

#endif

