################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/calculations.c \
../Core/Src/displayingdata.c \
../Core/Src/doppler.c \
../Core/Src/fft.c \
../Core/Src/fmcw.c \
../Core/Src/main.c \
../Core/Src/measuring.c \
../Core/Src/menu.c \
../Core/Src/pushbutton.c \
../Core/Src/range_doppler.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/system_stm32f4xx.c 

OBJS += \
./Core/Src/calculations.o \
./Core/Src/displayingdata.o \
./Core/Src/doppler.o \
./Core/Src/fft.o \
./Core/Src/fmcw.o \
./Core/Src/main.o \
./Core/Src/measuring.o \
./Core/Src/menu.o \
./Core/Src/pushbutton.o \
./Core/Src/range_doppler.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/system_stm32f4xx.o 

C_DEPS += \
./Core/Src/calculations.d \
./Core/Src/displayingdata.d \
./Core/Src/doppler.d \
./Core/Src/fft.d \
./Core/Src/fmcw.d \
./Core/Src/main.d \
./Core/Src/measuring.d \
./Core/Src/menu.d \
./Core/Src/pushbutton.d \
./Core/Src/range_doppler.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/system_stm32f4xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -DDEBUG -DSTM32F429xx -DARM_MATH_CM4 -c -I../Core/Inc -I../Drivers/CMSIS/Include -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I"../Drivers/BSP/Components/Common" -I"../Drivers/BSP/Components/stmpe811" -I"../Drivers/BSP/Components/ili9341" -I"../Drivers/BSP/STM32F429I-Discovery" -I"../Utilities/Fonts" -I../Drivers/CMSIS/DSP/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/calculations.d ./Core/Src/calculations.o ./Core/Src/calculations.su ./Core/Src/displayingdata.d ./Core/Src/displayingdata.o ./Core/Src/displayingdata.su ./Core/Src/doppler.d ./Core/Src/doppler.o ./Core/Src/doppler.su ./Core/Src/fft.d ./Core/Src/fft.o ./Core/Src/fft.su ./Core/Src/fmcw.d ./Core/Src/fmcw.o ./Core/Src/fmcw.su ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/measuring.d ./Core/Src/measuring.o ./Core/Src/measuring.su ./Core/Src/menu.d ./Core/Src/menu.o ./Core/Src/menu.su ./Core/Src/pushbutton.d ./Core/Src/pushbutton.o ./Core/Src/pushbutton.su ./Core/Src/range_doppler.d ./Core/Src/range_doppler.o ./Core/Src/range_doppler.su ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su

.PHONY: clean-Core-2f-Src

