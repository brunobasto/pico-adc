cmake_minimum_required(VERSION 3.13)

include(../cmake/pico_sdk_import.cmake)

project(pico_adc C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(files.cmake)

pico_sdk_init()

add_library(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_adc
    hardware_dma
    hardware_irq
)