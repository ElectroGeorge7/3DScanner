/**
 * @author Katukiya G.
 *
 * @file uart_terminal.h 
 *
 * @brief Communication control with terminal by uart interface.
 */

#ifndef UART_TERMINAL_H
#define UART_TERMINAL_H

#include "string.h"

#include "stm32h7xx_hal.h"
#include "modules.h"

#define UART_STR_PRINT_TIMEOUT     0xfff
#define UART_CMD_SCAN_TIMEOUT      0xff
#define UART_MEM_TRANSMIT_TIMEOUT  0xffffff

#define MAX_TERM_CMD_SIZE          75

#define CHAR_FILTER(x)             ( ( '-' == (x) ) | ( '_' == (x) )   | \
                                   ( ( '0' <= (x) ) & ( '9' >= (x) ) ) | \
                                   ( ( 'A' <= (x) ) & ( 'Z' >= (x) ) ) | \
                                   ( ( 'a' <= (x) ) & ( 'z' >= (x) ) ) )

//--------------------------------------------------- Public functions ---------------------------------------------//
/// @brief Initialize uart handler in uart_terminal
HAL_StatusTypeDef uart_terminal_init(UART_HandleTypeDef *huart);

/// @brief Print strings in terminal by uart transmit
HAL_StatusTypeDef uart_terminal_print(uint8_t *string);

/// @brief Transmit large memory arrays in terminal
HAL_StatusTypeDef uart_terminal_mem_transmit(uint8_t *memArr, uint16_t len);

/// @brief Define and add to the queue the command from terminal received by uart
HAL_StatusTypeDef uart_terminal_cmd_def(void);
//------------------------------------------------------------------------------------------------------------------//

//--------------------------------------------------- Private functions --------------------------------------------//
/// @brief Return the string from terminal received by uart
uint8_t *uart_terminal_scanf(void);
//------------------------------------------------------------------------------------------------------------------//

#endif /* UART_TERMINAL_H */
// Список команд:
// вывод строки (в функцию передаётся строка)
// чтение команды, занесение команды в очередь, создать массив указателей на функции команд/на переход к таску команды
// передача большого массива данных (изображений) (на вход подаётся указатель на массив)

