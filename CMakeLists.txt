cmake_minimum_required(VERSION 3.13)

include(cmake/pico_sdk_import.cmake)

project(pico_adc C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

pico_sdk_init()

add_library(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AdcSampler.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_adc
    hardware_dma
    hardware_irq
)

## Example

project(pico_adc_example C CXX ASM)

add_executable(pico_adc_example
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

target_link_libraries(pico_adc_example
    pico_adc
)

pico_enable_stdio_usb(pico_adc_example 1)
pico_enable_stdio_uart(pico_adc_example 0)
pico_add_extra_outputs(pico_adc_example)