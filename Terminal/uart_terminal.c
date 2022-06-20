/**
 * @author Katukiya G.
 * 
 * @file uart_terminal.h
 *
 * @brief Communication control with terminal by uart interface.
 */

#include "uart_terminal.h"
 
#include "main.h" // Error_Handler();

#include <string.h>
#include <stdio.h>

#include "modules.h"

static UART_HandleTypeDef huart4;
static uint8_t UartTermRxBuf[MAX_TERM_CMD_SIZE] = {0,};


//--------------------------------------------------- Private functions --------------------------------------------//
static void uart4_init(void);
/// @brief Return the string from terminal received by uart
static uint8_t *uart_terminal_scanf(void);
//------------------------------------------------------------------------------------------------------------------//


/**
 * @brief Initialize uart handler in uart_terminal
 * @param   huart  Uart handler created by CubeMx
 * @return  HAL_OK - success,
 *          HAL_ERROR - uart handler was not defined
 */
HAL_StatusTypeDef uart_terminal_init(void){
	uart4_init();
}

/**
 * @brief Print strings by uart transmit
 * @param   string  pointer to char array , consists of letters, words and etc.
 * @return  HAL_OK - success,
 *          HAL_ERROR - string transmission by uart was failed
 */
HAL_StatusTypeDef uart_terminal_print(uint8_t *string){

	if ( 0 != strlen(string) ){
		if ( HAL_OK == HAL_UART_Transmit(&huart4, string, strlen(string), UART_STR_PRINT_TIMEOUT) ){
			return HAL_OK;
		}
	}else{
		Error_Handler();
	}
}

/**
 * @brief Transmit large memory arrays in terminal
 * @param   memArr  pointer to array of memory
 * @param   len     number of bytes of memArr
 * @return  HAL_OK - success,
 *          HAL_ERROR - memory transmission by uart was failed
 */
HAL_StatusTypeDef uart_terminal_mem_transmit(uint8_t *memArr, uint16_t len){

	if ( 0 != len ){
		if ( HAL_OK == HAL_UART_Transmit(&huart4, memArr, len, UART_MEM_TRANSMIT_TIMEOUT) ){
			return HAL_OK;
		}
	}else{
		Error_Handler();
	}
}

// now only usb terminal is used for recieving cmds
#if 0
/**
 * @brief Define and add to the queue the command from terminal received by uart
 * @return  HAL_OK - success,
 *          HAL_ERROR - wrong command
 * @note terminal command format: module_name -function -parameter:x
 * 
 * @todo Сделать проверки корректности заполнения структур модулей,
 * 		 при неправильном заполнении бывает HardFault
 */

HAL_StatusTypeDef uart_terminal_cmd_def(void){
	uint16_t i=0;
	uint16_t moduleNmb = 0;
	uint8_t *rxBuf;
	uint8_t cmdStr[MAX_TERM_CMD_SIZE];
	const char *module_name = NULL;
	module_cmd_define_cb_t module_cmd_define_func = NULL;

	if ( NULL != (rxBuf = uart_terminal_scanf()) ){
		memmove(cmdStr, rxBuf, MAX_TERM_CMD_SIZE);

		while( i < MAX_TERM_CMD_SIZE){
			if ( CHAR_FILTER(cmdStr[i]) ){		// reach first character of module name, ignore whitespaces

				for ( moduleNmb = 0 ; moduleNmb < MODULES_NUMBER ; moduleNmb++){

					module_name = (const char *) *(psModules[moduleNmb]);
					if ( !strncmp(cmdStr+i, module_name, strlen(module_name)) ){
						// go to function for analyze command to module
						module_cmd_define_func = module_cmd_define_cbs[moduleNmb];
						module_cmd_define_func(cmdStr);
					}
				}
			} else { i++; };


			return HAL_OK;
		}
	} else {
		return HAL_ERROR;
	}
}
#endif
//------------------------------------------------------------------------------------------------------------------//

//--------------------------------------------------- Private functions --------------------------------------------//

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void uart4_init(void)
{

  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXOVERRUNDISABLE_INIT|UART_ADVFEATURE_DMADISABLEONERROR_INIT;
  huart4.AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;
  huart4.AdvancedInit.DMADisableonRxError = UART_ADVFEATURE_DMA_DISABLEONRXERROR;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart4, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart4, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart4) != HAL_OK)
  {
    Error_Handler();
  }

}

/**
 * @brief Return the string from terminal received by uart
 * @return  pointer to uart Rx buffer - success,
 *          NULL - memory transmission by uart was failed
 * @note we receive from terminal just short commands, no any large arrays of memory
 */
static uint8_t * uart_terminal_scanf(void){

	memset(UartTermRxBuf, 0, sizeof(UartTermRxBuf));
	/**
	 * @note 1. for normal working of uart interface it`s needed to 
	 * 			set uart GPIO pins in High/Very High Speed mode
	 * 		 2. we don`t check the answer of HAL_UART_Receive(), 
	 * 			because this function returns HAL_TIMEOUT 
	 * 			if packet is less than MAX_TERM_CMD_SIZE
	 */
	HAL_UART_Receive(&huart4, UartTermRxBuf, MAX_TERM_CMD_SIZE, UART_CMD_SCAN_TIMEOUT);
	if ( strlen(UartTermRxBuf) > 0 ){
		// check the end byte of conversation
		for(uint32_t i = 0; i < MAX_TERM_CMD_SIZE ; i++){
			if ( 0x0D == UartTermRxBuf[i] )   // CR (0x0D) must be enabled in terminal
						return UartTermRxBuf;
		}
	}

	Error_Handler();

	return NULL;
}


//------------------------------------------------------------------------------------------------------------------//
