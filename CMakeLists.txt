cmake_minimum_required(VERSION 3.13)

include(cmake/pico_sdk_import.cmake)

project(pico_adc C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(src/files.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

pico_sdk_init()

enable_testing()

add_subdirectory(src)
add_subdirectory(example)
add_subdirectory(test/desktop EXCLUDE_FROM_ALL)
# add_subdirectory(test/device EXCLUDE_FROM_ALL)