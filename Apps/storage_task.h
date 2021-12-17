/*
 * storage_task.h
 *
 *  Created on: 12 дек. 2021 г.
 *      Author: George
 */

#ifndef STORAGE_TASK_H_
#define STORAGE_TASK_H_

#include "fatfs.h"

void StorageTask(void *argument);

void fRead(char *configFileName, uint8_t *buf, uint32_t num, uint32_t *br);
void SavePicture(char *pictureName, uint16_t *buf, uint32_t num);

FRESULT open_append (
    FIL* fp,            // [OUT] File object to create
    const char* path    // [IN]  File name to be opened
);

static void MX_SDMMC2_SD_Init(void);

#endif /* STORAGE_TASK_H_ */
