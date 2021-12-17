/**
 * @author Katukiya G.
 *
 * @file modules.c
 *
 * @brief Structures of 3D scanner`s modules.
 */

#include "modules.h"
#include "string.h"
#include "stdlib.h"

#include "main.h"
#include "stm32h7xx_hal.h"

#include "storage_task.h"

#include "camera.h"

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


extern TIM_HandleTypeDef htim16;
#include "stm32h7xx_hal.h"

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
            HAL_TIM_PWM_Stop(&htim16, TIM_CHANNEL_1);
            __HAL_TIM_SET_COMPARE(&htim16, TIM_CHANNEL_1, laser.brightVal);
            HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
        }
        else
            HAL_TIM_PWM_Stop(&htim16, TIM_CHANNEL_1);



        /// @todo создать сообщение для обработки команды

        return HAL_OK;
    }

	return HAL_ERROR;
};

extern I2C_HandleTypeDef hi2c2;
#define OV7670_REG_NUM 			18

const uint8_t OV7670_reg[OV7670_REG_NUM][2] = {

		{ 0x12, 0x80 },	// Reset all registers
// Image format
		{ 0x12, 0x0c },																						// 0x14 = QVGA size(RGB); 0x8 = QCIF(YUV), 0xc = QCIF(RGB)
		{ 0x40, 0xd0 },		// RGB 565 mode and default Output range: [00] to [FF]
		{ 0x0c, 0x08 }, 	// No tri-state at power-down period
		{ 0x11, 0x40 }, 	// Use external clock directly (no clock pre-scale available)

		{ 0xb0, 0x84 },		// Color mode                                                                   (Not documented??)

		// Hardware window
		{ 0x11, 0x01 },		//PCLK settings, 15fps                                                        // при чтении этого регистра, почему то выводит 0x01, вместо 0x40
		{ 0x32, 0x80 },		//HREF
		{ 0x17, 0x17 },		//HSTART
		{ 0x18, 0x05 },		//HSTOP
		{ 0x03, 0x0a },		//VREF
		{ 0x19, 0x02 },		//VSTART
		{ 0x1a, 0x7a },		//VSTOP

		// Scalling numbers
		{ 0x70, 0x3a },		//X_SCALING
		{ 0x71, 0x35 },		//Y_SCALING
		{ 0x72, 0x11 },		//DCW_SCALING
		{ 0x73, 0xf0 },		//PCLK_DIV_SCALING
		{ 0xa2, 0x02 }		//PCLK_DELAY_SCALING
};

uint8_t SCCB_write_reg(uint8_t reg_addr, uint8_t data) {
	uint32_t timeout = 0xFFFFFF;

	uint8_t tempData[2];
	tempData[0]=reg_addr;
	tempData[1]=data;
	HAL_I2C_Master_Transmit(&hi2c2, 0x42, tempData, 2, timeout);

	return 0;
}

uint8_t OV7670_init(void) {
	uint8_t data, i = 0;
	uint8_t err;
	uint32_t timeout = 0xFFFFFF;

	// Configure camera registers
	for (i = 0; i < OV7670_REG_NUM; i++) {
		data = OV7670_reg[i][1];
		if (i == 0)
			HAL_Delay(100);
		err = SCCB_write_reg(OV7670_reg[i][0], data);

		if (err == 1) {
			break;
		}

		HAL_Delay(100);
	}
	return err;
}

#define IMG_ROWS   					144
#define IMG_COLUMNS   				174
extern DCMI_HandleTypeDef hdcmi;
extern volatile uint16_t frame_buffer[IMG_ROWS * IMG_COLUMNS];
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

HAL_StatusTypeDef camera_cmd_define_cb(uint8_t *cmdStr){
    uint8_t i = 0;
    char *pParamStr = NULL;
    char *pParamVal = NULL;
    uint8_t paramValLen = 0;
    char paramValStr[4] = {0};
    
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
            // Подача питания на камеру и переход в режим готовности
            HAL_GPIO_WritePin(CAM_En_GPIO_Port, CAM_En_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, CAM_Pwdn_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, CAM_Reset_Pin, GPIO_PIN_SET);
            // Включение тактирования
            HAL_RCC_MCOConfig(RCC_MCO2, RCC_MCO2SOURCE_HSE, RCC_MCODIV_1);

            // Конфигурирование регистра
            /**
             * @todo определять, если регистр уже записан, чтобы не было повтороной записи
             */

            SCCB_write_reg(camera.regAddr, camera.regVal);
            HAL_Delay(100);
            
            HAL_StatusTypeDef result;
            result = HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t) frame_buffer, IMG_ROWS * IMG_COLUMNS / 2);
	        result = HAL_DCMI_Stop(&hdcmi);
            //SavePicture(camera.pictureName, frame_buffer, IMG_ROWS * IMG_COLUMNS *2 );

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
            // Подача питания на камеру и переход в режим готовности
            HAL_GPIO_WritePin(CAM_En_GPIO_Port, CAM_En_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOA, CAM_Pwdn_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOA, CAM_Reset_Pin, GPIO_PIN_RESET);
        }

        return HAL_OK;
    }
    return HAL_ERROR;
}


HAL_StatusTypeDef stepM_cmd_define_cb(uint8_t *cmdStr){
    return HAL_OK;
};



