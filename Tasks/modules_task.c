/*
 * modules_task.c
 *
 *  Created on: 19 апр. 2022 г.
 *      Author: George
 */

#include "modules_task.h"

#include "stm32h7xx_hal.h"


void ModulesTask(void *argument)
{
/*
	DRV_GPIO_Init();

    HAL_GPIO_WritePin(DRV_Reset_GPIO_Port, DRV_Reset_Pin, GPIO_PIN_SET);

	HAL_GPIO_WritePin(GPIOA, DRV_Dir_Pin, GPIO_PIN_RESET);
	for (uint32_t i=0; i<100; i++){
		HAL_GPIO_WritePin(GPIOA, DRV_Step_Pin, GPIO_PIN_RESET);
		osDelay(10);
		HAL_GPIO_WritePin(GPIOA, DRV_Step_Pin, GPIO_PIN_SET);
		osDelay(10);
	}
*/
	osDelay(2000);

  for(;;)
  {
	  osThreadYield();
/*
	for (uint32_t i=0; i<100; i++){
		HAL_GPIO_WritePin(GPIOA, DRV_Step_Pin, GPIO_PIN_RESET);
		osDelay(10);
		HAL_GPIO_WritePin(GPIOA, DRV_Step_Pin, GPIO_PIN_SET);
		osDelay(10);
	}

	osDelay(2000);

	HAL_GPIO_WritePin(GPIOA, DRV_Dir_Pin, GPIO_PIN_SET);

	for (uint32_t i=0; i<50; i++){
		HAL_GPIO_WritePin(GPIOA, DRV_Step_Pin, GPIO_PIN_RESET);
		osDelay(10);
		HAL_GPIO_WritePin(GPIOA, DRV_Step_Pin, GPIO_PIN_SET);
		osDelay(10);
	}

	osDelay(2000);
*/
  }

}
