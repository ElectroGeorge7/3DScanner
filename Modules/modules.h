/**
 * @author Katukiya G.
 *
 * @file modules.h 
 *
 * @brief Structures types of 3D scanner`s modules.
 */

#ifndef MODULES_H
#define MODULES_H

#include "stdint.h"
#include "stm32h7xx_hal.h"

#define MODULES_NUMBER 3

typedef enum{
	SCANNER_OK = 0,
    SCANNER_ERROR = 0xff
} ScannerStatus_t;

/// @brief assigning an ordinal number to modules
typedef enum{
    LASER = 0,
	CAMERA,
    STEPM
} ModuleId_t;

/// @brief type enums of modules function state
typedef enum{
    DIS = 0,
    EN
} PowerState_t;

typedef enum{
    CW = 0,
    CCW
} RotDirection_t;


typedef ScannerStatus_t (*ModuleCmdParserCb_t)(uint8_t *cmdStr);
typedef ScannerStatus_t (*ModuleCmdCb_t)(void);

typedef struct ModulesHandle{

    /// @brief the array of pointers to module structures
    uint32_t *psModulesArray[MODULES_NUMBER];

    /// @brief array of callback functions for parse command string into module struct
    ModuleCmdParserCb_t moduleCmdParserCbs[MODULES_NUMBER];

    /// @brief array of callback functions to execute module command
    ModuleCmdCb_t moduleCmdCbs[MODULES_NUMBER];
    
} ModulesHandle_t;

/**
 * @note each module`s structure consists of this parts:
 * 1. module`s name - string with which the command begins, it is determined to which module the command belongs
 * 2. module functions structure - here all the available module functions and the functions state variables are listed
 * 3. module parameters structure - here all the available module parameters and the parameters value are listed
 * 
 * the difference between modules function and parameter is that parameter value is a number from some range, in the same time 
 * function has some definite state
 * 
 * @example some commands example:e[ m]
 *          laser -en -bright:20
 *          stepM -en -cw -steps:125 -speed:1
*/

#define LASER_POWER_UPDATE_F      0x01
#define LASER_BRIGHT_UPDATE_F     0x02

struct sLaser_t {
    const char *module_name;
    uint8_t status;

     // list of laser functions
    struct {
        // laser power func
        struct {
            const char *power_func_names[2];
            PowerState_t powerState;
        };
    };

    // list of laser params
    struct {
        // laser brightness param
        struct {
            const char *bright_param_name;
            uint8_t brightVal;
        };
    };

};

#define STEPM_POWER_UPDATE_F      0x01
#define STEPM_DIR_UPDATE_F        0x02
#define STEPM_STEPS_UPDATE_F      0x04
#define STEPM_SPEED_UPDATE_F      0x08

struct sStepM_t {
    const char *module_name;
    uint8_t status;

    // list of stepM functions
    struct {
        // stepM power func
        struct {
            const char *power_func_names[2];
            PowerState_t powerState;   
        };
        // stepM direction of rotation func
        struct {
            const char *rot_func_names[2];
            RotDirection_t  rotDirection;  
        };
    };

    // list of stepM params
    struct {
        // stepM number of steps param
        struct {
            const char *steps_param_name;
            uint16_t stepsVal;
        };

        // stepM number of steps param
        struct {
            const char *speed_param_name;
            uint8_t speedVal;
        };
    };

};


#define CAMERA_FILES_MAX_LENGTH 12

#define CAMERA_POWER_UPDATE_F      0x01
#define CAMERA_PICTURE_UPDATE_F    0x02
#define CAMERA_CONFIG_UPDATE_F     0x04
#define CAMERA_REG_UPDATE_F        0x08

#define CAMERA_EVT_FILE_CREATE          0x01
#define CAMERA_EVT_FILE_CREATE_DONE     0x02
#define CAMERA_EVT_DIR_CREATE           0x04
#define CAMERA_EVT_DIR_CREATE_DONE      0x08

#include "camera.h"

typedef struct {
  const char fileName[2*CAMERA_FILES_MAX_LENGTH];
  const char dirName[CAMERA_FILES_MAX_LENGTH];
  uint32_t *frameBuf;
  uint32_t *configBuf;
  uint32_t configBufSize;
} CameraQueueObj_t;

struct sCamera_t {
    const char *module_name;
    uint8_t status;

    // list of camera function
    struct {
        // camera power func
        struct {
            const char *power_func_names[2];
            PowerState_t powerState; 
        }
    };

    // list of camera params
    struct {
        // take one picture and save it as file "pictureName"
        struct {
            const char *picture_param_name;
            char pictureName[CAMERA_FILES_MAX_LENGTH];
        };

        // write in camera`s registers values from file "configFileName"
        struct {
            const char *config_param_name;
            char configFileName[CAMERA_FILES_MAX_LENGTH];
        };
        
        // write in one camera`s register "regName" the value "regVal"
        struct {
            const char *reg_param_name;
            uint8_t regAddr;
            uint8_t regVal;
        };
    };
};



ModulesHandle_t *modules_init(void);
ScannerStatus_t module_cmd_parse(ModulesHandle_t *phModules, const char *cmdStr, ModuleId_t *moduleId);


#endif /* MODULES_H */
