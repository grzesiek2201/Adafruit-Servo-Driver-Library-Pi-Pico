# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.14)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

set(PICO_BOARD pico_w)

# Set name of project (as PROJECT_NAME) and C/CXX standards
project(servo_driver C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

# Tell CMake where to find the executable source file
add_executable(${PROJECT_NAME}
    simple_servo.cpp
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

add_subdirectory(Adafruit-Servo-Driver-Library-Pi-Pico)

target_include_directories(${PROJECT_NAME}
    PRIVATE Adafruit-Servo-Driver-Library-Pi-Pico
)

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME}
    pico_cyw43_arch_none
	Adafruit-Servo-Driver-Library-Pi-Pico
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
