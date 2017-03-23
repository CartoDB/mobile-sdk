cmake_minimum_required(VERSION 2.6)
project(geocoding)

if(WIN32)
add_definitions("-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS")
endif(WIN32)

if(APPLE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") 
endif(APPLE)

if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") 
endif(CMAKE_COMPILER_IS_GNUCXX)

set(geocoding_SRC_DIR "${PROJECT_SOURCE_DIR}/src/geocoding")
set(geocoding_LIBS_DIR "${PROJECT_SOURCE_DIR}/../libs-external")

file(GLOB geocoding_SRC_FILES "${geocoding_SRC_DIR}/*.cpp" "${geocoding_SRC_DIR}/*.h")

if(SINGLE_LIBRARY)
add_library(geocoding INTERFACE)
set(geocoding_SRC_FILES ${geocoding_SRC_FILES} PARENT_SCOPE)
else()
add_library(geocoding OBJECT ${geocoding_SRC_FILES})
endif()
