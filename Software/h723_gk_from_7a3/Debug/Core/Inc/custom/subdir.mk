################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Inc/custom/cxk.c \
../Core/Inc/custom/debug.c \
../Core/Inc/custom/inner_main.c \
../Core/Inc/custom/sd_driver.c \
../Core/Inc/custom/segment_lcd.c \
../Core/Inc/custom/tas2563.c 

C_DEPS += \
./Core/Inc/custom/cxk.d \
./Core/Inc/custom/debug.d \
./Core/Inc/custom/inner_main.d \
./Core/Inc/custom/sd_driver.d \
./Core/Inc/custom/segment_lcd.d \
./Core/Inc/custom/tas2563.d 

OBJS += \
./Core/Inc/custom/cxk.o \
./Core/Inc/custom/debug.o \
./Core/Inc/custom/inner_main.o \
./Core/Inc/custom/sd_driver.o \
./Core/Inc/custom/segment_lcd.o \
./Core/Inc/custom/tas2563.o 


# Each subdirectory must supply rules for building sources it contributes
Core/Inc/custom/%.o Core/Inc/custom/%.su Core/Inc/custom/%.cyclo: ../Core/Inc/custom/%.c Core/Inc/custom/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H723xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Inc-2f-custom

clean-Core-2f-Inc-2f-custom:
	-$(RM) ./Core/Inc/custom/cxk.cyclo ./Core/Inc/custom/cxk.d ./Core/Inc/custom/cxk.o ./Core/Inc/custom/cxk.su ./Core/Inc/custom/debug.cyclo ./Core/Inc/custom/debug.d ./Core/Inc/custom/debug.o ./Core/Inc/custom/debug.su ./Core/Inc/custom/inner_main.cyclo ./Core/Inc/custom/inner_main.d ./Core/Inc/custom/inner_main.o ./Core/Inc/custom/inner_main.su ./Core/Inc/custom/sd_driver.cyclo ./Core/Inc/custom/sd_driver.d ./Core/Inc/custom/sd_driver.o ./Core/Inc/custom/sd_driver.su ./Core/Inc/custom/segment_lcd.cyclo ./Core/Inc/custom/segment_lcd.d ./Core/Inc/custom/segment_lcd.o ./Core/Inc/custom/segment_lcd.su ./Core/Inc/custom/tas2563.cyclo ./Core/Inc/custom/tas2563.d ./Core/Inc/custom/tas2563.o ./Core/Inc/custom/tas2563.su

.PHONY: clean-Core-2f-Inc-2f-custom

