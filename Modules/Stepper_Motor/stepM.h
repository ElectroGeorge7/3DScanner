/*
 * stepM.h
 *
 *  Created on: 30 апр. 2022 г.
 *      Author: George
 */

#ifndef STEPM_H_
#define STEPM_H_

#include "modules.h"

#define DRV_Dir_Pin GPIO_PIN_5
#define DRV_Dir_GPIO_Port GPIOA
#define DRV_Step_Pin GPIO_PIN_7
#define DRV_Step_GPIO_Port GPIOA
#define DRV_Reset_Pin GPIO_PIN_5
#define DRV_Reset_GPIO_Port GPIOC

ScannerStatus_t stepM_init(void);
ScannerStatus_t stepM_on(void);
ScannerStatus_t stepM_off(void);
ScannerStatus_t stepM_set_dir(RotDirection_t rotDir);
ScannerStatus_t stepM_set_speed(uint8_t speedVal);
ScannerStatus_t stepM_rotate(uint16_t steps);

#endif /* STEPPER_MOTOR_STEPM_H_ */
