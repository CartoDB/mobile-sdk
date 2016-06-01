cmake_minimum_required(VERSION 2.6)
project(cartocss)

if(WIN32)
add_definitions("-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DBOOST_VARIANT_DO_NOT_USE_VARIADIC_TEMPLATES")
endif(WIN32)

if(APPLE)
add_definitions("-std=c++11 -Wno-unsequenced")
endif(APPLE)

if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") 
endif(CMAKE_COMPILER_IS_GNUCXX)

set(cartocss_SRC_DIR "${PROJECT_SOURCE_DIR}/src/cartocss")
set(cartocss_LIBS_DIR "${PROJECT_SOURCE_DIR}/../libs-external")

file(GLOB cartocss_SRC_FILES "${cartocss_SRC_DIR}/*.cpp" "${cartocss_SRC_DIR}/*.h")

if(WIN32)
set_source_files_properties("${cartocss_SRC_DIR}/CartoCSS/CartoCSSParser.cpp" PROPERTIES COMPILE_FLAGS "/Od /Ob2")
endif(WIN32)

if(SINGLE_LIBRARY)
add_library(cartocss INTERFACE)
set(cartocss_SRC_FILES ${cartocss_SRC_FILES} PARENT_SCOPE)
else()
add_library(cartocss OBJECT ${cartocss_SRC_FILES})
endif()
