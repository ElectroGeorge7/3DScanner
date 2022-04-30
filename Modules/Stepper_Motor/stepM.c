/*
 * stepM.c
 *
 *  Created on: 30 апр. 2022 г.
 *      Author: George
 */

#include "stepM.h"

#include "main.h"  // Error_Handler();
#include "stm32h7xx_hal.h"

static uint8_t gRotSpeed;

ScannerStatus_t stepM_init(void){
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, DRV_Dir_Pin|DRV_Step_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(DRV_Reset_GPIO_Port, DRV_Reset_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : DRV_Dir_Pin DRV_Step_Pin */
	GPIO_InitStruct.Pin = DRV_Dir_Pin|DRV_Step_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : DRV_Reset_Pin */
	GPIO_InitStruct.Pin = DRV_Reset_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(DRV_Reset_GPIO_Port, &GPIO_InitStruct);

	return SCANNER_OK;
};

ScannerStatus_t stepM_on(void){
	HAL_GPIO_WritePin(DRV_Reset_GPIO_Port, DRV_Reset_Pin, GPIO_PIN_SET);
	return SCANNER_OK;
};

ScannerStatus_t stepM_off(void){
	HAL_GPIO_WritePin(DRV_Reset_GPIO_Port, DRV_Reset_Pin, GPIO_PIN_RESET);
	return SCANNER_OK;
};

ScannerStatus_t stepM_set_dir(RotDirection_t rotDir){
	if ( rotDir == CW  )
		HAL_GPIO_WritePin(GPIOA, DRV_Dir_Pin, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(GPIOA, DRV_Dir_Pin, GPIO_PIN_RESET);
	return SCANNER_OK;
};

ScannerStatus_t stepM_set_speed(uint8_t speedVal){
  gRotSpeed = speedVal;
  return SCANNER_OK;
};

ScannerStatus_t stepM_rotate(uint16_t steps){

	while(steps--){
		HAL_GPIO_WritePin(GPIOA, DRV_Step_Pin, GPIO_PIN_RESET);
		HAL_Delay(gRotSpeed);
		HAL_GPIO_WritePin(GPIOA, DRV_Step_Pin, GPIO_PIN_SET);
		HAL_Delay(gRotSpeed);
	}

	return SCANNER_OK;
};
