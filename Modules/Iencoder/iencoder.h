/**
 * @author Katukiya G.
 *
 * @file iencoder.h
 *
 * @brief Incremental encoder control.
 */

#ifndef IENCODER_H_
#define IENCODER_H_

#include "modules.h"

ScannerStatus_t iencoder_init(void);
ScannerStatus_t iencoder_on(void);
ScannerStatus_t iencoder_off(void);
RotDirection_t iencoder_get_dir(void);
uint16_t iencoder_get_steps(void);


#endif /* IENCODER_H_ */
