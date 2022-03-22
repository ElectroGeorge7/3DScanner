/*
 * storage_task.c
 *
 *  Created on: 12 дек. 2021 г.
 *      Author: George
 */

#include "storage_task.h"
#include "stm32h7xx_hal.h"

#include "usb_device.h"
#include "bsp_driver_sd.h"

SD_HandleTypeDef hsd2;
extern USBD_HandleTypeDef hUsbDeviceFS;

static FATFS sdFatFs;

//BYTE readBuf[50] = {0};
//UINT br = 0;

void StorageTask(void *argument)
{
  FIL sdFile;
  FRESULT fr;
  uint8_t str[] = "Hello!\n";
  uint8_t res = 0;

  MX_SDMMC2_SD_Init();
  MX_FATFS_Init();
  MX_USB_DEVICE_Init();


  // Open or create a log file and ready to append
  //fr = f_mkfs("", FM_ANY, 0, work, sizeof(work));
  //fr = f_mount(&sdFatFs, "", 1);
/*  do{
      //fr = open_append(&sdFile, "camconf.txt");
      fr = f_open(&sdFile, "camconf.txt", FA_READ);
  }while (fr);

  fr = f_read(&sdFile, readBuf, 40, &br);

  // Close the file
  f_close(&sdFile);
*/
  for(;;)
  {
    res = CDC_Transmit_FS(str, sizeof(str));
    osDelay(1000);
  }

}


void fRead(char *configFileName, uint8_t *buf, uint32_t num, uint32_t *br){
  FIL readFile;
  FRESULT fr;

  do{
      //fr = open_append(&sdFile, "camconf.txt");
      fr = f_open(&readFile, configFileName, FA_READ);
  }while (fr);

  fr = f_read(&readFile, buf, num, br);

  // Close the file
  f_close(&readFile);
}

#define WRITE_BUFE_SIZE 200
uint32_t bw = 0;
void SavePicture(char *pictureName, uint16_t *buf, uint32_t num){
  FIL writeFile;
  FRESULT fr;

  do{
      fr = open_append(&writeFile, pictureName);
  }while (fr);

  do{
    fr = f_write(&writeFile, buf, WRITE_BUFE_SIZE, &bw);
    fr = f_sync(&writeFile);
  }while( bw == WRITE_BUFE_SIZE);

  // Close the file
  f_close(&writeFile);

}

/**
  * @brief SDMMC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDMMC2_SD_Init(void)
{

  hsd2.Instance = SDMMC2;
  hsd2.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd2.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd2.Init.BusWide = SDMMC_BUS_WIDE_4B;
  hsd2.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
  hsd2.Init.ClockDiv = 4;
  hsd2.Init.TranceiverPresent = SDMMC_TRANSCEIVER_NOT_PRESENT;

}

// Только для записи в конец файла
FRESULT open_append (
    FIL* fp,            // [OUT] File object to create
    const char* path    // [IN]  File name to be opened
)
{
    FRESULT fr;

    // Opens an existing file. If not exist, creates a new file.
    fr = f_open(fp, path, FA_WRITE | FA_OPEN_ALWAYS);
    if (fr == FR_OK) {
        // Seek to end of the file to append data
        fr = f_lseek(fp, f_size(fp));
        if (fr != FR_OK)
            f_close(fp);
    }
    return fr;
}
