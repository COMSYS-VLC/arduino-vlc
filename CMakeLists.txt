set(ARDUINO_1_5 True)
set(ARDUINO_CPU atmega2560)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake) # Arduino Toolchain

cmake_minimum_required(VERSION 2.8)

project(vlc C CXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

print_board_list()

set(ARDUINO_DEFAULT_BOARD mega)
set(ARDUINO_DEFAULT_PORT /dev/ttyACM3)

generate_avr_firmware(train SRCS main.cpp TrainController.cpp MotorController.cpp LEDController.cpp Clock.cpp MAC.cpp VLCMAC.cpp VLCPHY.cpp UART.cpp BOARD mega PORT /dev/ttyTrain)
generate_avr_firmware(controller SRCS main.cpp StationController.cpp TurnoutController.cpp LEDController.cpp MAC.cpp VLCMAC.cpp VLCPHY.cpp BLE.cpp UART.cpp BOARD mega PORT /dev/ttyController)

target_compile_definitions(train PRIVATE TRAIN=1 TRAIN_ID=12)
target_compile_definitions(controller PRIVATE CONTROLLER=1)

generate_avr_firmware(eva-recv SRCS main.cpp LEDController.cpp MAC.cpp VLCMAC.cpp VLCPHY.cpp UART.cpp BOARD mega PORT /dev/ttyTrain)
generate_avr_firmware(eva-send SRCS main.cpp LEDController.cpp Clock.cpp MAC.cpp VLCMAC.cpp VLCPHY.cpp UART.cpp BOARD mega PORT /dev/ttyController)

target_compile_definitions(eva-recv PRIVATE EVA_RECV=1)
target_compile_definitions(eva-send PRIVATE EVA_SEND=1)
