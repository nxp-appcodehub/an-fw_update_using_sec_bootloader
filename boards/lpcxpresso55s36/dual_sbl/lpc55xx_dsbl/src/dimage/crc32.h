/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CRC32_H
#define CRC32_H

#include <stdint.h>

uint32_t crc32_compute(uint8_t *buf, int len);
void crc32_complete(uint32_t *CRC);
void crc32_generate(uint32_t *CRC, uint8_t *buf, int len);
void crc32_init (uint32_t *CRC);


#endif
