
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

HAL_StatusTypeDef camera_init(void);
HAL_StatusTypeDef camera_on(void);
HAL_StatusTypeDef camera_off(void);
HAL_StatusTypeDef camera_reg_config(uint8_t reg_addr, uint8_t data);
HAL_StatusTypeDef camera_config(const uint8_t camConf[][2]);
HAL_StatusTypeDef camera_default_config(void);
HAL_StatusTypeDef camera_get_frame(struct sCamera_t *camera, sFrameBuf_t *frameBuf);


#endif /* CAMERA_H */
