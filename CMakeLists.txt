cmake_minimum_required(VERSION 3.14)

project(OnBoardComputor CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(cpp_tests)

include(CTest)