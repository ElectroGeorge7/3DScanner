/**
 * @author Katukiya G.
 *
 * @file modules.c
 *
 * @brief Structures of 3D scanner`s modules.
 */

#include "../Modules/modules.h"

#include "string.h"
#include "stdlib.h"

#include "main.h"
#include "stm32h7xx_hal.h"
#include "usb_device.h"

#include "../Modules/Laser/laser.h"

#include "../Modules/Cam_OV7670/camera.h"
#include "../Modules/Cam_OV7670/ov7670reg.h"
#include "../Modules/Cam_OV7670/camera_hw_i2c.h"

#include "../Tasks/storage_task.h"


struct sLaser_t laser = {
    "laser",
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


struct sStepM_t stepM = {
    "stepM",
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

struct sCamera_t camera = {
    "camera",

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



/// @note the order of modules should be the same in all arrays, defined below
uint32_t *psModules[MODULES_NUMBER] = {
    (uint32_t *) &laser,
    (uint32_t *) &stepM,
	(uint32_t *) &camera
};

module_cmd_define_cb_t module_cmd_define_cbs[MODULES_NUMBER] = {
    laser_cmd_define_cb,
    stepM_cmd_define_cb,
    camera_cmd_define_cb,
};


/**
 * @brief 
 * 
 * @param cmdStr 
 * @return HAL_StatusTypeDef 
 */
HAL_StatusTypeDef laser_cmd_define_cb(uint8_t *cmdStr){
    uint8_t i = 0;
    char *pParamStr = NULL;
    char *pParamVal = NULL;
    uint8_t paramValLen = 0;
    char paramValStr[4] = {0}; 

    if (cmdStr != NULL){
        
        for ( i = 0; i < 2 ; i++){
            if ( strstr(cmdStr, laser.power_func_names[i]) )
                laser.powerState = (PowerState_t) i;
        };

        if ( pParamStr = strstr(cmdStr, laser.bright_param_name) ){
            pParamVal = pParamStr + strlen(laser.bright_param_name);
            paramValLen = 0;
            while ( ( *(pParamVal+paramValLen) != 0x0D) && ( *(pParamVal+paramValLen) != ' ') && (paramValLen < 4 ) ){
                paramValLen++;
            };
            memmove(paramValStr, pParamVal, paramValLen);
            if ( paramValStr[0] == '0' )
                laser.brightVal = 0;
            else  {
            	paramValStr[3] = '\0';
                if ( atoi(paramValStr) )
                    laser.brightVal = atoi(paramValStr);
            }
        }


        if ( laser.powerState == EN ){
        	laser_on();
        	// после проверки убрать включение и отключение таймера из laser_set
        	laser_set(laser.brightVal);
        }
        else{
        	laser_off();
        }
        /// @todo создать сообщение для обработки команды

        return HAL_OK;
    }

	return HAL_ERROR;
};


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
HAL_StatusTypeDef configCam(char *configFileName, char* readBuf, uint32_t bufSize){
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

HAL_StatusTypeDef camera_cmd_define_cb(uint8_t *cmdStr){
    uint8_t i = 0;
    char *pParamStr = NULL;
    char *pParamVal = NULL;
    uint8_t paramValLen = 0;
    char paramValStr[4] = {0};
    
    uint8_t resp=0;

    if (cmdStr != NULL){
        
        for ( i = 0; i < 2 ; i++){
            if ( strstr(cmdStr, camera.power_func_names[i]) )
                camera.powerState = (PowerState_t) i;
        };


        if ( pParamStr = strstr(cmdStr, camera.picture_param_name) ){
            pParamVal = pParamStr + strlen(camera.picture_param_name);
            paramValLen = 0;
            while ( ( *(pParamVal+paramValLen) != 0x0D)             && 
                        ( *(pParamVal+paramValLen) != ' ')          && 
                              ( *(pParamVal+paramValLen) != '\0')   &&
                                (paramValLen < CAMERA_FILES_MAX_LENGTH-1 )  ){
                paramValLen++;
            };

            if ( paramValLen != 0 ){
            	memset(camera.pictureName, 0, CAMERA_FILES_MAX_LENGTH);
                memmove(camera.pictureName, pParamVal, paramValLen);
            }
            else 
            	;
                /// @todo создать сообщение для обработки ошибки
        }

        if ( pParamStr = strstr(cmdStr, camera.config_param_name) ){
            pParamVal = pParamStr + strlen(camera.config_param_name);
            paramValLen = 0;
            while ( ( *(pParamVal+paramValLen) != 0x0D)             && 
                        ( *(pParamVal+paramValLen) != ' ')          && 
                              ( *(pParamVal+paramValLen) != '\0')   &&
                                (paramValLen < CAMERA_FILES_MAX_LENGTH-1 )  ){
                paramValLen++;
            };

            if ( paramValLen != 0 ){
            	memset(camera.configFileName, 0, CAMERA_FILES_MAX_LENGTH);
                memmove(camera.configFileName, pParamVal, paramValLen);
            }
            else 
            	;
                /// @todo создать сообщение для обработки ошибки
        }

        if ( pParamStr = strstr(cmdStr, camera.reg_param_name) ){
            pParamVal = pParamStr + strlen(camera.reg_param_name);
            paramValLen = 0;
            while ( ( *(pParamVal+paramValLen) != 0x0D)         &&
                        ( *(pParamVal+paramValLen) != ' ')      &&
                            ( *(pParamVal+paramValLen) != ',')  &&
                                (paramValLen < 4 ) ){
                paramValLen++;
            };
            memmove(paramValStr, pParamVal, paramValLen);
            if ( paramValStr[0] == '0' )
                camera.regAddr = 0;
            else  {
            	paramValStr[3] = '\0';
                if ( atoi(paramValStr) )
                    camera.regAddr = atoi(paramValStr);
            }

            pParamVal += paramValLen+1;
            paramValLen = 0;
            memset(paramValStr, 0, 4);
            while ( ( *(pParamVal+paramValLen) != 0x0D)         &&
                        ( *(pParamVal+paramValLen) != ' ')      &&
                                (paramValLen < 4 ) ){
                paramValLen++;
            };  
            memmove(paramValStr, pParamVal, paramValLen);
            if ( paramValStr[0] == '0' )
                camera.regVal = 0;
            else  {
            	paramValStr[3] = '\0';
                if ( atoi(paramValStr) )
                    camera.regVal = atoi(paramValStr);
            }          

        }


        if ( camera.powerState == EN ){
        	// перенести в camera

            // Конфигурирование регистра
            /**
             * @todo определять, если регистр уже записан, чтобы не было повтороной записи
             */
        	camera_on();


            /**
             * @todo разобраться, почему при стирании на изображении появляются артефакты
             */
            //memset(new_frame_buffer.pFrameBuf1, 0, FRAME_BUF_PART_SIZE*4);
            //memset(new_frame_buffer.pFrameBuf2, 0, FRAME_BUF_PART_SIZE*4);
            //memset(new_frame_buffer.pFrameBuf3, 0, FRAME_BUF_PART_SIZE*4);
            //memset(new_frame_buffer.pFrameBuf4, 0, FRAME_BUF_PART_SIZE*4);

            if ( camera.regAddr != 0 ){
            	//resp = SCCB_write_reg(camera.regAddr, camera.regVal);
				//HAL_Delay(100);
            	camera_reg_config(camera.regAddr, camera.regVal);
            }

            MX_USB_DEVICE_Stop();

            if (camera.pictureName[0] != 0 && camera.regAddr == 0 )
            	//ConfigCamera8();
            	camera_default_config();

            HAL_Delay(100);
            
            camera_get_frame(&camera, &new_frame_buffer);

            MX_USB_DEVICE_Start();

            //OV7670_init();
            // Нахождение файла конфигурации
            /**
             * @todo сейчас эта функция относится к ControlTask, поэтому тратит её стек,
             * 		 в дальнейшем нужно будет создавать сообщения, чтоб чтение выполняло
             * 		 именно storageTask
             */
            //configCam(camera.configFileName, readBuf, sizeof(readBuf));
            // Создание файла снимка 

            
        } else {
        	camera_off();
        }

        return HAL_OK;
    }
    return HAL_ERROR;
}



HAL_StatusTypeDef stepM_cmd_define_cb(uint8_t *cmdStr){
    return HAL_OK;
};



