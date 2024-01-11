cmake_minimum_required(VERSION 3.13)

project(tests C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
include(CTest)
include(../../src/files.cmake)

# CppUTest
include(FetchContent)
FetchContent_Declare(
    CppUTest
    GIT_REPOSITORY https://github.com/cpputest/cpputest.git
    GIT_TAG        master # or use release tag, eg. v4.0
)
FetchContent_MakeAvailable(CppUTest)

add_executable(tests
    test_main.cpp
    ../AdcSamplerTest.cpp
    ${SRC_FILES}
)

target_include_directories(tests PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/../../include"
)

target_link_libraries(tests PRIVATE
  CppUTest::CppUTest
  CppUTest::CppUTestExt
)

enable_testing()
add_test(NAME AdcSampler COMMAND ${CMAKE_CURRENT_LIST_DIR}/build/tests)