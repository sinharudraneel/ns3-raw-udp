cmake_minimum_required(VERSION 3.10)

project(NS3RawSocketUDP)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ENABLE_PYTHON)
set(ENABLE_PYVIZ)   

if (NOT ${MANUAL})
    message("actually here")
    set(NS3_INCLUDE_PATH $ENV{NS3_INCLUDE_PATH})
    if (NOT NS3_INCLUDE_PATH)
        set(NS3_INCLUDE_PATH "/usr/include/ns3.35/")
    endif()

    set (NS3_LIB_PATH $ENV{NS3_LIB_PATH})
    if (NOT NS3_LIB_PATH)
        set(NS3_LIB_PATH "/usr/lib/x86_64-linux-gnu/")
    endif()
else()
    message("here")
    set(NS3_INCLUDE_PATH $ENV{NS3_INCLUDE_PATH})
    if (NOT NS3_INCLUDE_PATH)
        set(NS3_INCLUDE_PATH "/home/rudraneel.s/ns-3-dev-udpnet/build/include/")
    endif()

    set (NS3_LIB_PATH $ENV{NS3_LIB_PATH})
    if (NOT NS3_LIB_PATH)
        set(NS3_LIB_PATH "/home/rudraneel.s/ns-3-dev-udpnet/build/lib/")
    endif()
endif()

include_directories(${NS3_INCLUDE_PATH})
include_directories(".")

file(GLOB NS3_LIBS "${NS3_LIB_PATH}/libns3*.so")

set(SOURCES
    RawApp.h
    RawApp.cc
    rawudpnet.cc    
)

add_executable(rawudpnet ${SOURCES})

target_link_libraries(rawudpnet ${NS3_LIBS})
add_definitions(-DNS3_LOG_ENABLE)
add_definitions(-DNS3_PYTHON_BINDINGS)