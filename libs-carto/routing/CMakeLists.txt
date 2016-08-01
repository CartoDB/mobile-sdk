cmake_minimum_required(VERSION 2.6)
project(routing)

if(WIN32)
add_definitions("-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS")
endif(WIN32)

if(APPLE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") 
endif(APPLE)

if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") 
endif(CMAKE_COMPILER_IS_GNUCXX)

set(routing_SRC_DIR "${PROJECT_SOURCE_DIR}/src/routing")
set(routing_LIBS_DIR "${PROJECT_SOURCE_DIR}/../libs-external")

file(GLOB routing_SRC_FILES "${routing_SRC_DIR}/*.cpp" "${routing_SRC_DIR}/*.h")

if(SINGLE_LIBRARY)
add_library(routing INTERFACE)
set(routing_SRC_FILES ${routing_SRC_FILES} PARENT_SCOPE)
else()
add_library(routing OBJECT ${routing_SRC_FILES})
endif()
