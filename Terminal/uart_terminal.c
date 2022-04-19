/**
 * @author Katukiya G.
 *
 * @file uart_terminal.h
 *
 * @brief Communication control with terminal by uart interface.
 */

#include "../Terminal/uart_terminal.h"
#include "stm32h7xx_hal.h"

static UART_HandleTypeDef *ghuart;
static uint8_t UartTermRxBuf[MAX_TERM_CMD_SIZE] = {0,};

//--------------------------------------------------- Public functions ---------------------------------------------//

/**
 * @brief Initialize uart handler in uart_terminal
 * @param   huart  Uart handler created by CubeMx
 * @return  HAL_OK - success,
 *          HAL_ERROR - uart handler was not defined
 */
HAL_StatusTypeDef uart_terminal_init(UART_HandleTypeDef *huart){

	if ( huart != NULL ){
		ghuart = huart;
		return   HAL_OK;
	}else{
		// создать сообщение об ошибке, и затем показать её в реестре ошибок.
	}
}

/**
 * @brief Print strings by uart transmit
 * @param   string  pointer to char array , consists of letters, words and etc.
 * @return  HAL_OK - success,
 *          HAL_ERROR - string transmission by uart was failed
 */
HAL_StatusTypeDef uart_terminal_print(uint8_t *string){

	if ( 0 != strlen(string) ){
		if ( HAL_OK == HAL_UART_Transmit(ghuart, string, strlen(string), UART_STR_PRINT_TIMEOUT) ){
			return HAL_OK;
		}
	}else{
		 // создать сообщение об ошибке, и затем показать её в реестре ошибок.
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
		if ( HAL_OK == HAL_UART_Transmit(ghuart, memArr, len, UART_MEM_TRANSMIT_TIMEOUT) ){
			return HAL_OK;
		}
	}else{
		// создать сообщение об ошибке, и затем показать её в реестре ошибок.
	}
}

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

//------------------------------------------------------------------------------------------------------------------//

//--------------------------------------------------- Private functions --------------------------------------------//

/**
 * @brief Return the string from terminal received by uart
 * @return  pointer to uart Rx buffer - success,
 *          NULL - memory transmission by uart was failed
 * @note we receive from terminal just short commands, no any large arrays of memory
 */
uint8_t * uart_terminal_scanf(void){

	memset(UartTermRxBuf, 0, sizeof(UartTermRxBuf));
	/**
	 * @note 1. for normal working of uart interface it`s needed to 
	 * 			set uart GPIO pins in High/Very High Speed mode
	 * 		 2. we don`t check the answer of HAL_UART_Receive(), 
	 * 			because this function returns HAL_TIMEOUT 
	 * 			if packet is less than MAX_TERM_CMD_SIZE
	 */
	HAL_UART_Receive(ghuart, UartTermRxBuf, MAX_TERM_CMD_SIZE, UART_CMD_SCAN_TIMEOUT);
	if ( strlen(UartTermRxBuf) > 0 ){
		// check the end byte of conversation
		for(uint32_t i = 0; i < MAX_TERM_CMD_SIZE ; i++){
			if ( 0x0D == UartTermRxBuf[i] )   // CR (0x0D) must be enabled in terminal
						return UartTermRxBuf;
		}
	}
	// создать сообщение об ошибке, и затем показать её в реестре ошибок.
	return NULL;
}



__section (".ram_d3") static char __aligned(32) uartprintbuf[256 + 2] = {'\0'};

void uartconsole_printf(const char* fmt, ...){
    //char printbuf[128 + 2] = {0};
    va_list args;
    size_t size = 0;
    va_start(args, fmt);
    int ret = vsnprintf(uartprintbuf, sizeof(uartprintbuf) - 2, fmt, args);
    va_end(args);
    if(ret < 0){
        return;
    }
    size += ret;
    uartprintbuf[size] = '\n';
    uartprintbuf[size + 1] = '\0';

    uart_terminal_print(uartprintbuf);

}

//------------------------------------------------------------------------------------------------------------------//
