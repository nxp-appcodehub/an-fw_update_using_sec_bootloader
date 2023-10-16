/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _SBL_API_H
#define _SBL_API_H

#include <stdint.h>


typedef struct
{
    uint32_t update_flag;
    uint32_t marker;
    uint32_t update_retry_cnt;  /* max retry count after app call set_update_flag */
}sbl_nvm_t;

typedef struct
{
    void (*reinvoke)(void);
    void (*set_update_flag)(void);
    void (*test)(void);
}sbl_api_t;


#endif
