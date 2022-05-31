cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_COMPILER_FORCED 1)
project(AtVsidoConnectLibrary CXX)


add_library(lib${PROJECT_NAME})
target_sources(lib${PROJECT_NAME}
    PRIVATE
    At_Vsido_Connect_Sender.cpp    
    At_Vsido_Connect_Library.cpp
)

add_executable(test test/test.cpp test/vsido_connect_library_test.cpp)
target_include_directories(test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test
    PRIVATE
    lib${PROJECT_NAME}
    gtest
    gtest_main
    gmock
    pthread)