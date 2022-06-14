/**
 * @author Katukiya G.
 *
 * @file modules.c
 *
 * @brief Structures of 3D scanner`s modules.
 */

#include "modules.h"

#include <string.h>
#include <stdlib.h>

#include "cmsis_os2.h"

#include "main.h"
#include "stm32h7xx_hal.h"
#include "terminal.h"

#include "laser.h"
#include "stepM.h"
#include "camera.h"
#include "camera_hw_i2c.h"

#include "storage_task.h"

extern osMessageQueueId_t cameraQueueHandler;
extern osEventFlagsId_t cameraEvtId;

static struct sLaser_t laser;
static struct sCamera_t camera;
static struct sStepM_t stepM;

static ScannerStatus_t laser_cmd_parser_cb(uint8_t *cmdStr);
static ScannerStatus_t camera_cmd_parser_cb(uint8_t *cmdStr);
static ScannerStatus_t stepM_cmd_parser_cb(uint8_t *cmdStr);

static ScannerStatus_t laser_cmd_cb(void);
static ScannerStatus_t camera_cmd_cb(void);
static ScannerStatus_t stepM_cmd_cb(void);

static ModulesHandle_t gModulesHandle = {

    .psModulesArray = {
        (uint32_t *) &laser,
        (uint32_t *) &camera,
        (uint32_t *) &stepM
    },

    .moduleCmdParserCbs = {
    	laser_cmd_parser_cb,
		camera_cmd_parser_cb,
		stepM_cmd_parser_cb
    },

    .moduleCmdCbs = {
        laser_cmd_cb,
        camera_cmd_cb,
        stepM_cmd_cb
    }

};

static struct sLaser_t laser = {
    "laser",
    0,
	{
        {
            { " -dis ", " -en " },
            DIS
        }
	},

	{
       " -bright:",
       0
	}
};


static struct sStepM_t stepM = {
    "stepM",
    0,
    {
        {
            { " -dis ", " -en "},
            DIS
        },

        {
            { " -cw ", " -ccw "},
            CW
        }
    },

    {
        {
            " -steps:",
            0
        },

        {
            " -speed:",
            0
        }
    }
};

static struct sCamera_t camera = {
    "camera",
    0,
    // list of camera function
    {
        // camera power func
        {
            { " -dis ", " -en "},
            DIS
        }
    },

    // list of camera params
    {
        // take one picture and save it as file "pictureName"
        {
            "-picture:",
			{0,}
        },

        // write in camera`s registers values from file "configFileName"
        {
            "-config:",
			{0,}
        },
        
        // write in one camera`s register "regName" the value "regVal"
        {
            "-reg:",
            0,
            0
        }
    }
};

ModulesHandle_t *modules_init(void){
    return (&gModulesHandle);
}

ScannerStatus_t module_cmd_parse(ModulesHandle_t *phModules, const char *cmdStr, ModuleId_t *moduleId){
	const char *module_name = NULL;
	ModuleCmdParserCb_t moduleCmdParserFunc = NULL;
	ScannerStatus_t res = SCANNER_ERROR;

	for ( uint8_t moduleNmb = 0 ; moduleNmb < MODULES_NUMBER ; moduleNmb++){
		module_name = (const char *) *(phModules->psModulesArray[moduleNmb]);
		if ( !strncmp(cmdStr, module_name, strlen(module_name)) ){
			// go to function for analyze command to module
			*moduleId = moduleNmb;
			moduleCmdParserFunc = phModules->moduleCmdParserCbs[moduleNmb];
			res = moduleCmdParserFunc(cmdStr);
		}
	}

	return res;
}

#define IMG_ROWS   					640
#define IMG_COLUMNS   				480
#define FRAME_BUF_PART_SIZE ( ( (IMG_ROWS * IMG_COLUMNS * 2) / 4) / 4 )
__section (".bss") uint32_t __aligned(32) frame_buffer_1[FRAME_BUF_PART_SIZE] = {0};
__section (".bss") uint32_t __aligned(32) frame_buffer_2[FRAME_BUF_PART_SIZE] = {0};
__section (".bss") uint32_t __aligned(32) frame_buffer_3[FRAME_BUF_PART_SIZE] = {0};
__section (".ram_d2") uint32_t __aligned(32) frame_buffer_4[FRAME_BUF_PART_SIZE] = {0};

sFrameBuf_t new_frame_buffer = {
    .pFrameBuf1 = frame_buffer_1,
    .pFrameBuf2 = frame_buffer_2,
    .pFrameBuf3 = frame_buffer_3,
    .pFrameBuf4 = frame_buffer_4,
    .size = 4 * FRAME_BUF_PART_SIZE
};

/*
HAL_StatusTypeDef result = HAL_OK;
static char readBuf[256] = {0};
static uint32_t br = 0;
HAL_StatusTypeDef camera_config(char *configFileName, char* readBuf, uint32_t bufSize){
    uint32_t regAddr = 0, regVal = 0;
    char *pEnd = NULL;
    uint32_t hexNumb = 0;
    do{
        fRead(configFileName, readBuf, bufSize, &br);
        if (pEnd == NULL){
            regAddr = strtol(readBuf, &pEnd, 16);
            regVal = strtol(pEnd, &pEnd, 16);
            SCCB_write_reg((uint8_t)regAddr,(uint8_t)regVal);
        };

        while( pEnd-readBuf < br){
            regAddr = strtol(pEnd, &pEnd, 16);
            regVal = strtol(pEnd, &pEnd, 16);
            SCCB_write_reg((uint8_t)regAddr,(uint8_t)regVal);
        }
    }while(br==bufSize);

    return HAL_OK;
}
*/

/**
 * @brief
 *
 * @param cmdStr
 * @return ScannerStatus_t
 */
static ScannerStatus_t laser_cmd_parser_cb(uint8_t *cmdStr){
    char *pParamStr;
    char *pParamVal;

    if (cmdStr == NULL)
        return SCANNER_ERROR;

    for (uint8_t i = 0; i < 2 ; i++){
        if ( strstr(cmdStr, laser.power_func_names[i]) ){
            laser.powerState = (PowerState_t) i;
            laser.status |= LASER_POWER_UPDATE_F;
        }
    };

    if ( pParamStr = strstr(cmdStr, laser.bright_param_name) ){
        pParamVal = pParamStr + strlen(laser.bright_param_name);
        if ( !sscanf(pParamVal, "%3d", &(laser.brightVal)) ){
            usbprintf("wrong parameter");
            return SCANNER_ERROR;
        }
        laser.status |= LASER_BRIGHT_UPDATE_F;
    };

	return SCANNER_OK;
};

/**
 * @brief
 *
 * @param cmdStr
 * @return ScannerStatus_t
 */
static ScannerStatus_t stepM_cmd_parser_cb(uint8_t *cmdStr){
    char *pParamStr;
    char *pParamVal;

    if (cmdStr == NULL)
        return SCANNER_ERROR;

    for (uint8_t i = 0; i < 2 ; i++){
        if ( strstr(cmdStr, stepM.power_func_names[i]) ){
            stepM.powerState = (PowerState_t) i;
            stepM.status |= STEPM_POWER_UPDATE_F;
        }
    };

    for (uint8_t i = 0; i < 2 ; i++){
        if ( strstr(cmdStr, stepM.rot_func_names[i]) ){
            stepM.rotDirection = (RotDirection_t) i;
            stepM.status |= STEPM_DIR_UPDATE_F;
        }
    };

    if ( pParamStr = strstr(cmdStr, stepM.steps_param_name) ){
        pParamVal = pParamStr + strlen(stepM.steps_param_name);
        if ( !sscanf(pParamVal, "%3d", &(stepM.stepsVal)) ){
            usbprintf("wrong parameter");
            return SCANNER_ERROR;
        }
        stepM.status |= STEPM_STEPS_UPDATE_F;
    };

    if ( pParamStr = strstr(cmdStr, stepM.speed_param_name) ){
        pParamVal = pParamStr + strlen(stepM.speed_param_name);
        if ( !sscanf(pParamVal, "%3d", &(stepM.speedVal)) ){
            usbprintf("wrong parameter");
            return SCANNER_ERROR;
        }
        stepM.status |= STEPM_SPEED_UPDATE_F;
    };

    return SCANNER_OK;
};


static ScannerStatus_t camera_cmd_parser_cb(uint8_t *cmdStr){
    char *pParamStr = NULL;
    char *pParamVal = NULL;

    if (cmdStr == NULL)
        return SCANNER_ERROR;

    for (uint8_t i = 0; i < 2 ; i++){
        if ( strstr(cmdStr, camera.power_func_names[i]) ){
            camera.powerState = (PowerState_t) i;
            camera.status |= CAMERA_POWER_UPDATE_F;
        }
    };

    if ( pParamStr = strstr(cmdStr, camera.picture_param_name) ){
        pParamVal = pParamStr + strlen(camera.picture_param_name);
        memset(camera.pictureName, 0, CAMERA_FILES_MAX_LENGTH);
        if ( !sscanf(pParamVal, "%7s .rgb", camera.pictureName) ){
            usbprintf("wrong parameter");
            return SCANNER_ERROR;
        }
        strcat(camera.pictureName, ".jpg");
        camera.status |= CAMERA_PICTURE_UPDATE_F;
    };

    if ( pParamStr = strstr(cmdStr, camera.config_param_name) ){
        pParamVal = pParamVal + strlen(camera.config_param_name);
        memset(camera.configFileName, 0, CAMERA_FILES_MAX_LENGTH);
        if ( !sscanf(pParamStr, "%12s", camera.configFileName) ){
            usbprintf("wrong parameter");
            return SCANNER_ERROR;
        }
        camera.status |= CAMERA_CONFIG_UPDATE_F;
    };

    if ( pParamStr = strstr(cmdStr, camera.reg_param_name) ){
        pParamVal = pParamVal + strlen(camera.reg_param_name);
        memset(camera.configFileName, 0, CAMERA_FILES_MAX_LENGTH);
        if ( sscanf(pParamStr, "0x%2x, 0x%2x", camera.regAddr, camera.regVal) != 2 ){
            usbprintf("wrong parameter");
            return SCANNER_ERROR;
        }
        camera.status |= CAMERA_REG_UPDATE_F;
    };

    return SCANNER_OK;
};

static ScannerStatus_t laser_cmd_cb(void){
	ScannerStatus_t res = SCANNER_OK;

    if ( laser.status & LASER_POWER_UPDATE_F){
        res |= ( 0 != laser.powerState ) ? laser_on() : laser_off();
        laser.status &= ~LASER_POWER_UPDATE_F;
    };

    if ( laser.status & LASER_BRIGHT_UPDATE_F){
        res |= laser_set(laser.brightVal);
        laser.status &= ~LASER_BRIGHT_UPDATE_F;
    };

    return res;
};

static ScannerStatus_t camera_cmd_cb(void){
    ScannerStatus_t res = SCANNER_OK;
    osStatus_t osRes;
    uint32_t flags;
    CameraQueueObj_t cameraMsg;

    if ( camera.status & CAMERA_POWER_UPDATE_F){
        res |= ( 0 != camera.powerState ) ? camera_on() : camera_off();
        camera.status &= ~CAMERA_POWER_UPDATE_F;
    };

    if ( camera.status & CAMERA_REG_UPDATE_F){
        res |= camera_config_reg(camera.regAddr, camera.regVal);;
        camera.status &= ~CAMERA_REG_UPDATE_F;
    };

    if ( camera.status & CAMERA_CONFIG_UPDATE_F){
        //res |= camera_config();
        camera.status &= ~CAMERA_CONFIG_UPDATE_F;
    };

    if ( camera.status & CAMERA_PICTURE_UPDATE_F){
        camera_get_frame(&new_frame_buffer);
        cameraMsg.frameBuf = (uint32_t *) &new_frame_buffer;
        strncpy(cameraMsg.fileName, camera.pictureName, CAMERA_FILES_MAX_LENGTH);
        osRes = osMessageQueuePut (cameraQueueHandler, &cameraMsg, 0, 0);
        osEventFlagsSet(cameraEvtId, CAMERA_EVT_FILE_CREATE);
		flags = osEventFlagsWait(cameraEvtId, CAMERA_EVT_FILE_CREATE_DONE, osFlagsWaitAny, osWaitForever);
		if ( !(flags & CAMERA_EVT_FILE_CREATE_DONE) )
			usbprintf("File creating error");
        // дождаться события окончания
        //res |= stepM_set_dir(stepM.brightVal);
        camera.status &= ~CAMERA_PICTURE_UPDATE_F;
    };

};

static ScannerStatus_t stepM_cmd_cb(void){
    ScannerStatus_t res = SCANNER_OK;

    if ( stepM.status & STEPM_POWER_UPDATE_F){
        res |= ( 0 != stepM.powerState ) ? stepM_on() : stepM_off();
        stepM.status &= ~STEPM_POWER_UPDATE_F;
    };

    if ( stepM.status & STEPM_DIR_UPDATE_F){
        res |= stepM_set_dir(stepM.rotDirection);
        stepM.status &= ~STEPM_DIR_UPDATE_F;
    };

    if ( stepM.status & STEPM_STEPS_UPDATE_F){
        res |= stepM_rotate(stepM.stepsVal);
        stepM.status &= ~STEPM_STEPS_UPDATE_F;
    };

    if ( stepM.status & STEPM_SPEED_UPDATE_F){
        res |= stepM_set_speed(stepM.speedVal);
        stepM.status &= ~STEPM_SPEED_UPDATE_F;
    };

    return res;
};
