cmake_minimum_required(VERSION 3.27)

project(OnBoardComputer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

enable_testing()

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE .)

add_subdirectory(singelton)
add_subdirectory(cpp_tests)

include(CTest)