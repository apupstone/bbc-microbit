cmake_minimum_required(VERSION 3.1...3.13)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(BLE_Demo  VERSION 1.0
                  DESCRIPTION "BBC Microbit BLE Demo"
                  LANGUAGES C CXX)
				  
				  

add_executable(one two.cpp three.h)