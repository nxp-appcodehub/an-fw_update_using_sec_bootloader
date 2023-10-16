/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef SBL_CONFIG_H
#define SBL_CONFIG_H

/* bootloader has two regions: data region and paramapter region */
#define BL_START            (0)
#define BL_SIZE             (60*1024)
#define BL_DATA_START       (BL_START + BL_SIZE)
#define BL_DATA_SIZE        (4*1024)
#define BL_DATA_MARKER      (0x0FFEB6A7)

/* golden image region */
#define GOLDEN_REGION_START      (64*1024)
#define GOLDEN_REGION_LEN        (64*1024)

/* back up image region */
#define BACKUP_REGION_START     (128*1024)
#define BACKUP_REGION_LEN       (64*1024)



#endif
