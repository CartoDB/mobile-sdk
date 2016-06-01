cmake_minimum_required(VERSION 2.6)
project(nml)

if(WIN32)
add_definitions("-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS")
endif(WIN32)

if(APPLE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") 
endif(APPLE)

if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") 
endif(CMAKE_COMPILER_IS_GNUCXX)

set(nml_SRC_DIR "${PROJECT_SOURCE_DIR}/src/nml")
set(nml_LIBS_DIR "${PROJECT_SOURCE_DIR}/../libs-external")

file(GLOB nml_SRC_FILES "${nml_SRC_DIR}/*.cpp" "${nml_SRC_DIR}/*.h")

if(SINGLE_LIBRARY)
add_library(nml INTERFACE)
set(nml_SRC_FILES ${nml_SRC_FILES} PARENT_SCOPE)
else()
add_library(nml OBJECT ${nml_SRC_FILES})
endif()
