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

/// @brief assigning an ordinal number to modules
enum moduleNumber{
    Laser, 
    StepM
};

/// @brief type enums of modules function state
typedef enum{
    DIS = 0,
    EN
} PowerState_t;

typedef enum{
    CW = 0,
    CCW
} RotDirection_t;

/**
 * @note each module`s structure consists of this parts:
 * 1. module`s name - string with which the command begins, it is determined to which module the command belongs
 * 2. module functions structure - here all the available module functions and the functions state variables are listed
 * 3. module parameters structure - here all the available module parameters and the parameters value are listed
 * 
 * the difference between modules function and parameter is that parameter value is a number from some range, in the same time 
 * function has some definite state
 * 
 * @example some commands example:
 *          laser -en -bright:20
 *          stepM -en -cw -steps:125 -speed:1
*/

struct sLaser_t {
    const char *module_name;

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


struct sStepM_t {
    const char *module_name;

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
            uint8_t stepsVal;
        };

        // stepM number of steps param
        struct {
            const char *speed_param_name;
            uint8_t speedVal;
        };
    };

};


#define CAMERA_FILES_MAX_LENGTH 12

struct sCamera_t {
    const char *module_name;

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


/// @brief the array of pointers to module structures
uint32_t *psModules[MODULES_NUMBER];


//--------------------------------------------------- Public functions ---------------------------------------------//
/// @brief Callback functions for define command that referred to specific module
HAL_StatusTypeDef laser_cmd_define_cb(uint8_t *cmdStr);
HAL_StatusTypeDef stepM_cmd_define_cb(uint8_t *cmdStr);
HAL_StatusTypeDef camera_cmd_define_cb(uint8_t *cmdStr);
//------------------------------------------------------------------------------------------------------------------//


//--------------------------------------------------- Private functions --------------------------------------------//
//------------------------------------------------------------------------------------------------------------------//

typedef HAL_StatusTypeDef (*module_cmd_define_cb_t)(uint8_t *cmdStr);
/// @brief array of callback functions for analyse command according to module
module_cmd_define_cb_t module_cmd_define_cbs[MODULES_NUMBER];


// struct psModules_t {
//     //uint32_t *pRef;
//     struct sLaser_t *psLaser;
//     struct sStepM_t *psStepM;
// };

//extern struct psModules_t psModules;

// union upsModule_t{
// 	struct sLaser_t *upsLaser;
// 	struct sStepM_t *upsStepM;
// };

// // Array of module structures 
// union upsModule_t modules[MODULES_NUMBER] = {
// 		{.upsLaser = &laser},
// 		{.upsStepM = &stepM}
// };

#endif /* MODULES_H */
