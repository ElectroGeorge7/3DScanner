
#include "camera_hw_init.h"
#include "camera_hw_i2c.h"
#include "../Modules/Cam_OV7670/ov7670.h"
#include "../Tasks/storage_task.h"

HAL_StatusTypeDef camera_init(void){
	camera_hw_init();
	camera_on();
	camera_default_config();
};


HAL_StatusTypeDef camera_on(void){
	camera_hw_on();
};

HAL_StatusTypeDef camera_off(void){
	camera_hw_off();
};

HAL_StatusTypeDef camera_config_reg(uint8_t reg_addr, uint8_t data){
	SCCB_write_reg(reg_addr, data);
	return HAL_OK;
};

HAL_StatusTypeDef camera_config(const uint8_t camConf[][2]){
	uint8_t data, i = 0;
	uint8_t resp;
	const uint8_t *packet;
	uint32_t timeout = 0xFFFFFF;

	// Configure camera registers
	for (i = 0; i < 134; i++) {
		data = camConf[i][1];
		if (i == 0)
			HAL_Delay(100);
		SCCB_write_reg(camConf[i][0], data);

		//if (resp != data) {
		//	break;
		//}
	}
	return resp;
};

HAL_StatusTypeDef camera_default_config(void){
	camera_config(ov7670_default_config);
};

HAL_StatusTypeDef camera_get_frame(struct sCamera_t *camera, sFrameBuf_t *frameBuf){
    HAL_StatusTypeDef result;
    result = camera_hw_get_frame(frameBuf);

//    if (camera->pictureName[0] != 0 )
//    	SavePictureMB(camera->pictureName, frameBuf, (frameBuf->size) / 2);
}
