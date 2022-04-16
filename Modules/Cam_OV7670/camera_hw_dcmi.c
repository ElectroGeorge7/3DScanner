
#include "camera_hw_dcmi.h"

#include "main.h"  // Error_Handler();


DCMI_HandleTypeDef ghdcmi;
DMA_HandleTypeDef hdma_dcmi;

static sFrameBuf_t *hwFrameBuf = NULL;

void MX_DMA_Init(void);

void HAL_DCMI_MspInit(DCMI_HandleTypeDef* hdcmi);
void HAL_DCMI_MspDeInit(DCMI_HandleTypeDef* hdcmi);
void MX_DCMI_Init(void);
static void DCMI_DMAError(DMA_HandleTypeDef *hdma);
void DCMI_DMAXferCpltMB2(DMA_HandleTypeDef *hdma);
HAL_StatusTypeDef HAL_DCMI_MultiBufferStart_DMA( uint32_t DCMI_Mode, 	   \
												 sFrameBuf_t *frameBuf,    \
												 uint32_t Length);

/**
  * Enable DMA controller clock
  */
void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 15, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);

}

/**
* @brief DCMI MSP Initialization
* This function configures the hardware resources used in this example
* @param hdcmi: DCMI handle pointer
* @retval None
*/
void HAL_DCMI_MspInit(DCMI_HandleTypeDef* hdcmi)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hdcmi->Instance==DCMI)
  {
    /* Peripheral clock enable */
    __HAL_RCC_DCMI_CLK_ENABLE();

    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**DCMI GPIO Configuration
    PE4     ------> DCMI_D4
    PE5     ------> DCMI_D6
    PE6     ------> DCMI_D7
    PA4     ------> DCMI_HSYNC
    PA6     ------> DCMI_PIXCLK
    PC6     ------> DCMI_D0
    PC7     ------> DCMI_D1
    PC8     ------> DCMI_D2
    PD3     ------> DCMI_D5
    PB7     ------> DCMI_VSYNC
    PE1     ------> DCMI_D3
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* DCMI DMA Init */
    /* DCMI Init */
    hdma_dcmi.Instance = DMA1_Stream0;
    hdma_dcmi.Init.Request = DMA_REQUEST_DCMI;
    hdma_dcmi.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_dcmi.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_dcmi.Init.MemInc = DMA_MINC_ENABLE;
    hdma_dcmi.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_dcmi.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_dcmi.Init.Mode = DMA_CIRCULAR;
    hdma_dcmi.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_dcmi.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_dcmi) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(hdcmi,DMA_Handle,hdma_dcmi);

  }

}

/**
* @brief DCMI MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hdcmi: DCMI handle pointer
* @retval None
*/
void HAL_DCMI_MspDeInit(DCMI_HandleTypeDef* hdcmi)
{
  if(hdcmi->Instance==DCMI)
  {
    /* Peripheral clock disable */
    __HAL_RCC_DCMI_CLK_DISABLE();

    /**DCMI GPIO Configuration
    PE4     ------> DCMI_D4
    PE5     ------> DCMI_D6
    PE6     ------> DCMI_D7
    PA4     ------> DCMI_HSYNC
    PA6     ------> DCMI_PIXCLK
    PC6     ------> DCMI_D0
    PC7     ------> DCMI_D1
    PC8     ------> DCMI_D2
    PD3     ------> DCMI_D5
    PB7     ------> DCMI_VSYNC
    PE1     ------> DCMI_D3
    */
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_1);

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4|GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_3);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);

    /* DCMI DMA DeInit */
    HAL_DMA_DeInit(hdcmi->DMA_Handle);

  }

}

/**
  * @brief DCMI Initialization Function
  * @param None
  * @retval None
  */
void MX_DCMI_Init(void)
{

  ghdcmi.Instance = DCMI;
  ghdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
  ghdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;
  ghdcmi.Init.VSPolarity = DCMI_VSPOLARITY_HIGH;
  ghdcmi.Init.HSPolarity = DCMI_HSPOLARITY_LOW;
  ghdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
  ghdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
  ghdcmi.Init.JPEGMode = DCMI_JPEG_DISABLE;
  ghdcmi.Init.ByteSelectMode = DCMI_BSM_ALL;
  ghdcmi.Init.ByteSelectStart = DCMI_OEBS_ODD;
  ghdcmi.Init.LineSelectMode = DCMI_LSM_ALL;
  ghdcmi.Init.LineSelectStart = DCMI_OELS_ODD;
  if (HAL_DCMI_Init(&ghdcmi) != HAL_OK)
  {
    Error_Handler();
  }



}

/**
  * @brief  DMA error callback
  * @param  hdma pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
static void DCMI_DMAError(DMA_HandleTypeDef *hdma)
{
  DCMI_HandleTypeDef *hdcmi = (DCMI_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

  if (hdcmi->DMA_Handle->ErrorCode != HAL_DMA_ERROR_FE)
  {
    /* Initialize the DCMI state*/
    hdcmi->State = HAL_DCMI_STATE_READY;

    /* Set DCMI Error Code */
    hdcmi->ErrorCode |= HAL_DCMI_ERROR_DMA;
  }

  /* DCMI error Callback */
#if (USE_HAL_DCMI_REGISTER_CALLBACKS == 1)
  /*Call registered DCMI error callback*/
  hdcmi->ErrorCallback(hdcmi);
#else
  HAL_DCMI_ErrorCallback(hdcmi);
#endif /* USE_HAL_DCMI_REGISTER_CALLBACKS */
}


void DCMI_DMAXferCpltMB2(DMA_HandleTypeDef *hdma)
{
  uint32_t tmp ;

  DCMI_HandleTypeDef *hdcmi = (DCMI_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

  if (hdcmi->XferCount != 0U)
  {
    /* Update memory 0 address location */
    tmp = ((((DMA_Stream_TypeDef *)(hdcmi->DMA_Handle->Instance))->CR) & DMA_SxCR_CT);
    if (((hdcmi->XferCount % 2U) == 0U) && (tmp != 0U))
    {
      tmp = ((DMA_Stream_TypeDef *)(hdcmi->DMA_Handle->Instance))->M0AR;
      (void)HAL_DMAEx_ChangeMemory(hdcmi->DMA_Handle, (uint32_t)(hwFrameBuf->pFrameBuf3), MEMORY0);
      hdcmi->XferCount--;
    }
    /* Update memory 1 address location */
    else if ((((DMA_Stream_TypeDef *)(hdcmi->DMA_Handle->Instance))->CR & DMA_SxCR_CT) == 0U)
    {
      tmp = ((DMA_Stream_TypeDef *)(hdcmi->DMA_Handle->Instance))->M1AR;
      (void)HAL_DMAEx_ChangeMemory(hdcmi->DMA_Handle, (uint32_t)(hwFrameBuf->pFrameBuf4), MEMORY1);
      hdcmi->XferCount--;
    }
    else
    {
      /* Nothing to do */
    }
  }
  /* Update memory 0 address location */
  else if ((((DMA_Stream_TypeDef *)(hdcmi->DMA_Handle->Instance))->CR & DMA_SxCR_CT) != 0U)
  {
    ((DMA_Stream_TypeDef *)(hdcmi->DMA_Handle->Instance))->M0AR = hdcmi->pBuffPtr;
  }
  /* Update memory 1 address location */
  else if ((((DMA_Stream_TypeDef *)(hdcmi->DMA_Handle->Instance))->CR & DMA_SxCR_CT) == 0U)
  {
    tmp = hdcmi->pBuffPtr;
    ((DMA_Stream_TypeDef *)(hdcmi->DMA_Handle->Instance))->M1AR = (uint32_t)(hwFrameBuf->pFrameBuf2);
    hdcmi->XferCount = hdcmi->XferTransferNumber;
  }
  else
  {
    /* Nothing to do */
  }

  /* Check if the frame is transferred */
  if (hdcmi->XferCount == hdcmi->XferTransferNumber)
  {
    /* Enable the Frame interrupt */
    __HAL_DCMI_ENABLE_IT(hdcmi, DCMI_IT_FRAME);

    /* When snapshot mode, set dcmi state to ready */
    if ((hdcmi->Instance->CR & DCMI_CR_CM) == DCMI_MODE_SNAPSHOT)
    {
      hdcmi->State = HAL_DCMI_STATE_READY;
    }
  }
}


/**
  * @brief  Enables DCMI DMA request and enables DCMI capture
  * @param  hdcmi     pointer to a DCMI_HandleTypeDef structure that contains
  *                    the configuration information for DCMI.
  * @param  DCMI_Mode DCMI capture mode snapshot or continuous grab.
  * @param  pData     The destination memory Buffer address (LCD Frame buffer).
  * @param  Length    The length of capture to be transferred.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DCMI_MultiBufferStart_DMA(uint32_t DCMI_Mode, sFrameBuf_t *frameBuf, uint32_t Length)
{
	DCMI_HandleTypeDef *hdcmi;
	hdcmi = &ghdcmi;

  /* Check function parameters */
  assert_param(IS_DCMI_CAPTURE_MODE(DCMI_Mode));

  /* Process Locked */
  __HAL_LOCK(hdcmi);

  hwFrameBuf = frameBuf;

  /* Lock the DCMI peripheral state */
  hdcmi->State = HAL_DCMI_STATE_BUSY;

  /* Enable DCMI by setting DCMIEN bit */
  __HAL_DCMI_ENABLE(hdcmi);

  /* Configure the DCMI Mode */
  hdcmi->Instance->CR &= ~(DCMI_CR_CM);
  hdcmi->Instance->CR |= (uint32_t)(DCMI_Mode);

  /* Set the DMA memory0 conversion complete callback */
  hdcmi->DMA_Handle->XferCpltCallback = DCMI_DMAXferCpltMB2;

  /* Set the DMA error callback */
  hdcmi->DMA_Handle->XferErrorCallback = DCMI_DMAError;

  /* Set the dma abort callback */
  hdcmi->DMA_Handle->XferAbortCallback = NULL;

    /* DCMI_DOUBLE_BUFFER Mode */
    /* Set the DMA memory1 conversion complete callback */
    hdcmi->DMA_Handle->XferM1CpltCallback = DCMI_DMAXferCpltMB2;

    /* Initialize transfer parameters */
    hdcmi->XferCount = 2;
    hdcmi->XferTransferNumber = hdcmi->XferCount;
    //hdcmi->XferSize = Length / 3;
    hdcmi->XferSize = Length / 4;
    hdcmi->pBuffPtr = (uint32_t)(frameBuf->pFrameBuf1);

    /* Start DMA multi buffer transfer */
    if (HAL_DMAEx_MultiBufferStart_IT(hdcmi->DMA_Handle, (uint32_t)&hdcmi->Instance->DR, (uint32_t)(frameBuf->pFrameBuf1),
                                        (uint32_t)(frameBuf->pFrameBuf2), hdcmi->XferSize ) != HAL_OK)
    {
      /* Set Error Code */
      hdcmi->ErrorCode = HAL_DCMI_ERROR_DMA;
      /* Change DCMI state */
      hdcmi->State = HAL_DCMI_STATE_READY;
      /* Release Lock */
      __HAL_UNLOCK(hdcmi);
      /* Return function status */
      return HAL_ERROR;
    }

  /* Enable Capture */
  hdcmi->Instance->CR |= DCMI_CR_CAPTURE;

  /* Release Lock */
  __HAL_UNLOCK(hdcmi);

  HAL_DCMI_Stop(hdcmi);

  /* Return function status */
  return HAL_OK;
}

