/**
 * @author Katukiya G.
 *
 * @file iencoder_task.c
 *
 * @brief Incremental encoder task.
 */

#include "iencoder_task.h"
#include "stm32h7xx_hal.h"

#include "cmsis_os2.h"

#include "terminal.h"

#include "modules.h"
#include "iencoder.h"
#include "stepM.h"


extern osThreadId_t controlTaskHandle;
extern osSemaphoreId_t calibrationSem;
extern osMessageQueueId_t cmdQueueHandler;


void IencoderTask(void *argument)
{
	RotDirection_t rotDir;
	uint16_t steps;

	// wait until switching to calibration mode
    osSemaphoreAcquire(calibrationSem, osWaitForever);
	
	iencoder_init();
	iencoder_on();
	stepM_set_speed(5);
	stepM_on();

  	for(;;)
  	{
		if ( osMessageQueueGetCount (cmdQueueHandler) > 0 )
		{
			// before going to ControlTask, 
			// it is necessary to turn off the encoder and stepM
			iencoder_off();
			stepM_off();

			osThreadResume(controlTaskHandle);
			osSemaphoreAcquire(calibrationSem, osWaitForever);

			// and turn on when return
			iencoder_on();
			stepM_on();
		}

		rotDir = iencoder_get_dir();
		// turning one step is too slow, so we set the coefficient
		steps = iencoder_get_steps() * 10;	

		// uartprintf("Iencoder - dir: %d, steps: %d", rotDir, steps);
		// usbprintf("Iencoder - dir: %d, steps: %d", rotDir, steps);

		stepM_set_dir(rotDir);
		stepM_rotate(steps);
		// check encoder value 2 times a second
		osDelay(500); 
  }

}
