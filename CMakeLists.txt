cmake_minimum_required(VERSION 3.10)

project(NS3RawSocketUDP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("/usr/include/ns3.35/")
include_directories(".")

file(GLOB NS3_LIBS "/usr/lib/x86_64-linux-gnu/libns3*.so")

set(SOURCES
    RawApp.h
    RawApp.cc
    rawudpnet.cc    
)

add_executable(raw-app ${SOURCES})

target_link_libraries(raw-app ${NS3_LIBS})
add_definitions(-DNS3_LOG_ENABLE)