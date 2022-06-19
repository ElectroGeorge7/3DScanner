/**
 * @author Katukiya G.
 *
 * @file iencoder.c
 *
 * @brief Incremental encoder control.
 */

#include "iencoder.h"

#include "main.h"  // Error_Handler();


TIM_HandleTypeDef htim4;

//--------------------------------------------------- Private functions --------------------------------------------//
static void MX_TIM4_Init(void);
//------------------------------------------------------------------------------------------------------------------//

/// @brief Init TIM in encoder mode
ScannerStatus_t iencoder_init(void){
	MX_TIM4_Init();
	return SCANNER_OK;
};

/// @brief Start TIM in encoder mode
ScannerStatus_t iencoder_on(void){
	HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
	return SCANNER_OK;
};

/// @brief Stop TIM in encoder mode
ScannerStatus_t iencoder_off(void){
	HAL_TIM_Encoder_Stop(&htim4, TIM_CHANNEL_ALL);
	return SCANNER_OK;
};

/// @brief Get DIR bit value from TIM`s CR1 register
/// @note Shows the direction of the last encoder move
RotDirection_t iencoder_get_dir(void){
	return __HAL_TIM_IS_TIM_COUNTING_DOWN(&htim4) ? CW : CCW;
};

/// @brief Get TIM`s CNT register value
uint16_t iencoder_get_steps(void){
	uint16_t cntrVal =  __HAL_TIM_GET_COUNTER(&htim4);
  // suppose that in one check of the encoder 
  // the shaft will turn a maximum of 360 deg ( 20 steps * 2 fronts)
	if (cntrVal > 40)
		cntrVal = 81 - cntrVal;
  // reset the counter value
	__HAL_TIM_SET_COUNTER(&htim4, 0);
  // each step increments CNT by 2 
	return (cntrVal / 2);
};


//--------------------------------------------------- Private functions --------------------------------------------//

/**
* @brief TIM_Encoder MSP Initialization
* This function configures the hardware resources used in this example
* @param htim_encoder: TIM_Encoder handle pointer
* @retval None
*/
void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef* htim_encoder)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(htim_encoder->Instance==TIM4)
  {
    /* Peripheral clock enable */
    __HAL_RCC_TIM4_CLK_ENABLE();

    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**TIM4 GPIO Configuration
    PD12     ------> TIM4_CH1
    PD13     ------> TIM4_CH2
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  }

}

/**
* @brief TIM_Encoder MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param htim_encoder: TIM_Encoder handle pointer
* @retval None
*/
void HAL_TIM_Encoder_MspDeInit(TIM_HandleTypeDef* htim_encoder)
{
  if(htim_encoder->Instance==TIM4)
  {
    /* Peripheral clock disable */
    __HAL_RCC_TIM4_CLK_DISABLE();

    /**TIM4 GPIO Configuration
    PD12     ------> TIM4_CH1
    PD13     ------> TIM4_CH2
    */
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_12|GPIO_PIN_13);

  }
}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{
  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 80;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI1;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 0;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim4, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

//------------------------------------------------------------------------------------------------------------------//