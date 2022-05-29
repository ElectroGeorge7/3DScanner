/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usb_device.c
  * @version        : v1.0_Cube
  * @brief          : This file implements the USB Device
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/

#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc_msc.h"

#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#include "usbd_msc.h"
#include "usbd_msc_bot.h"
#include "usbd_storage_if.h"

#include "terminal.h"


/* USB Device Core handle declaration. */
// should be in RAM_D1,
// otherwise USB stack don`t work after power switch
USBD_HandleTypeDef hUsbDeviceFS;


void MX_USB_DEVICE_Start(void)
{
	USBD_Start(&hUsbDeviceFS);
}

void MX_USB_DEVICE_Stop(void)
{
	USBD_Stop(&hUsbDeviceFS);
}

void MX_USB_DEVICE_Suspend(void)
{
	USBD_StatusTypeDef res;
	//USBD_LL_Suspend(&hUsbDeviceFS);
	//res = USBD_LL_StallEP(&hUsbDeviceFS, MSC_OUT_EP);
	//res = USBD_LL_StallEP(&hUsbDeviceFS, MSC_IN_EP);
	//res = MSC_BOT_Abort(&hUsbDeviceFS);
	res = USBD_MSC_DeInit(&hUsbDeviceFS, 0);
	uartprintf("res: %d", res);
}

void MX_USB_DEVICE_Resume(void)
{
	USBD_StatusTypeDef res;
	//USBD_LL_Resume(&hUsbDeviceFS);
	//res = USBD_LL_ClearStallEP(&hUsbDeviceFS, MSC_OUT_EP);
	//res = USBD_LL_ClearStallEP(&hUsbDeviceFS, MSC_IN_EP);
	res = USBD_MSC_Init(&hUsbDeviceFS, 0);
	uartprintf("res: %d", res);
}

/**
  * Init USB device Library, add supported class and start the library
  * @retval None
  */
void usb_comp_dev_init(void)
{
	HAL_PWREx_EnableUSBVoltageDetector(); // should be before Init

  /* Init Device Library, add supported class and start the library. */
  if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_MSC_CDC_ClassDriver) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_MSC_RegisterStorage(&hUsbDeviceFS, &USBD_Storage_Interface_fops_FS) != USBD_OK)
  {
    Error_Handler();
  }
  if (USBD_Start(&hUsbDeviceFS) != USBD_OK)
  {
    Error_Handler();
  }

}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
