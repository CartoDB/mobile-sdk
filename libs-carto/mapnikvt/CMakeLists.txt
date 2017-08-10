cmake_minimum_required(VERSION 2.6)
project(mapnikvt)

if(WIN32)
add_definitions("-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS")
endif(WIN32)

if(APPLE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif(APPLE)

if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") 
endif(CMAKE_COMPILER_IS_GNUCXX)

set(mapnikvt_SRC_DIR "${PROJECT_SOURCE_DIR}/src/mapnikvt")
set(mapnikvt_LIBS_DIR "${PROJECT_SOURCE_DIR}/../libs")

file(GLOB mapnikvt_SRC_FILES "${mapnikvt_SRC_DIR}/*.cpp" "${mapnikvt_SRC_DIR}/*.h" "${mapnikvt_SRC_DIR}/MBVTPackage/*.cc" "${mapnikvt_SRC_DIR}/MBVTPackage/*.h")

if(SINGLE_LIBRARY)
add_library(mapnikvt INTERFACE)
set(mapnikvt_SRC_FILES ${mapnikvt_SRC_FILES} PARENT_SCOPE)
else()
add_library(mapnikvt OBJECT ${mapnikvt_SRC_FILES})
endif()
