#ifndef LASER_H_
#define LASER_H_

#include "modules.h"

HAL_StatusTypeDef laser_init(void);
HAL_StatusTypeDef laser_on(void);
HAL_StatusTypeDef laser_off(void);
HAL_StatusTypeDef laser_set(uint8_t bright);

#endif /* LASER_H_ */
