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
#include "usb_device.h"

#include "storage_task.h"

#include "camera.h"
#include "ov7670reg.h"

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

	uint8_t response=0;
uint8_t SCCB_write_reg(uint8_t reg_addr, uint8_t data) {
	uint32_t timeout = 0xFFFFFF;

	uint8_t tempData[2];
	tempData[0]=reg_addr;
	tempData[1]=data;
	HAL_I2C_Master_Transmit(&hi2c2, 0x42, tempData, 2, timeout);
	HAL_Delay(2);
	HAL_I2C_Master_Transmit(&hi2c2, 0x42, tempData, 1, timeout);
	HAL_I2C_Master_Receive(&hi2c2, 0x43, &response, 1, timeout);
	return response;
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


extern DCMI_HandleTypeDef hdcmi;
extern volatile uint32_t frame_buffer[2];
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

void ConfigCamera(void){
    SCCB_write_reg(0x12, 0x80); // RESET CAMERA
    HAL_Delay(200);

    SCCB_write_reg(REG_RGB444, 0x00);              // Disable RGB444
    //SCCB_write_reg(REG_COM10, 0x02);               // 0x02   VSYNC negative (http://nasulica.homelinux.org/?p=959)
    SCCB_write_reg(REG_MVFP, 0x27);                // mirror image 
 
    SCCB_write_reg(REG_CLKRC, 0x80);               // prescaler x1     
    SCCB_write_reg(DBLV, 0x0a);                    // bypass PLL 
        
    SCCB_write_reg(REG_COM11, 0x0A) ;
    SCCB_write_reg(REG_TSLB, 0x04);                // 0D = UYVY  04 = YUYV     
    SCCB_write_reg(REG_COM13, 0x88);               // connect to REG_TSLB

    SCCB_write_reg(REG_COM7, 0x04);           // RGB + color bar disable 
    SCCB_write_reg(REG_RGB444, 0x00);         // Disable RGB444
    SCCB_write_reg(REG_COM15, 0x10);          // Set rgb565 with Full range    0xD0
    SCCB_write_reg(REG_COM3, 0x04);
    SCCB_write_reg(REG_CLKRC, 0x80);          // prescaler x1 

    SCCB_write_reg(0x70, 0x3A);                   // Scaling Xsc
    SCCB_write_reg(0x71, 0x35);                   // Scaling Ysc
    SCCB_write_reg(0xA2, 0x02);                   // pixel clock delay

    SCCB_write_reg(REG_COM14, 0x1a);          // divide by 4
    SCCB_write_reg(0x72, 0x22);               // downsample by 4
    SCCB_write_reg(0x73, 0xf2);               // divide by 4
    SCCB_write_reg(REG_HREF, 0xa4);
    SCCB_write_reg(REG_HSTART, 0x16);
    SCCB_write_reg(REG_HSTOP, 0x04);
    SCCB_write_reg(REG_VREF, 0x0a);   
    SCCB_write_reg(REG_VSTART, 0x02);
    SCCB_write_reg(REG_VSTOP, 0x7a);        
            
    SCCB_write_reg(0x7a, 0x20);
    SCCB_write_reg(0x7b, 0x1c);
    SCCB_write_reg(0x7c, 0x28);
    SCCB_write_reg(0x7d, 0x3c);
    SCCB_write_reg(0x7e, 0x5a);
    SCCB_write_reg(0x7f, 0x68);
    SCCB_write_reg(0x80, 0x76);
    SCCB_write_reg(0x81, 0x80);
    SCCB_write_reg(0x82, 0x88);
    SCCB_write_reg(0x83, 0x8f);
    SCCB_write_reg(0x84, 0x96);
    SCCB_write_reg(0x85, 0xa3);
    SCCB_write_reg(0x86, 0xaf);
    SCCB_write_reg(0x87, 0xc4);
    SCCB_write_reg(0x88, 0xd7);
    SCCB_write_reg(0x89, 0xe8);
    
    SCCB_write_reg(0x13, 0xe0);
    SCCB_write_reg(0x00, 0x00);
    SCCB_write_reg(0x10, 0x00);
    SCCB_write_reg(0x0d, 0x40);
    SCCB_write_reg(0x14, 0x18);
    SCCB_write_reg(0xa5, 0x05);
    SCCB_write_reg(0xab, 0x07);
    SCCB_write_reg(0x24, 0x95);
    SCCB_write_reg(0x25, 0x33);
    SCCB_write_reg(0x26, 0xe3);
    SCCB_write_reg(0x9f, 0x78);
    SCCB_write_reg(0xa0, 0x68);
    SCCB_write_reg(0xa1, 0x03);
    SCCB_write_reg(0xa6, 0xd8);
    SCCB_write_reg(0xa7, 0xd8);
    SCCB_write_reg(0xa8, 0xf0);
    SCCB_write_reg(0xa9, 0x90);
    SCCB_write_reg(0xaa, 0x94);
    SCCB_write_reg(0x13, 0xe5);
    
    SCCB_write_reg(0x0e, 0x61);
    SCCB_write_reg(0x0f, 0x4b);
    SCCB_write_reg(0x16, 0x02);

    SCCB_write_reg(0x21, 0x02);
    SCCB_write_reg(0x22, 0x91);
    SCCB_write_reg(0x29, 0x07);
    SCCB_write_reg(0x33, 0x0b);
    SCCB_write_reg(0x35, 0x0b);
    SCCB_write_reg(0x37, 0x1d);
    SCCB_write_reg(0x38, 0x71);
    SCCB_write_reg(0x39, 0x2a);
    SCCB_write_reg(0x3c, 0x78);
    SCCB_write_reg(0x4d, 0x40);
    SCCB_write_reg(0x4e, 0x20);
    SCCB_write_reg(0x69, 0x00);

    SCCB_write_reg(0x74, 0x10);
    SCCB_write_reg(0x8d, 0x4f);
    SCCB_write_reg(0x8e, 0x00);
    SCCB_write_reg(0x8f, 0x00);
    SCCB_write_reg(0x90, 0x00);
    SCCB_write_reg(0x91, 0x00);
    SCCB_write_reg(0x92, 0x00);

    SCCB_write_reg(0x96, 0x00);
    SCCB_write_reg(0x9a, 0x80);
    SCCB_write_reg(0xb0, 0x84);
    SCCB_write_reg(0xb1, 0x0c);
    SCCB_write_reg(0xb2, 0x0e);
    SCCB_write_reg(0xb3, 0x82);
    SCCB_write_reg(0xb8, 0x0a);
    
    SCCB_write_reg(0x43, 0x0a);
    SCCB_write_reg(0x44, 0xf0);
    SCCB_write_reg(0x45, 0x34);
    SCCB_write_reg(0x46, 0x58);
    SCCB_write_reg(0x47, 0x28);
    SCCB_write_reg(0x48, 0x3a);
    SCCB_write_reg(0x59, 0x88);
    SCCB_write_reg(0x5a, 0x88);
    SCCB_write_reg(0x5b, 0x44);
    SCCB_write_reg(0x5c, 0x67);
    SCCB_write_reg(0x5d, 0x49);
    SCCB_write_reg(0x5e, 0x0e);
    SCCB_write_reg(0x64, 0x04);
    SCCB_write_reg(0x65, 0x20);
    SCCB_write_reg(0x66, 0x05);
    SCCB_write_reg(0x94, 0x04);
    SCCB_write_reg(0x95, 0x08);

    SCCB_write_reg(0x6c, 0x0a);
    SCCB_write_reg(0x6d, 0x55);
    SCCB_write_reg(0x6e, 0x11);
    SCCB_write_reg(0x6f, 0x9f);
    SCCB_write_reg(0x6a, 0x40);
    SCCB_write_reg(0x01, 0x40);
    SCCB_write_reg(0x02, 0x40);
    SCCB_write_reg(0x13, 0xe7);
    //SCCB_write_reg(0x15, 0x02);

    SCCB_write_reg(0x4f, 0x80);
    SCCB_write_reg(0x50, 0x80);
    SCCB_write_reg(0x51, 0x00);
    SCCB_write_reg(0x52, 0x22);
    SCCB_write_reg(0x53, 0x5e);
    SCCB_write_reg(0x54, 0x80);
    SCCB_write_reg(0x58, 0x9e);
    
    SCCB_write_reg(0x41, 0x08);
    SCCB_write_reg(0x3f, 0x00);
    SCCB_write_reg(0x75, 0x05);
    SCCB_write_reg(0x76, 0xe1);
    SCCB_write_reg(0x4c, 0x00);
    SCCB_write_reg(0x77, 0x01);
    SCCB_write_reg(0x3d, 0xc1);
    SCCB_write_reg(0x4b, 0x09);
    SCCB_write_reg(0xc9, 0x60);
    SCCB_write_reg(0x41, 0x38);
    SCCB_write_reg(0x56, 0x40);
    
    SCCB_write_reg(0x34, 0x11);
    SCCB_write_reg(0x3b, 0x02);
    SCCB_write_reg(0xa4, 0x88);
    SCCB_write_reg(0x96, 0x00);
    SCCB_write_reg(0x97, 0x30);
    SCCB_write_reg(0x98, 0x20);
    SCCB_write_reg(0x99, 0x30);
    SCCB_write_reg(0x9a, 0x84);
    SCCB_write_reg(0x9b, 0x29);
    SCCB_write_reg(0x9c, 0x03);
    SCCB_write_reg(0x9d, 0x4c);
    SCCB_write_reg(0x9e, 0x3f);
    SCCB_write_reg(0x78, 0x04);
    
    SCCB_write_reg(0x79, 0x01);
    SCCB_write_reg(0xc8, 0xf0);
    SCCB_write_reg(0x79, 0x0f);
    SCCB_write_reg(0xc8, 0x00);
    SCCB_write_reg(0x79, 0x10);
    SCCB_write_reg(0xc8, 0x7e);
    SCCB_write_reg(0x79, 0x0a);
    SCCB_write_reg(0xc8, 0x80);
    SCCB_write_reg(0x79, 0x0b);
    SCCB_write_reg(0xc8, 0x01);
    SCCB_write_reg(0x79, 0x0c);
    SCCB_write_reg(0xc8, 0x0f);
    SCCB_write_reg(0x79, 0x0d);
    SCCB_write_reg(0xc8, 0x20);
    SCCB_write_reg(0x79, 0x09);
    SCCB_write_reg(0xc8, 0x80);
    SCCB_write_reg(0x79, 0x02);
    SCCB_write_reg(0xc8, 0xc0);
    SCCB_write_reg(0x79, 0x03);
    SCCB_write_reg(0xc8, 0x40);
    SCCB_write_reg(0x79, 0x05);
    SCCB_write_reg(0xc8, 0x30);
    SCCB_write_reg(0x79, 0x26);
    SCCB_write_reg(0x09, 0x03);
    SCCB_write_reg(0x3b, 0x42);
    
    SCCB_write_reg(0xff, 0xff);   /* END MARKER */ 
}


void ConfigCamera2(void){
	uint8_t resp=0;
        // QQVGA RGB565
	resp = SCCB_write_reg(REG_CLKRC,0x80);  // prescaler x1
	HAL_Delay(100);
    resp = SCCB_write_reg(REG_COM11,0x0A) ; // filtering and exposure timing
    resp = SCCB_write_reg(REG_TSLB,0x04);   // 0D = UYVY  04 = YUYV  
    resp = SCCB_write_reg(REG_COM7,0x04) ;  // RGB selection
    resp = SCCB_write_reg(REG_RGB444, 0x00);    // turn off rgb44 ) (на всякий)
    resp = SCCB_write_reg(REG_COM15, 0xd0);     // RGB 565 mode and default Output range: [00] to [FF]
    resp = SCCB_write_reg(REG_HSTART,0x16) ;
    resp = SCCB_write_reg(REG_HSTOP,0x04) ;
    resp = SCCB_write_reg(REG_HREF,0x24) ;
    resp = SCCB_write_reg(REG_VSTART,0x02) ;
    resp = SCCB_write_reg(REG_VSTOP,0x7a) ;
    resp = SCCB_write_reg(REG_VREF,0x0a) ;
    //resp = SCCB_write_reg(REG_COM10,0x02) ;
    resp = SCCB_write_reg(REG_COM3, 0x04);
    resp = SCCB_write_reg(REG_COM14, 0x1a);
    resp = SCCB_write_reg(REG_MVFP,0x07) ;
    resp = SCCB_write_reg(0x72, 0x22);
    resp = SCCB_write_reg(0x73, 0xf2);

    // COLOR SETTING
    resp = SCCB_write_reg(0x4f,0x80);
    resp = SCCB_write_reg(0x50,0x80);
    resp = SCCB_write_reg(0x51,0x00);
    resp = SCCB_write_reg(0x52,0x22);
    resp = SCCB_write_reg(0x53,0x5e);
    resp = SCCB_write_reg(0x54,0x80);
    resp = SCCB_write_reg(0x56,0x40);
    resp = SCCB_write_reg(0x58,0x9e);
    resp = SCCB_write_reg(0x59,0x88);
    resp = SCCB_write_reg(0x5a,0x88);
    resp = SCCB_write_reg(0x5b,0x44);
    resp = SCCB_write_reg(0x5c,0x67);
    resp = SCCB_write_reg(0x5d,0x49);
    resp = SCCB_write_reg(0x5e,0x0e);
    resp = SCCB_write_reg(0x69,0x00);
    resp = SCCB_write_reg(0x6a,0x40);
    resp = SCCB_write_reg(0x6b,0x0a);
    resp = SCCB_write_reg(0x6c,0x0a);
    resp = SCCB_write_reg(0x6d,0x55);
    resp = SCCB_write_reg(0x6e,0x11);
    resp = SCCB_write_reg(0x6f,0x9f);

    resp = SCCB_write_reg(0xb0,0x84);
}

void ConfigCamera3(void){
	uint8_t resp=0;
    resp = SCCB_write_reg(0x12, 0x80);  // Reset all registers
    HAL_Delay(100);
    resp = SCCB_write_reg(0x12, 0x80);  // Reset all registers

    resp = SCCB_write_reg(DBLV, 0x0a);       // bypass PLL 
//    resp = SCCB_write_reg(REG_CLKRC,0x80);  // prescaler x1 (not documented)
//   resp = SCCB_write_reg(REG_COM11,0x0A) ; // use prescaller of input freq: /12 // попробую, скорей всего нужно будет убрать
    resp = SCCB_write_reg(0x11, 0x40);  // Use external clock directly (no clock pre-scale available)
//    resp = SCCB_write_reg(0x11, 0x01); // prescaler /1

//   resp = SCCB_write_reg(REG_COM10, 0x02); // 0x02   VSYNC negative (попробовать)
    resp = SCCB_write_reg(REG_COM3, 0x04);
    resp = SCCB_write_reg(REG_COM7,0x0c) ;  // RGB selection + (qcif? - 0x0e) +color bar
    resp = SCCB_write_reg(REG_RGB444, 0x00);    // turn off rgb44 ) (на всякий)
    resp = SCCB_write_reg(REG_COM15, 0xd0);  // RGB 565 mode and default Output range: [00] to [FF]


 /*   resp = SCCB_write_reg(REG_COM10,0x02) ;  // 0x02   VSYNC negative ()


// Image format
    resp = SCCB_write_reg(0x12, 0x0e);  // 0x14 = QVGA size(RGB); 0x8 = QCIF(YUV), 0xc = QCIF(RGB) + color bar
    resp = SCCB_write_reg(0x40, 0xd0);
    resp = SCCB_write_reg(0x0c, 0x08);  // No tri-state at power-down period
    resp = SCCB_write_reg(0x11, 0x40);

    resp = SCCB_write_reg(0xb0, 0x84);  // Color mode (Not documented??)
// Hardware window
    resp = SCCB_write_reg(0x11, 0x01);  //PCLK settings, 15fps
    resp = SCCB_write_reg(0x32, 0x80);  //HREF
    resp = SCCB_write_reg(0x17, 0x17);  //HSTART
    resp = SCCB_write_reg(0x18, 0x05);  //HSTOP
    resp = SCCB_write_reg(0x03, 0x0a);  //VREF
    resp = SCCB_write_reg(0x19, 0x02);  //VSTART
    resp = SCCB_write_reg(0x1a, 0x7a);  //VSTOP
// Scalling numbers
    resp = SCCB_write_reg(0x70, 0x3a);  //X_SCALING
    resp = SCCB_write_reg(0x71, 0x35);  //Y_SCALING
    resp = SCCB_write_reg(0x72, 0x11);  //DCW_SCALING
    resp = SCCB_write_reg(0x73, 0xf0);  //PCLK_DIV_SCALING
    resp = SCCB_write_reg(0xa2, 0x02);  //PCLK_DELAY_SCALING
*/
}

void ConfigCamera4(void){
    uint8_t resp=0;
    resp = SCCB_write_reg(0x12, 0x80); 
    HAL_Delay(100);
    resp = SCCB_write_reg(0x3a, 0x04);
    resp = SCCB_write_reg(0x12, 0);
    //resp = SCCB_write_reg(0x15, 32);
    resp = SCCB_write_reg(0x0c, 4);
    resp = SCCB_write_reg(0x3e, 0x19);
    resp = SCCB_write_reg(0x72, 0x11);
    resp = SCCB_write_reg(0x73, 0xf1);
    resp = SCCB_write_reg(0x17, 0x16);
    resp = SCCB_write_reg(0x18, 0x04);
    resp = SCCB_write_reg(0x32, 0xa4);
    resp = SCCB_write_reg(0x19, 0x02);
    resp = SCCB_write_reg(0x1a, 0x7a);
    resp = SCCB_write_reg(0x03, 0x0a);
    resp = SCCB_write_reg(0x12, 0);
    resp = SCCB_write_reg(0x11, 0x12);
}

void ConfigCamera5(void){
	uint8_t resp=0;
    resp = SCCB_write_reg(0x12, 0x80);  // Reset all registers
    HAL_Delay(100);
    resp = SCCB_write_reg(0x12, 0x14);  // QVGA, RGB
    resp = SCCB_write_reg(0x8C, 0x00);   // Disable RGB444
    resp = SCCB_write_reg(0x40, 0x10 + 0xc0);   // RGB565, 00 - FF

    resp = SCCB_write_reg(0x3A, 0x04 + 8);      // UYVY (why?)
    resp = SCCB_write_reg(0x3D, 0x80 + 0x00);   // gamma enable, UV auto adjust, UYVY
    resp = SCCB_write_reg(0xB0, 0x84);          // Color mode (Not documented??)

    resp = SCCB_write_reg(REG_HSTART,0x16) ;
    resp = SCCB_write_reg(REG_HSTOP,0x04) ;
    resp = SCCB_write_reg(REG_HREF,0x24) ;
    resp = SCCB_write_reg(REG_VSTART,0x02) ;
    resp = SCCB_write_reg(REG_VSTOP,0x7a) ;
    resp = SCCB_write_reg(REG_VREF,0x0a) ;
    resp = SCCB_write_reg(0x72, 0x22);
    resp = SCCB_write_reg(0x73, 0xf2);

}

void ConfigCamera6(void){
	uint8_t resp=0;
    resp = SCCB_write_reg(0x12, 0x80);  // Reset all registers
    HAL_Delay(100);
    resp = SCCB_write_reg(REG_CLKRC,0x80);
    resp = SCCB_write_reg(REG_COM11,0x0A);
    resp = SCCB_write_reg(REG_TSLB,0x04);
    resp = SCCB_write_reg(REG_COM7,0x04);
    resp = SCCB_write_reg(REG_RGB444, 0x00);
    resp = SCCB_write_reg(REG_COM15, 0xd0);
    resp = SCCB_write_reg(REG_HSTART,0x16);
    resp = SCCB_write_reg(REG_HSTOP,0x04);
    resp = SCCB_write_reg(REG_HREF,0x80);
    resp = SCCB_write_reg(REG_VSTART,0x02);
    resp = SCCB_write_reg(REG_VSTOP,0x7a);
    resp = SCCB_write_reg(REG_VREF,0x0a);
    //resp = SCCB_write_reg(REG_COM10,0x02);
    resp = SCCB_write_reg(REG_COM3, 0x04);
    resp = SCCB_write_reg(REG_COM14, 0x19);
    // resp = SCCB_write_reg(REG_MVFP,0x07 | (flipv ? 0x10:0) | (fliph ? 0x20:0)) ;
    resp = SCCB_write_reg(0x72, 0x11);
    resp = SCCB_write_reg(0x73, 0xf1);
    // COLOR SETTING
    resp = SCCB_write_reg(0x4f,0x80);
    resp = SCCB_write_reg(0x50,0x80);
    resp = SCCB_write_reg(0x51,0x00);
    resp = SCCB_write_reg(0x52,0x22);
    resp = SCCB_write_reg(0x53,0x5e);
    resp = SCCB_write_reg(0x54,0x80);
    resp = SCCB_write_reg(0x56,0x40);
    resp = SCCB_write_reg(0x58,0x9e);
    resp = SCCB_write_reg(0x59,0x88);
    resp = SCCB_write_reg(0x5a,0x88);
    resp = SCCB_write_reg(0x5b,0x44);
    resp = SCCB_write_reg(0x5c,0x67);
    resp = SCCB_write_reg(0x5d,0x49);
    resp = SCCB_write_reg(0x5e,0x0e);
    resp = SCCB_write_reg(0x69,0x00);
    resp = SCCB_write_reg(0x6a,0x40);
    resp = SCCB_write_reg(0x6b,0x0a);
    resp = SCCB_write_reg(0x6c,0x0a);
    resp = SCCB_write_reg(0x6d,0x55);
    resp = SCCB_write_reg(0x6e,0x11);
    resp = SCCB_write_reg(0x6f,0x9f);

    resp = SCCB_write_reg(0xb0,0x84);

}

void ConfigCamera7(void){
	uint8_t resp=0;
    resp = SCCB_write_reg(0x12, 0x80);  // Reset all registers
    HAL_Delay(100);
         resp = SCCB_write_reg(REG_CLKRC, 0x01);     
         resp = SCCB_write_reg(REG_TSLB,  0x04);    
         resp = SCCB_write_reg(REG_COM7, 0x01);        
         resp = SCCB_write_reg(DBLV, 0x4a); 
         resp = SCCB_write_reg(REG_COM3, 0);        
         resp = SCCB_write_reg(REG_COM14, 0);
        
         resp = SCCB_write_reg(REG_HSTART, 0x13);   
         resp = SCCB_write_reg(REG_HSTOP, 0x01);
         resp = SCCB_write_reg(REG_HREF, 0xb6);     
         resp = SCCB_write_reg(REG_VSTART, 0x02);
         resp = SCCB_write_reg(REG_VSTOP, 0x7a);    
         resp = SCCB_write_reg(REG_VREF, 0x0a);
         resp = SCCB_write_reg(0x72, 0x11);         
         resp = SCCB_write_reg(0x73, 0xf0);  
        
        /* Gamma curve values */
         resp = SCCB_write_reg(0x7a, 0x20);         
         resp = SCCB_write_reg(0x7b, 0x10);
         resp = SCCB_write_reg(0x7c, 0x1e);         
         resp = SCCB_write_reg(0x7d, 0x35);
         resp = SCCB_write_reg(0x7e, 0x5a);         
         resp = SCCB_write_reg(0x7f, 0x69);
         resp = SCCB_write_reg(0x80, 0x76);         
         resp = SCCB_write_reg(0x81, 0x80);
         resp = SCCB_write_reg(0x82, 0x88);         
         resp = SCCB_write_reg(0x83, 0x8f);
         resp = SCCB_write_reg(0x84, 0x96);         
         resp = SCCB_write_reg(0x85, 0xa3);
         resp = SCCB_write_reg(0x86, 0xaf);         
         resp = SCCB_write_reg(0x87, 0xc4);
         resp = SCCB_write_reg(0x88, 0xd7);         
         resp = SCCB_write_reg(0x89, 0xe8);
        
        /* AGC and AEC parameters.  Note we start by disabling those features,
        then turn them only after tweaking the values. */
         resp = SCCB_write_reg(0x13, COM8_FASTAEC | COM8_AECSTEP | COM8_BFILT);
         resp = SCCB_write_reg(0x00, 0);        
         resp = SCCB_write_reg(0x10, 0);
         resp = SCCB_write_reg(0x0d, 0x40); 
         resp = SCCB_write_reg(0x14, 0x18); 
         resp = SCCB_write_reg(0xa5, 0x05);  
         resp = SCCB_write_reg(0xab, 0x07);
         resp = SCCB_write_reg(0x24, 0x95);      
         resp = SCCB_write_reg(0x25, 0x33);
         resp = SCCB_write_reg(0x26, 0xe3);      
         resp = SCCB_write_reg(0x9f, 0x78);
         resp = SCCB_write_reg(0xa0, 0x68);   
         resp = SCCB_write_reg(0xa1, 0x03); 
         resp = SCCB_write_reg(0xa6, 0xd8);   
         resp = SCCB_write_reg(0xa7, 0xd8);
         resp = SCCB_write_reg(0xa8, 0xf0);   
         resp = SCCB_write_reg(0xa9, 0x90);
         resp = SCCB_write_reg(0xaa, 0x94);
         resp = SCCB_write_reg(0x13, COM8_FASTAEC|COM8_AECSTEP|COM8_BFILT|COM8_AGC|COM8_AEC);  
        
        /* Almost all of these are magic "reserved" values.  */    
         resp = SCCB_write_reg(0x0e, 0x61);     
         resp = SCCB_write_reg(0x0f, 0x4b);
         resp = SCCB_write_reg(0x16, 0x02);        
         resp = SCCB_write_reg(0x1e, 0x27);
         resp = SCCB_write_reg(0x21, 0x02);         
         resp = SCCB_write_reg(0x22, 0x91);
         resp = SCCB_write_reg(0x29, 0x07);         
         resp = SCCB_write_reg(0x33, 0x0b);
         resp = SCCB_write_reg(0x35, 0x0b);         
         resp = SCCB_write_reg(0x37, 0x1d);
         resp = SCCB_write_reg(0x38, 0x71);         
         resp = SCCB_write_reg(0x39, 0x2a);
         resp = SCCB_write_reg(0x3c, 0x78);    
         resp = SCCB_write_reg(0x4d, 0x40);
         resp = SCCB_write_reg(0x4e, 0x20);         
         resp = SCCB_write_reg(0x69, 0);
         resp = SCCB_write_reg(0x6b, 0x0a);         
         resp = SCCB_write_reg(0x74, 0x10);
         resp = SCCB_write_reg(0x8d, 0x4f);         
         resp = SCCB_write_reg(0x8e, 0);
         resp = SCCB_write_reg(0x8f, 0);            
         resp = SCCB_write_reg(0x90, 0);
         resp = SCCB_write_reg(0x91, 0);            
         resp = SCCB_write_reg(0x96, 0);
         resp = SCCB_write_reg(0x9a, 0);          
         resp = SCCB_write_reg(0xb0, 0x84);
         resp = SCCB_write_reg(0xb1, 0x0c);         
         resp = SCCB_write_reg(0xb2, 0x0e);
         resp = SCCB_write_reg(0xb3, 0x82);         
         resp = SCCB_write_reg(0xb8, 0x0a);
        
        /* More reserved magic, some of which tweaks white balance */
         resp = SCCB_write_reg(0x43, 0x0a);         
         resp = SCCB_write_reg(0x44, 0xf0);
         resp = SCCB_write_reg(0x45, 0x34);         
         resp = SCCB_write_reg(0x46, 0x58);
         resp = SCCB_write_reg(0x47, 0x28);         
         resp = SCCB_write_reg(0x48, 0x3a);
         resp = SCCB_write_reg(0x59, 0x88);         
         resp = SCCB_write_reg(0x5a, 0x88);
         resp = SCCB_write_reg(0x5b, 0x44);         
         resp = SCCB_write_reg(0x5c, 0x67);
         resp = SCCB_write_reg(0x5d, 0x49);         
         resp = SCCB_write_reg(0x5e, 0x0e);
         resp = SCCB_write_reg(0x6c, 0x0a);         
         resp = SCCB_write_reg(0x6d, 0x55);
         resp = SCCB_write_reg(0x6e, 0x11);         
         resp = SCCB_write_reg(0x6f, 0x9f); 
         resp = SCCB_write_reg(0x6a, 0x40);         
         resp = SCCB_write_reg(0x01, 0x40);
         resp = SCCB_write_reg(0x02, 0x60);
         resp = SCCB_write_reg(0x13, COM8_FASTAEC|COM8_AECSTEP|COM8_BFILT|COM8_AGC|COM8_AEC|COM8_AWB);  
        
        /* Matrix coefficients */
         resp = SCCB_write_reg(0x4f, 0x80);         
         resp = SCCB_write_reg(0x50, 0x80);
         resp = SCCB_write_reg(0x51, 0);            
         resp = SCCB_write_reg(0x52, 0x22);
         resp = SCCB_write_reg(0x53, 0x5e);         
         resp = SCCB_write_reg(0x54, 0x80);
         resp = SCCB_write_reg(0x58, 0x9e);
        
         resp = SCCB_write_reg(0x41, 0x08);   
         resp = SCCB_write_reg(0x3f, 0);
         resp = SCCB_write_reg(0x75, 0x05);         
         resp = SCCB_write_reg(0x76, 0xe1);
         resp = SCCB_write_reg(0x4c, 0);            
         resp = SCCB_write_reg(0x77, 0x01);
         resp = SCCB_write_reg(0x3d, 0xc3);    
         resp = SCCB_write_reg(0x4b, 0x09);
         resp = SCCB_write_reg(0xc9, 0x60);         
         resp = SCCB_write_reg(0x41, 0x38);
         resp = SCCB_write_reg(0x56, 0x40);
        
         resp = SCCB_write_reg(0x34, 0x11);         
         resp = SCCB_write_reg(0x3b, COM11_EXP|COM11_HZAUTO);
         resp = SCCB_write_reg(0xa4, 0x88);         
         resp = SCCB_write_reg(0x96, 0);
         resp = SCCB_write_reg(0x97, 0x30);         
         resp = SCCB_write_reg(0x98, 0x20);
         resp = SCCB_write_reg(0x99, 0x30);         
         resp = SCCB_write_reg(0x9a, 0x84);
         resp = SCCB_write_reg(0x9b, 0x29);         
         resp = SCCB_write_reg(0x9c, 0x03);
         resp = SCCB_write_reg(0x9d, 0x4c);         
         resp = SCCB_write_reg(0x9e, 0x3f);
         resp = SCCB_write_reg(0x78, 0x04);
        
        /* Extra-weird stuff.  Some sort of multiplexor register */
         resp = SCCB_write_reg(0x79, 0x01);         
         resp = SCCB_write_reg(0xc8, 0xf0);
         resp = SCCB_write_reg(0x79, 0x0f);         
         resp = SCCB_write_reg(0xc8, 0x00);
         resp = SCCB_write_reg(0x79, 0x10);         
         resp = SCCB_write_reg(0xc8, 0x7e);
         resp = SCCB_write_reg(0x79, 0x0a);         
         resp = SCCB_write_reg(0xc8, 0x80);
         resp = SCCB_write_reg(0x79, 0x0b);         
         resp = SCCB_write_reg(0xc8, 0x01);
         resp = SCCB_write_reg(0x79, 0x0c);         
         resp = SCCB_write_reg(0xc8, 0x0f);
         resp = SCCB_write_reg(0x79, 0x0d);         
         resp = SCCB_write_reg(0xc8, 0x20);
         resp = SCCB_write_reg(0x79, 0x09);         
         resp = SCCB_write_reg(0xc8, 0x80);
         resp = SCCB_write_reg(0x79, 0x02);         
         resp = SCCB_write_reg(0xc8, 0xc0);
         resp = SCCB_write_reg(0x79, 0x03);         
         resp = SCCB_write_reg(0xc8, 0x40);
         resp = SCCB_write_reg(0x79, 0x05);         
         resp = SCCB_write_reg(0xc8, 0x30);
         resp = SCCB_write_reg(0x79, 0x26);
        
         resp = SCCB_write_reg(0xff, 0xff); /* END MARKER */    
}

#include "stm32h7xx_hal_dcmi.h"

#define IMG_ROWS   					320
#define IMG_COLUMNS   				240
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


/**
  * @brief  DMA error callback
  * @param  hdma pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
static void DCMI_DMAError(DMA_HandleTypeDef *hdma)
{
  DCMI_HandleTypeDef *hdcmi = (DCMI_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

  if (hdcmi->DMA_Handle->ErrorCode != HAL_DMA_ERROR_FE)
  {
    /* Initialize the DCMI state*/
    hdcmi->State = HAL_DCMI_STATE_READY;

    /* Set DCMI Error Code */
    hdcmi->ErrorCode |= HAL_DCMI_ERROR_DMA;
  }

  /* DCMI error Callback */
#if (USE_HAL_DCMI_REGISTER_CALLBACKS == 1)
  /*Call registered DCMI error callback*/
  hdcmi->ErrorCallback(hdcmi);
#else
  HAL_DCMI_ErrorCallback(hdcmi);
#endif /* USE_HAL_DCMI_REGISTER_CALLBACKS */
}


static void DCMI_DMAXferCpltMB2(DMA_HandleTypeDef *hdma)
{
  uint32_t tmp ;

  DCMI_HandleTypeDef *hdcmi = (DCMI_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

  if (hdcmi->XferCount != 0U)
  {
    /* Update memory 0 address location */
    tmp = ((((DMA_Stream_TypeDef *)(hdcmi->DMA_Handle->Instance))->CR) & DMA_SxCR_CT);
    if (((hdcmi->XferCount % 2U) == 0U) && (tmp != 0U))
    {
      tmp = ((DMA_Stream_TypeDef *)(hdcmi->DMA_Handle->Instance))->M0AR;
      (void)HAL_DMAEx_ChangeMemory(hdcmi->DMA_Handle, (uint32_t)(new_frame_buffer.pFrameBuf3), MEMORY0);
      hdcmi->XferCount--;
    }
    /* Update memory 1 address location */
    else if ((((DMA_Stream_TypeDef *)(hdcmi->DMA_Handle->Instance))->CR & DMA_SxCR_CT) == 0U)
    {
      tmp = ((DMA_Stream_TypeDef *)(hdcmi->DMA_Handle->Instance))->M1AR;
      (void)HAL_DMAEx_ChangeMemory(hdcmi->DMA_Handle, (uint32_t)(new_frame_buffer.pFrameBuf4), MEMORY1);
      hdcmi->XferCount--;
    }
    else
    {
      /* Nothing to do */
    }
  }
  /* Update memory 0 address location */
  else if ((((DMA_Stream_TypeDef *)(hdcmi->DMA_Handle->Instance))->CR & DMA_SxCR_CT) != 0U)
  {
    ((DMA_Stream_TypeDef *)(hdcmi->DMA_Handle->Instance))->M0AR = hdcmi->pBuffPtr;
  }
  /* Update memory 1 address location */
  else if ((((DMA_Stream_TypeDef *)(hdcmi->DMA_Handle->Instance))->CR & DMA_SxCR_CT) == 0U)
  {
    tmp = hdcmi->pBuffPtr;
    ((DMA_Stream_TypeDef *)(hdcmi->DMA_Handle->Instance))->M1AR = (uint32_t)(new_frame_buffer.pFrameBuf2);
    hdcmi->XferCount = hdcmi->XferTransferNumber;
  }
  else
  {
    /* Nothing to do */
  }

  /* Check if the frame is transferred */
  if (hdcmi->XferCount == hdcmi->XferTransferNumber)
  {
    /* Enable the Frame interrupt */
    __HAL_DCMI_ENABLE_IT(hdcmi, DCMI_IT_FRAME);

    /* When snapshot mode, set dcmi state to ready */
    if ((hdcmi->Instance->CR & DCMI_CR_CM) == DCMI_MODE_SNAPSHOT)
    {
      hdcmi->State = HAL_DCMI_STATE_READY;
    }
  }
}

/**
  * @brief  Enables DCMI DMA request and enables DCMI capture
  * @param  hdcmi     pointer to a DCMI_HandleTypeDef structure that contains
  *                    the configuration information for DCMI.
  * @param  DCMI_Mode DCMI capture mode snapshot or continuous grab.
  * @param  pData     The destination memory Buffer address (LCD Frame buffer).
  * @param  Length    The length of capture to be transferred.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_DCMI_MultiBufferStart_DMA(DCMI_HandleTypeDef *hdcmi, uint32_t DCMI_Mode, sFrameBuf_t *frameBuf, uint32_t Length)
{

  /* Check function parameters */
  assert_param(IS_DCMI_CAPTURE_MODE(DCMI_Mode));

  /* Process Locked */
  __HAL_LOCK(hdcmi);

  /* Lock the DCMI peripheral state */
  hdcmi->State = HAL_DCMI_STATE_BUSY;

  /* Enable DCMI by setting DCMIEN bit */
  __HAL_DCMI_ENABLE(hdcmi);

  /* Configure the DCMI Mode */
  hdcmi->Instance->CR &= ~(DCMI_CR_CM);
  hdcmi->Instance->CR |= (uint32_t)(DCMI_Mode);

  /* Set the DMA memory0 conversion complete callback */
  hdcmi->DMA_Handle->XferCpltCallback = DCMI_DMAXferCpltMB2;

  /* Set the DMA error callback */
  hdcmi->DMA_Handle->XferErrorCallback = DCMI_DMAError;

  /* Set the dma abort callback */
  hdcmi->DMA_Handle->XferAbortCallback = NULL;

    /* DCMI_DOUBLE_BUFFER Mode */
    /* Set the DMA memory1 conversion complete callback */
    hdcmi->DMA_Handle->XferM1CpltCallback = DCMI_DMAXferCpltMB2;

    /* Initialize transfer parameters */
    hdcmi->XferCount = 2;
    hdcmi->XferTransferNumber = hdcmi->XferCount;
    //hdcmi->XferSize = Length / 3;
    hdcmi->XferSize = Length / 4;
    hdcmi->pBuffPtr = (uint32_t)(frameBuf->pFrameBuf1);

    /* Start DMA multi buffer transfer */
    if (HAL_DMAEx_MultiBufferStart_IT(hdcmi->DMA_Handle, (uint32_t)&hdcmi->Instance->DR, (uint32_t)(frameBuf->pFrameBuf1),
                                        (uint32_t)(frameBuf->pFrameBuf2), hdcmi->XferSize ) != HAL_OK)
    {
      /* Set Error Code */
      hdcmi->ErrorCode = HAL_DCMI_ERROR_DMA;
      /* Change DCMI state */
      hdcmi->State = HAL_DCMI_STATE_READY;
      /* Release Lock */
      __HAL_UNLOCK(hdcmi);
      /* Return function status */
      return HAL_ERROR;
    }

  /* Enable Capture */
  hdcmi->Instance->CR |= DCMI_CR_CAPTURE;

  /* Release Lock */
  __HAL_UNLOCK(hdcmi);

  /* Return function status */
  return HAL_OK;
}



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


            /**
             * @todo разобраться, почему при стирании на изображении появляются артефакты
             */
            //memset(new_frame_buffer.pFrameBuf1, 0, FRAME_BUF_PART_SIZE*4);
            //memset(new_frame_buffer.pFrameBuf2, 0, FRAME_BUF_PART_SIZE*4);
            //memset(new_frame_buffer.pFrameBuf3, 0, FRAME_BUF_PART_SIZE*4);
            //memset(new_frame_buffer.pFrameBuf4, 0, FRAME_BUF_PART_SIZE*4);

            if ( camera.regAddr != 0 ){
            	resp = SCCB_write_reg(camera.regAddr, camera.regVal);
				HAL_Delay(100);
            }

            MX_USB_DEVICE_Stop();

            if (camera.pictureName[0] != 0 && camera.regAddr == 0 )
            	ConfigCamera6();

            HAL_Delay(100);
            
            HAL_StatusTypeDef result;
            //result = HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t) frame_buffer, IMG_ROWS * IMG_COLUMNS / 2);
            //result = HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t) new_frame_buffer.pFrameBuf3, IMG_ROWS * IMG_COLUMNS / 2);
            result = HAL_DCMI_MultiBufferStart_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, &new_frame_buffer, new_frame_buffer.size);
	        result = HAL_DCMI_Stop(&hdcmi);
	        if (camera.pictureName[0] != 0 )
	        	SavePictureMB(camera.pictureName, &new_frame_buffer, 320 * 240 / 4/*IMG_ROWS * IMG_COLUMNS*/ );
	        	//SavePicture(camera.pictureName, (uint16_t*) new_frame_buffer.pFrameBuf3, 320 * 240/*IMG_ROWS * IMG_COLUMNS*/ );

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



