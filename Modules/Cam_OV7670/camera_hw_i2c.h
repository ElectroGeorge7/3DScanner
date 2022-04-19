
#ifndef CAMERA_HW_I2C_H_
#define CAMERA_HW_I2C_H_

#include <stdint.h>
#include "stm32h7xx_hal.h"

__section (".ram_d3") void MX_I2C2_Init(void);
__section (".ram_d3") HAL_StatusTypeDef SCCB_write_reg(uint8_t reg_addr, uint8_t data);


#endif /* CAMERA_HW_I2C_H_ */
