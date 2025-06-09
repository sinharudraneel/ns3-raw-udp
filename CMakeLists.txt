cmake_minimum_required(VERSION 3.10)

project(NS3RawSocketUDP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ENABLE_PYTHON)
set(ENABLE_PYVIZ)

set(NS3_INCLUDE_PATH $ENV{NS3_INCLUDE_PATH})
if (NOT NS3_INCLUDE_PATH)
    set(NS3_INCLUDE_PATH "/usr/include/ns3.35/")
endif()

set (NS3_LIB_PATH $ENV{NS3_LIB_PATH})
if (NOT NS3_LIB_PATH)
    set(NS3_LIB_PATH "/usr/lib/x86_64-linux-gnu/")
endif()

include_directories(${NS3_INCLUDE_PATH})
include_directories(".")

file(GLOB NS3_LIBS "${NS3_LIB_PATH}/libns3*.so")

set(SOURCES
    RawApp.h
    RawApp.cc
    rawudpnet.cc
    simple-net-device-pcap.h
    simple-net-device-pcap.cc 
    simple-channel-pcap.h
    simple-channel-pcap.cc   
    simple-net-device-helper-pcap.h
    simple-net-device-helper-pcap.cc
)

add_executable(rawudpnet ${SOURCES})

target_link_libraries(rawudpnet ${NS3_LIBS})
add_definitions(-DNS3_LOG_ENABLE)