cmake_minimum_required(VERSION 3.13)

include(../cmake/pico_sdk_import.cmake)

set(PROJECT_NAME "pico_adc_example")
project(${PROJECT_NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(../src/files.cmake)

project(${PROJECT_NAME} C CXX ASM)

add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(${PROJECT_NAME}
    pico_adc
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_add_extra_outputs(${PROJECT_NAME})