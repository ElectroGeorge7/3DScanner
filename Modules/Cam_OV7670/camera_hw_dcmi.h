/*
 * camera_hw_dcmi.h
 *
 *  Created on: 12 апр. 2022 г.
 *      Author: George
 */

#ifndef CAMERA_HW_DCMI_H_
#define CAMERA_HW_DCMI_H_

#include <stdint.h>

#include "stm32h7xx_hal.h"
#include "camera.h"

void MX_DMA_Init(void);

void MX_DCMI_Init(void);
HAL_StatusTypeDef HAL_DCMI_MultiBufferStart_DMA( uint32_t DCMI_Mode, 	  	 \
												 sFrameBuf_t *frameBuf, 	 \
												uint32_t Length);

#endif /* CAMERA_HW_DCMI_H_ */
