
#include "camera_hw_init.h"
#include "camera_hw_i2c.h"
#include "camera_hw_dcmi.h"
#include "main.h"  // Error_Handler();

#define CAM_En_Pin GPIO_PIN_2
#define CAM_En_GPIO_Port GPIOC
#define CAM_Reset_Pin GPIO_PIN_2
#define CAM_Reset_GPIO_Port GPIOA
#define CAM_Pwdn_Pin GPIO_PIN_3
#define CAM_Pwdn_GPIO_Port GPIOA

// todo: убрать в соответствующие модули
#define DRV_Dir_Pin GPIO_PIN_5
#define DRV_Dir_GPIO_Port GPIOA
#define DRV_Step_Pin GPIO_PIN_7
#define DRV_Step_GPIO_Port GPIOA
#define DRV_Reset_Pin GPIO_PIN_5
#define DRV_Reset_GPIO_Port GPIOC

#define Detect_SDIO_Pin GPIO_PIN_0
#define Detect_SDIO_GPIO_Port GPIOB


static void MX_GPIO_Init(void);


HAL_StatusTypeDef camera_hw_init(void){
	MX_GPIO_Init();
	MX_I2C2_Init();
	MX_DMA_Init();
	MX_DCMI_Init();
	return HAL_OK;
}

HAL_StatusTypeDef camera_hw_on(void){
    // Подача питания на камеру и переход в режим готовности
    HAL_GPIO_WritePin(CAM_En_GPIO_Port, CAM_En_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, CAM_Pwdn_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, CAM_Reset_Pin, GPIO_PIN_SET);
    // Включение тактирования
    HAL_RCC_MCOConfig(RCC_MCO2, RCC_MCO2SOURCE_HSE, RCC_MCODIV_1);
}

HAL_StatusTypeDef camera_hw_off(void){
        // Подача питания на камеру и переход в режим готовности
        HAL_GPIO_WritePin(CAM_En_GPIO_Port, CAM_En_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOA, CAM_Pwdn_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOA, CAM_Reset_Pin, GPIO_PIN_RESET);
}

HAL_StatusTypeDef camera_hw_get_frame(sFrameBuf_t *frameBuf){
    HAL_StatusTypeDef result;

    //result = HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t) frame_buffer, IMG_ROWS * IMG_COLUMNS / 2);
    //result = HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t) new_frame_buffer.pFrameBuf3, IMG_ROWS * IMG_COLUMNS / 2);
    result = HAL_DCMI_MultiBufferStart_DMA(DCMI_MODE_SNAPSHOT, frameBuf, frameBuf->size);
	return HAL_OK;
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CAM_En_GPIO_Port, CAM_En_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, CAM_Reset_Pin|CAM_Pwdn_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, DRV_Dir_Pin|DRV_Step_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DRV_Reset_GPIO_Port, DRV_Reset_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : CAM_En_Pin */
  GPIO_InitStruct.Pin = CAM_En_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(CAM_En_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : CAM_Reset_Pin CAM_Pwdn_Pin */
  GPIO_InitStruct.Pin = CAM_Reset_Pin|CAM_Pwdn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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

  /*Configure GPIO pins : Detect_SDIO_Pin PB2 */
  GPIO_InitStruct.Pin = Detect_SDIO_Pin|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}
