
#ifndef CAMERA_H
#define CAMERA_H

#include "stm32h7xx_hal.h"
#include "../Modules/modules.h"

typedef struct sFrameBuf{
	uint32_t *pFrameBuf1;
	uint32_t *pFrameBuf2;
	uint32_t *pFrameBuf3;
    uint32_t *pFrameBuf4;
    uint32_t size;
} sFrameBuf_t;

__section (".ram_d3") HAL_StatusTypeDef camera_init(void);
__section (".ram_d3") HAL_StatusTypeDef camera_on(void);
__section (".ram_d3") HAL_StatusTypeDef camera_off(void);
__section (".ram_d3") HAL_StatusTypeDef camera_config_reg(uint8_t reg_addr, uint8_t data);
__section (".ram_d3") HAL_StatusTypeDef camera_config(const uint8_t camConf[][2]);
__section (".ram_d3") HAL_StatusTypeDef camera_default_config(void);
__section (".ram_d3") HAL_StatusTypeDef camera_get_frame(sFrameBuf_t *frameBuf);


#endif /* CAMERA_H */
