################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/Voltage_Reader/Src/battery_manager.c 

OBJS += \
./Drivers/Voltage_Reader/Src/battery_manager.o 

C_DEPS += \
./Drivers/Voltage_Reader/Src/battery_manager.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/Voltage_Reader/Src/%.o Drivers/Voltage_Reader/Src/%.su Drivers/Voltage_Reader/Src/%.cyclo: ../Drivers/Voltage_Reader/Src/%.c Drivers/Voltage_Reader/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L496xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-Voltage_Reader-2f-Src

clean-Drivers-2f-Voltage_Reader-2f-Src:
	-$(RM) ./Drivers/Voltage_Reader/Src/battery_manager.cyclo ./Drivers/Voltage_Reader/Src/battery_manager.d ./Drivers/Voltage_Reader/Src/battery_manager.o ./Drivers/Voltage_Reader/Src/battery_manager.su

.PHONY: clean-Drivers-2f-Voltage_Reader-2f-Src

