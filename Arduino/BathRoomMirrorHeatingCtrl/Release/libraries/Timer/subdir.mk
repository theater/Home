################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
D:/HomeAutomation/ArduinoLibs/Timer/Event.cpp \
D:/HomeAutomation/ArduinoLibs/Timer/Timer.cpp 

LINK_OBJ += \
./libraries/Timer/Event.cpp.o \
./libraries/Timer/Timer.cpp.o 

CPP_DEPS += \
./libraries/Timer/Event.cpp.d \
./libraries/Timer/Timer.cpp.d 


# Each subdirectory must supply rules for building sources it contributes
libraries/Timer/Event.cpp.o: D:/HomeAutomation/ArduinoLibs/Timer/Event.cpp
	@echo 'Building file: $<'
	@echo 'Starting C++ compile'
	"C:\eclipse\arduinoPlugin\tools\arduino\avr-gcc\4.8.1-arduino5/bin/avr-g++" -c -g -Os -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -MMD -mmcu=atmega168 -DF_CPU=16000000L -DARDUINO=10606 -DARDUINO_AVR_PRO -DARDUINO_ARCH_AVR     -I"C:\eclipse\arduinoPlugin\packages\arduino\hardware\avr\1.6.5\cores\arduino" -I"C:\eclipse\arduinoPlugin\packages\arduino\hardware\avr\1.6.5\variants\eightanaloginputs" -I"D:\HomeAutomation\ArduinoLibs\Timer" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -D__IN_ECLIPSE__=1 -x c++ "$<"  -o  "$@"   -Wall
	@echo 'Finished building: $<'
	@echo ' '

libraries/Timer/Timer.cpp.o: D:/HomeAutomation/ArduinoLibs/Timer/Timer.cpp
	@echo 'Building file: $<'
	@echo 'Starting C++ compile'
	"C:\eclipse\arduinoPlugin\tools\arduino\avr-gcc\4.8.1-arduino5/bin/avr-g++" -c -g -Os -fno-exceptions -ffunction-sections -fdata-sections -fno-threadsafe-statics -MMD -mmcu=atmega168 -DF_CPU=16000000L -DARDUINO=10606 -DARDUINO_AVR_PRO -DARDUINO_ARCH_AVR     -I"C:\eclipse\arduinoPlugin\packages\arduino\hardware\avr\1.6.5\cores\arduino" -I"C:\eclipse\arduinoPlugin\packages\arduino\hardware\avr\1.6.5\variants\eightanaloginputs" -I"D:\HomeAutomation\ArduinoLibs\Timer" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -D__IN_ECLIPSE__=1 -x c++ "$<"  -o  "$@"   -Wall
	@echo 'Finished building: $<'
	@echo ' '


