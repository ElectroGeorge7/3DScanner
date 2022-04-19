#ifndef CAMERA_HW_INIT_H_
#define CAMERA_HW_INIT_H_

#include "stm32h7xx_hal.h"
#include "camera.h"

__section (".ram_d3") HAL_StatusTypeDef camera_hw_init(void);
__section (".ram_d3") HAL_StatusTypeDef camera_hw_get_frame(sFrameBuf_t *frameBuf);

#endif /* CAMERA_HW_INIT_H_ */
