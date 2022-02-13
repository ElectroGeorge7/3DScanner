/**
  ******************************************************************************
  * @file    usbd_template_core.h
  * @author  MCD Application Team
  * @brief   Header file for the usbd_template_core.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_CDC_MSC_CORE_H
#define __USB_CDC_MSC_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_TEMPLATE
  * @brief This file is the header file for usbd_template_core.c
  * @{
  */


/** @defgroup USBD_TEMPLATE_Exported_Defines
  * @{
  */

#define USB_TEMPLATE_CONFIG_DESC_SIZ       64U


#define MSC_INTERFACE_IDX 0x0                            	// Index of MSC interface
#define CDC_INTERFACE_IDX 0x1                            	// Index of CDC interface

// endpoints numbers
// endpoints numbers
#define MSC_EP_IDX                      0x01
#define CDC_CMD_EP_IDX                  0x02
#define CDC_EP_IDX                      0x03

#define IN_EP_DIR						0x80 // Adds a direction bit

#define MSC_OUT_EP                      MSC_EP_IDX                  /* EP1 for BULK OUT */
#define MSC_IN_EP                       MSC_EP_IDX | IN_EP_DIR      /* EP1 for BULK IN */
#define CDC_CMD_EP                      CDC_CMD_EP_IDX| IN_EP_DIR   /* EP2 for CDC commands */
#define CDC_OUT_EP                      CDC_EP_IDX                  /* EP3 for data OUT */
#define CDC_IN_EP                       CDC_EP_IDX | IN_EP_DIR      /* EP3 for data IN */

#define USB_CDC_CONFIG_DESC_SIZ       98
/**
  * @}
  */


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */

/**
  * @}
  */



/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */

extern USBD_ClassTypeDef USBD_MSC_CDC_ClassDriver;
/**
  * @}
  */

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif  /* __USB_TEMPLATE_CORE_H */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
