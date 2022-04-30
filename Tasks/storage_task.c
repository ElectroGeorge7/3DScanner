/*
 * storage_task.c
 *
 *  Created on: 12 дек. 2021 г.
 *      Author: George
 */

#include "storage_task.h"

#include "cmsis_os2.h"
#include "modules.h"

#include "camera.h"
#include "usb_device.h"

#include "stm32h7xx_hal.h"

#define Detect_SDIO_Pin GPIO_PIN_0
#define Detect_SDIO_GPIO_Port GPIOB

SD_HandleTypeDef hsd2;

__section (".ram_d3") static FATFS __aligned(32) sdFatFs;
__section (".ram_d3") static FIL __aligned(32) sdFile;

extern osMessageQueueId_t cameraQueueHandler;
extern osEventFlagsId_t cameraEvtId;

//BYTE readBuf[50] = {0};
//UINT br = 0;
FRESULT fr;
DIR dp;
void StorageTask(void *argument)
{
  //FRESULT fr;
  uint8_t str[] = "Hello!\n\r";
  osStatus_t res = 0;
  CameraQueueObj_t cameraMsg;
  uint32_t flags;

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
  storage_init();
  fr = f_mkdir("ap");
  fr = f_opendir(&dp, "/ap");

  for(;;)
  {
    //res = CDC_Transmit_FS(str, sizeof(str));
    flags = osEventFlagsWait(cameraEvtId, CAMERA_EVT_FILE_CREATE, osFlagsWaitAny, osWaitForever);
    switch(flags){
    case CAMERA_EVT_FILE_CREATE:
      res = osMessageQueueGet(cameraQueueHandler, &cameraMsg, 0, osWaitForever);
      MX_USB_DEVICE_Stop();
      SavePictureMB(cameraMsg.fileName, (sFrameBuf_t *)cameraMsg.frameBuf, (((sFrameBuf_t *)cameraMsg.frameBuf)->size) / 2);
      MX_USB_DEVICE_Start();
      break;
    case CAMERA_EVT_DIR_CREATE:
    	;
      break;
    default:
    	;
    }

    osDelay(1000);
  }

}

static void MX_DETECT_SDIO_GPIO_Init(void);

void storage_init(void){
	MX_DETECT_SDIO_GPIO_Init();
	MX_SDMMC2_SD_Init();
	MX_FATFS_Init();
	HAL_Delay(1000);
	fr = f_mount(&sdFatFs, "", 1);
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

#define WRITE_BUFE_SIZE 400//20
UINT bw = 0;
__section (".ram_d3") uint8_t __aligned(32)rgbBuf[WRITE_BUFE_SIZE*3] = {0};
void SavePicture(char *pictureName, uint16_t *buf, uint32_t num){
  FIL writeFile;
  FRESULT fr;
  uint16_t *pBuf = rgbBuf;
  uint32_t counter = 0;
  uint8_t rVal5, gVal6, bVal5;
  uint8_t desc[] = 
  { 0x50, 0x36,         // P6
    0x0a, 
    0x33, 0x32, 0x30,   // 320
    0x20, 
    0x32, 0x34, 0x30,   // 240
    0x0a, 
    0x32, 0x35, 0x35,   // 255
    0x0a
  };

  uint32_t temp = 0;

  do{
      fr = open_append(&writeFile, pictureName);
  }while (fr);

  // add file descriptor for .rgb format 
  fr = f_write(&writeFile, desc, sizeof(desc), &bw);
  fr = f_sync(&writeFile);

  do{

	for (uint32_t j = 0; j < WRITE_BUFE_SIZE*3; j+=3, counter++){
		  rVal5=(uint8_t)( (buf[counter] & 0b11111000) );
		  gVal6=(uint8_t)( (buf[counter] & 0b1110000000000000) >> 11 ); // младшие разряды
		  gVal6 |= (uint8_t)( (buf[counter] & 0b111) << 5 );            // старшие разряды
		  bVal5=(uint8_t)( (buf[counter] & 0b1111100000000) >> 5 );
		  rgbBuf[j] = (uint8_t) ( rVal5 );		// red
		  rgbBuf[j+1]= (uint8_t) ( gVal6 );	// green
		  rgbBuf[j+2]= (uint8_t) ( bVal5 );		// blue
	}
    fr = f_write(&writeFile, pBuf, WRITE_BUFE_SIZE*3, &bw);
    fr = f_sync(&writeFile);
    memset(rgbBuf, 0 , WRITE_BUFE_SIZE*3);
    //counter += WRITE_BUFE_SIZE;
  }while( counter < num);

  // Close the file
  f_close(&writeFile);

}

#include "../Modules/modules.h"
#include "camera.h"
void SavePictureMB(char *pictureName, sFrameBuf_t *frameBuf, uint32_t num){
  FIL writeFile;
  FRESULT fr;
  uint16_t *pBuf = rgbBuf;
  uint32_t counter = 0;
  uint8_t rVal5, gVal6, bVal5;
  uint8_t desc[] = 
  { 0x50, 0x36,         // P6
    0x0a, 
    0x36, 0x34, 0x30,   // 640
    0x20, 
    0x34, 0x38, 0x30,   // 480
    0x0a, 
    0x32, 0x35, 0x35,   // 255
    0x0a
  };

  uint16_t *temp;

  do{
      fr = open_append(&writeFile, pictureName);
  }while (fr);

  // add file descriptor for .rgb format 
  fr = f_write(&writeFile, desc, sizeof(desc), &bw);
  fr = f_sync(&writeFile);

  temp = (uint16_t *)(frameBuf->pFrameBuf1);
  do{
	for (uint32_t j = 0; j < WRITE_BUFE_SIZE*3; j+=3, counter++){
		  //rVal5=(uint8_t)( (temp[counter] & 0b11111000) );
		  //gVal6=(uint8_t)( (temp[counter] & 0b1110000000000000) >> 11 ); // младшие разряды
		  //gVal6 |= (uint8_t)( (temp[counter] & 0b111) << 5 );            // старшие разряды
		  //bVal5=(uint8_t)( (temp[counter] & 0b1111100000000) >> 5 );
		  rgbBuf[j] = (uint8_t) ( (temp[counter] & 0b11111000) );	// red
		  rgbBuf[j+1] = (uint8_t) (  ( (temp[counter] & 0b1110000000000000) >> 11 ) | ( (temp[counter] & 0b111) << 5 ) );	// green
		  rgbBuf[j+2] = (uint8_t) ( (temp[counter] & 0b1111100000000) >> 5 );	// blue
	}
    fr = f_write(&writeFile, pBuf, WRITE_BUFE_SIZE*3, &bw);
    fr = f_sync(&writeFile);
    //memset(rgbBuf, 0 , WRITE_BUFE_SIZE*3);
  }while( counter < num);

  temp = (uint16_t *)(frameBuf->pFrameBuf2);
  counter = 0;

  do{
	for (uint32_t j = 0; j < WRITE_BUFE_SIZE*3; j+=3, counter++){
		  //rVal5=(uint8_t)( (temp[counter] & 0b11111000) );
		  //gVal6=(uint8_t)( (temp[counter] & 0b1110000000000000) >> 11 ); // младшие разряды
		  //gVal6 |= (uint8_t)( (temp[counter] & 0b111) << 5 );            // старшие разряды
		  //bVal5=(uint8_t)( (temp[counter] & 0b1111100000000) >> 5 );
		  rgbBuf[j] = (uint8_t) ( (temp[counter] & 0b11111000) );	// red
		  rgbBuf[j+1] = (uint8_t) (  ( (temp[counter] & 0b1110000000000000) >> 11 ) | ( (temp[counter] & 0b111) << 5 ) );	// green
		  rgbBuf[j+2] = (uint8_t) ( (temp[counter] & 0b1111100000000) >> 5 );	// blue
	}
    fr = f_write(&writeFile, pBuf, WRITE_BUFE_SIZE*3, &bw);
    fr = f_sync(&writeFile);
    //memset(rgbBuf, 0 , WRITE_BUFE_SIZE*3);
  }while( counter < num);

  temp = (uint16_t *)(frameBuf->pFrameBuf3);
  counter = 0;

  do{
	for (uint32_t j = 0; j < WRITE_BUFE_SIZE*3; j+=3, counter++){
		  //rVal5=(uint8_t)( (temp[counter] & 0b11111000) );
		  //gVal6=(uint8_t)( (temp[counter] & 0b1110000000000000) >> 11 ); // младшие разряды
		  //gVal6 |= (uint8_t)( (temp[counter] & 0b111) << 5 );            // старшие разряды
		  //bVal5=(uint8_t)( (temp[counter] & 0b1111100000000) >> 5 );
		  rgbBuf[j] = (uint8_t) ( (temp[counter] & 0b11111000) );	// red
		  rgbBuf[j+1]= (uint8_t) (  ( (temp[counter] & 0b1110000000000000) >> 11 ) | ( (temp[counter] & 0b111) << 5 ) );	// green
		  rgbBuf[j+2]= (uint8_t) ( (temp[counter] & 0b1111100000000) >> 5 );	// blue
	}
    fr = f_write(&writeFile, pBuf, WRITE_BUFE_SIZE*3, &bw);
    fr = f_sync(&writeFile);
    //memset(rgbBuf, 0 , WRITE_BUFE_SIZE*3);
  }while( counter < num);


  temp = (uint16_t *)(frameBuf->pFrameBuf4);
  counter = 0;

  do{
	for (uint32_t j = 0; j < WRITE_BUFE_SIZE*3; j+=3, counter++){
		  //rVal5=(uint8_t)( (temp[counter] & 0b11111000) );
		  //gVal6=(uint8_t)( (temp[counter] & 0b1110000000000000) >> 11 ); // младшие разряды
		  //gVal6 |= (uint8_t)( (temp[counter] & 0b111) << 5 );            // старшие разряды
		  //bVal5=(uint8_t)( (temp[counter] & 0b1111100000000) >> 5 );
		  rgbBuf[j] = (uint8_t) ( (temp[counter] & 0b11111000) );	// red
		  rgbBuf[j+1]= (uint8_t) (  ( (temp[counter] & 0b1110000000000000) >> 11 ) | ( (temp[counter] & 0b111) << 5 ) );	// green
		  rgbBuf[j+2]= (uint8_t) ( (temp[counter] & 0b1111100000000) >> 5 );	// blue
	}
    fr = f_write(&writeFile, pBuf, WRITE_BUFE_SIZE*3, &bw);
    fr = f_sync(&writeFile);
    //memset(rgbBuf, 0 , WRITE_BUFE_SIZE*3);
  }while( counter < num);

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


static void MX_DETECT_SDIO_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pins : Detect_SDIO_Pin PB2 */
  GPIO_InitStruct.Pin = Detect_SDIO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Detect_SDIO_GPIO_Port, &GPIO_InitStruct);
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
