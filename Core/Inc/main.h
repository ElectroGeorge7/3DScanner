/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"


//void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);


/* Private defines -----------------------------------------------------------*/
#define CAM_En_Pin GPIO_PIN_2
#define CAM_En_GPIO_Port GPIOC
#define CAM_Reset_Pin GPIO_PIN_2
#define CAM_Reset_GPIO_Port GPIOA
#define CAM_Pwdn_Pin GPIO_PIN_3
#define CAM_Pwdn_GPIO_Port GPIOA

#define DRV_Dir_Pin GPIO_PIN_5
#define DRV_Dir_GPIO_Port GPIOA
#define DRV_Step_Pin GPIO_PIN_7
#define DRV_Step_GPIO_Port GPIOA
#define DRV_Reset_Pin GPIO_PIN_5
#define DRV_Reset_GPIO_Port GPIOC

#define Detect_SDIO_Pin GPIO_PIN_0
#define Detect_SDIO_GPIO_Port GPIOB


#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
