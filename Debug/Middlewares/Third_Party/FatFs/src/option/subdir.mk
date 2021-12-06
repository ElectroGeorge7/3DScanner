################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/Third_Party/FatFs/src/option/syscall.c 

OBJS += \
./Middlewares/Third_Party/FatFs/src/option/syscall.o 

C_DEPS += \
./Middlewares/Third_Party/FatFs/src/option/syscall.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/Third_Party/FatFs/src/option/syscall.o: ../Middlewares/Third_Party/FatFs/src/option/syscall.c
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DSTM32 -DSTM32H7SINGLE -DSTM32H7 -DSTM32H743VITx -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Inc -I"G:/BMSTU/Bachelor_Qualification_Work/Code/3DScanner_WS/3DScanner/Drivers/STM32H7xx_HAL_Driver/Inc" -I"G:/BMSTU/Bachelor_Qualification_Work/Code/3DScanner_WS/3DScanner/Drivers/CMSIS/Device/ST/STM32H7xx/Include" -I"G:/BMSTU/Bachelor_Qualification_Work/Code/3DScanner_WS/3DScanner/Drivers/CMSIS/Include" -I"G:/BMSTU/Bachelor_Qualification_Work/Code/3DScanner_WS/3DScanner/Core/Inc" -I"G:/BMSTU/Bachelor_Qualification_Work/Code/3DScanner_WS/3DScanner/FATFS/Target" -I"G:/BMSTU/Bachelor_Qualification_Work/Code/3DScanner_WS/3DScanner/FATFS/App" -I"G:/BMSTU/Bachelor_Qualification_Work/Code/3DScanner_WS/3DScanner/Middlewares/Third_Party/FatFs/src" -I"G:/BMSTU/Bachelor_Qualification_Work/Code/3DScanner_WS/3DScanner/Middlewares/Third_Party/FatFs/src/option" -I"G:/BMSTU/Bachelor_Qualification_Work/Code/3DScanner_WS/3DScanner/Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Inc" -I"G:/BMSTU/Bachelor_Qualification_Work/Code/3DScanner_WS/3DScanner/Middlewares/ST/STM32_USB_Device_Library/Core/Inc" -I"G:/BMSTU/Bachelor_Qualification_Work/Code/3DScanner_WS/3DScanner/USB_DEVICE/App" -I"G:/BMSTU/Bachelor_Qualification_Work/Code/3DScanner_WS/3DScanner/USB_DEVICE/Target" -I"G:/BMSTU/Bachelor_Qualification_Work/Code/3DScanner_WS/3DScanner/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F" -I"G:/BMSTU/Bachelor_Qualification_Work/Code/3DScanner_WS/3DScanner/Middlewares/Third_Party/FreeRTOS/Source/include" -I"G:/BMSTU/Bachelor_Qualification_Work/Code/3DScanner_WS/3DScanner/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2" -I"G:/BMSTU/Bachelor_Qualification_Work/Code/3DScanner_WS/3DScanner/Apps" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"Middlewares/Third_Party/FatFs/src/option/syscall.d" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

