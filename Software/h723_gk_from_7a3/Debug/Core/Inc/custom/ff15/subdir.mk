################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Inc/custom/ff15/diskio.c \
../Core/Inc/custom/ff15/ff.c \
../Core/Inc/custom/ff15/ffsystem.c \
../Core/Inc/custom/ff15/ffunicode.c 

C_DEPS += \
./Core/Inc/custom/ff15/diskio.d \
./Core/Inc/custom/ff15/ff.d \
./Core/Inc/custom/ff15/ffsystem.d \
./Core/Inc/custom/ff15/ffunicode.d 

OBJS += \
./Core/Inc/custom/ff15/diskio.o \
./Core/Inc/custom/ff15/ff.o \
./Core/Inc/custom/ff15/ffsystem.o \
./Core/Inc/custom/ff15/ffunicode.o 


# Each subdirectory must supply rules for building sources it contributes
Core/Inc/custom/ff15/%.o Core/Inc/custom/ff15/%.su Core/Inc/custom/ff15/%.cyclo: ../Core/Inc/custom/ff15/%.c Core/Inc/custom/ff15/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H723xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/MSC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Inc-2f-custom-2f-ff15

clean-Core-2f-Inc-2f-custom-2f-ff15:
	-$(RM) ./Core/Inc/custom/ff15/diskio.cyclo ./Core/Inc/custom/ff15/diskio.d ./Core/Inc/custom/ff15/diskio.o ./Core/Inc/custom/ff15/diskio.su ./Core/Inc/custom/ff15/ff.cyclo ./Core/Inc/custom/ff15/ff.d ./Core/Inc/custom/ff15/ff.o ./Core/Inc/custom/ff15/ff.su ./Core/Inc/custom/ff15/ffsystem.cyclo ./Core/Inc/custom/ff15/ffsystem.d ./Core/Inc/custom/ff15/ffsystem.o ./Core/Inc/custom/ff15/ffsystem.su ./Core/Inc/custom/ff15/ffunicode.cyclo ./Core/Inc/custom/ff15/ffunicode.d ./Core/Inc/custom/ff15/ffunicode.o ./Core/Inc/custom/ff15/ffunicode.su

.PHONY: clean-Core-2f-Inc-2f-custom-2f-ff15

