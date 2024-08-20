cmake_minimum_required(VERSION 3.14)

project(Test CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

enable_testing()

add_subdirectory(googletest)
include_directories(googletest/include)

add_executable(${PROJECT_NAME} test.cpp
    mock_board.cpp mock_board.h
    )

target_link_libraries(${PROJECT_NAME} PRIVATE gtest gmock)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})