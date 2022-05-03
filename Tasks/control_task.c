/*
 * control_task.c
 *
 *  Created on: 19 апр. 2022 г.
 *      Author: George
 */

#include "terminal.h"

#include "cmsis_os2.h"

#include "uart_terminal.h"
#include "modules.h"

#include "laser.h"
#include "stepM.h"
#include "camera.h"

#include <stdio.h>
#include <string.h>
#include "modules_task.h"
#include "stm32h7xx_hal.h"

extern osMessageQueueId_t cmdQueueHandler;
extern osMessageQueueId_t cameraQueueHandler;
extern osEventFlagsId_t cameraEvtId;

typedef enum{
	WAIT,
    SCANNING,
    CALIBRATION
} ScannerMode_t;

static ScannerMode_t gScannerMode = WAIT;

extern sFrameBuf_t new_frame_buffer;

ScannerStatus_t scan_start(const char *objName){
	ScannerStatus_t res = SCANNER_OK;
    osStatus_t osRes;
	CameraQueueObj_t cameraMsg;
	uint32_t flags;
	char pictureName[CAMERA_FILES_MAX_LENGTH] = {0};
	char picturePath[2*CAMERA_FILES_MAX_LENGTH] = {0};

	laser_on();
	camera_on();
	stepM_on();
	stepM_set_dir(CW);
	stepM_set_speed(10);

	// создать диреторию
	strncpy(cameraMsg.dirName, objName, CAMERA_FILES_MAX_LENGTH);
	osRes = osMessageQueuePut (cameraQueueHandler, &cameraMsg, 0, 0);
	osEventFlagsSet(cameraEvtId, CAMERA_EVT_DIR_CREATE);
	flags = osEventFlagsWait(cameraEvtId, CAMERA_EVT_DIR_CREATE_DONE, osFlagsWaitAny, osWaitForever);
	if ( !(flags & CAMERA_EVT_DIR_CREATE_DONE) )
		usbprintf("Dir creating error");
	// цикл (12 раз - 360гр.)
	for (uint8_t i=0; i < 8; i++){
		// включить лазер
		laser_set(250);
		// получить фрейм
		camera_get_frame(&new_frame_buffer);
		// сохранить фотографию
		cameraMsg.frameBuf = (uint32_t *) &new_frame_buffer;
		sprintf(pictureName, "im%d.rgb", 2*i);
		sprintf(picturePath, "%s/%s", objName, pictureName);
		strncpy(cameraMsg.fileName, picturePath, CAMERA_FILES_MAX_LENGTH);
		osRes = osMessageQueuePut (cameraQueueHandler, &cameraMsg, 0, 0);
		osEventFlagsSet(cameraEvtId, CAMERA_EVT_FILE_CREATE);
		flags = osEventFlagsWait(cameraEvtId, CAMERA_EVT_FILE_CREATE_DONE, osFlagsWaitAny, osWaitForever);
		if ( !(flags & CAMERA_EVT_FILE_CREATE_DONE) )
			usbprintf("File creating error");
		// отключить лазер
		laser_set(0);
		// получить фрейм
		camera_get_frame(&new_frame_buffer);
		// сохранить фотографию
		cameraMsg.frameBuf = (uint32_t *) &new_frame_buffer;
		sprintf(pictureName, "im%d.rgb", 2*i+1);
		sprintf(picturePath, "%s/%s", objName, pictureName);
		strncpy(cameraMsg.fileName, picturePath, CAMERA_FILES_MAX_LENGTH);
		osRes = osMessageQueuePut (cameraQueueHandler, &cameraMsg, 0, 0);
		osEventFlagsSet(cameraEvtId, CAMERA_EVT_FILE_CREATE);
		flags = osEventFlagsWait(cameraEvtId, CAMERA_EVT_FILE_CREATE_DONE, osFlagsWaitAny, osWaitForever);
		if ( !(flags & CAMERA_EVT_FILE_CREATE_DONE) )
			usbprintf("File creating error");
		// повернуть на 30гр.
		stepM_rotate(25);
	}
};

/**
  * @brief  Function implementing the controlTask thread.
  * @param  argument: Not used
  * @retval None
  */
void ControlTask(void *argument)
{
	osStatus_t res;
	char msg[256];
	char strBuf[50];

	ModulesHandle_t *phModules;
	ModuleId_t moduleId;
	ModuleCmdCb_t moduleCmd = NULL;

	phModules = modules_init();
	if ( phModules == NULL){
		usbprintf("Modules Init Error");
		uartprintf("Modules Init Error");
	};

  	laser_init();
  	camera_init();
	stepM_init();
/*
  HAL_Delay(2000);

  uart_terminal_init(&huart4);
  uart_terminal_print("Control task start!\n");
	uint8_t str[] = "Hello!\n";
	uint8_t res = 0;
	uartprintf("Hello, %s", str);
	usbprintf("Hello, %s", str);
	uartprintf("Hello, %d", 10);
	usbprintf("Hello, %d", 10);
*/
  for(;;)
  {

   res = osMessageQueueGet(cmdQueueHandler, msg, 0, osWaitForever);

    if( res == osOK )
    {
    	uartprintf(msg);
//    	usbprintf(ucRxData);

    	sscanf(msg, "%s", strBuf);
		if (!strncmp(strBuf, "scan", strlen("scan"))){
			gScannerMode = SCANNING;
			sscanf(msg, "%*s -object:%s", strBuf);
			// запустить сканирование
			scan_start(strBuf);
			usbprintf("scan object name - %s", strBuf);
			osDelay(1000);
			}
		else if (!strncmp(strBuf, "calibrate", strlen("calibrate"))){
			gScannerMode = CALIBRATION;
			// просто переход в режим калибровки и ожидание других команд
			usbprintf("calibrate");
			osDelay(1000);
			}
		else if (!strncmp(strBuf, "exit", strlen("exit"))){
				usbprintf("exit");
				// прекращение всех процессов и переход к начальному состоянию
				osDelay(1000);
			}
		else if (!module_cmd_parse(phModules, msg, &moduleId)){
			if (gScannerMode == CALIBRATION){
				// go to function to execute module`s cmd
				moduleCmd = phModules->moduleCmdCbs[moduleId];
				moduleCmd();
			} else {
				usbprintf("go to calibration mode");
			}
		}
		else {
			usbprintf("wrong command");
		}


    }

    osDelay(100);
  }

}
