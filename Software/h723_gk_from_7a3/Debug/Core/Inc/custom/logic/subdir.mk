################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Inc/custom/logic/display.c \
../Core/Inc/custom/logic/maudio.c \
../Core/Inc/custom/logic/mevent.c 

C_DEPS += \
./Core/Inc/custom/logic/display.d \
./Core/Inc/custom/logic/maudio.d \
./Core/Inc/custom/logic/mevent.d 

OBJS += \
./Core/Inc/custom/logic/display.o \
./Core/Inc/custom/logic/maudio.o \
./Core/Inc/custom/logic/mevent.o 


# Each subdirectory must supply rules for building sources it contributes
Core/Inc/custom/logic/%.o Core/Inc/custom/logic/%.su Core/Inc/custom/logic/%.cyclo: ../Core/Inc/custom/logic/%.c Core/Inc/custom/logic/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H723xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Inc-2f-custom-2f-logic

clean-Core-2f-Inc-2f-custom-2f-logic:
	-$(RM) ./Core/Inc/custom/logic/display.cyclo ./Core/Inc/custom/logic/display.d ./Core/Inc/custom/logic/display.o ./Core/Inc/custom/logic/display.su ./Core/Inc/custom/logic/maudio.cyclo ./Core/Inc/custom/logic/maudio.d ./Core/Inc/custom/logic/maudio.o ./Core/Inc/custom/logic/maudio.su ./Core/Inc/custom/logic/mevent.cyclo ./Core/Inc/custom/logic/mevent.d ./Core/Inc/custom/logic/mevent.o ./Core/Inc/custom/logic/mevent.su

.PHONY: clean-Core-2f-Inc-2f-custom-2f-logic

