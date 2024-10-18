cmake_minimum_required(VERSION 3.14)

project(Test CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${CMAKE_SOURCE_DIR}/singelton)
include_directories(${CMAKE_SOURCE_DIR}/pool-allocator)
include_directories(${CMAKE_SOURCE_DIR}/system_abstraction)

include("${CMAKE_SOURCE_DIR}/cpp_tests/rapidjson.cmake")
include_directories(${RAPIDJSON_INCLUDE_DIR})

include("${CMAKE_SOURCE_DIR}/cpp_tests/googletest.cmake")

file(GLOB_RECURSE RAPIDJSON_FILES     
    ${PROJECT_SOURCE_DIR}/cpp_tests/rapidjson/*.cc 
    ${PROJECT_SOURCE_DIR}/cpp_tests/rapidjson/*.cpp
)

set(TEST_FILES
    test.cpp
    mock_board.cpp
    mock_board.h
)

add_executable(${PROJECT_NAME} 
    ${RAPIDJSON_FILES}
    ${TEST_FILES}
)

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/cpp_tests/data/ ${PROJECT_BINARY_DIR}/data/
)

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory pages
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    GTest::GTest gtest gmock
    OnBoardComputer
)
add_dependencies(${PROJECT_NAME} rapidjson)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
