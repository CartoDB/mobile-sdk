cmake_minimum_required(VERSION 2.8)

find_package(Java REQUIRED)
include(UseJava)

project(carto_mobile_sdk_java)

set(ANDROID_SDK_DIR "/Applications/android-sdk-macosx")
set(ANDROID_NDK "/Applications/android-ndk-r10e")
set(ANDROID_NATIVE_API_LEVEL "android-9")

set(ANDROID_API_VER 8)
set(DIST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../dist/androidx")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../all/native" carto_mobile_sdk)

file(GLOB_RECURSE JAVA_FILES1 "${CMAKE_CURRENT_SOURCE_DIR}/../../android/generated/proxies/*.java")
file(GLOB_RECURSE JAVA_FILES2 "${CMAKE_CURRENT_SOURCE_DIR}/../../android/java/*.java")

set(CMAKE_JAVA_INCLUDE_PATH "${ANDROID_SDK_DIR}/platforms/android-${ANDROID_API_VER}/android.jar")

add_jar(carto_mobile_sdk_java SOURCES ${JAVA_FILES1} ${JAVA_FILES2} OUTPUT_NAME "nutiteq-maps-sdk")

add_custom_command(TARGET carto_mobile_sdk_java POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${DIST_DIR}")
add_custom_command(TARGET carto_mobile_sdk_java POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:carto_mobile_sdk> "${DIST_DIR}")
add_custom_command(TARGET carto_mobile_sdk_java POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "nutiteq-maps-sdk.jar" "${DIST_DIR}")
