cmake_minimum_required(VERSION 2.8)
project( AtVsidoConnectLibrary )

add_library(libAtVsidoConnect)
target_sources(libAtVsidoConnect
    PRIVATE
    At_Vsido_Connect_Library.cpp
)

add_executable(test test/test.cpp test/vsido_connect_library_test.cpp)
target_include_directories(test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test
    PRIVATE
    libAtVsidoConnect
    gtest
    gtest_main
    gmock  #今回は使わないけど
    pthread)