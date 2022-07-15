cmake_minimum_required(VERSION 3.1)
project(carto_mobile_sdk)

# General options
option(SINGLE_LIBRARY "Compile as single library" OFF)

if(APPLE)
  option(INCLUDE_OBJC "Include ObjC code on iOS" OFF)
  option(SHARED_LIBRARY "Build as shared library on iOS" OFF)
endif()

# Directories
set(SDK_BASE_DIR "${PROJECT_SOURCE_DIR}/../..")
set(SDK_SRC_DIR "${SDK_BASE_DIR}/all/native")
set(SDK_CARTO_LIBS_DIR "${SDK_BASE_DIR}/libs-carto" )
set(SDK_EXTERNAL_LIBS_DIR "${SDK_BASE_DIR}/libs-external")

# General platform-specific flags
if(WIN32 OR ANDROID)
  set(SDK_LIBRARY_TYPE SHARED)
elseif(SHARED_LIBRARY)
  set(SDK_LIBRARY_TYPE SHARED)
elseif(NOT SHARED_LIBRARY)
  set(SDK_LIBRARY_TYPE STATIC)
endif()

# C++ compiler flags
add_definitions("${SDK_CPP_DEFINES}" "-DBOOST_ALL_NO_LIB")

if(CMAKE_BUILD_TYPE MATCHES "Release|RELEASE")
  add_definitions("-DNDEBUG")
endif()

if(WIN32)
  add_definitions("-DNOMINMAX -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DGL_GLEXT_PROTOTYPES")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4244")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /std:c++17 /Zc:__cplusplus /EHs /GR")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Os /GL")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Os /GL")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /WINMD vccorlib.lib")
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG")

  set(CMAKE_VS_WINRT_EXTENSIONS ON)
endif(WIN32)

if(APPLE AND (NOT IOS))
  execute_process(COMMAND "xcodebuild" "-version" "-sdk" "macosx" "Path" OUTPUT_VARIABLE OSX_SDK_PATH ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

  add_compile_options("-target" "${SDK_IOS_ARCH}-apple-ios-13.0-macabi" "-iframework" "${OSX_SDK_PATH}/System/iOSSupport/System/Library/Frameworks")
  add_compile_options("-miphoneos-version-min=13.0")

  set(CMAKE_XCODE_EFFECTIVE_PLATFORMS "-maccatalyst")
endif()

if(APPLE)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fobjc-arc -fmodules -fvisibility=hidden")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc -fmodules -stdlib=libc++ -std=c++17 -ftemplate-depth=1024 -fexceptions -frtti -fvisibility=hidden -fvisibility-inlines-hidden -fno-aligned-allocation")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os")
  set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -ObjC")

  set(CMAKE_XCODE_ATTRIBUTE_CLANG_ENABLE_CODE_COVERAGE NO)
  set(CMAKE_XCODE_ATTRIBUTE_GCC_INSTRUMENT_PROGRAM_FLOW_ARCS NO)

  if(SDK_CPP_DEFINES MATCHES ".*_CARTO_USE_METALANGLE.*")
    set(CMAKE_XCODE_ATTRIBUTE_PRELINK_LIBS "${SDK_EXTERNAL_LIBS_DIR}/angle-metal/${SDK_IOS_BASEARCH}/libangle.a")
  endif()

  if(SDK_DEV_TEAM)
    set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${SDK_DEV_TEAM})
  endif()

  if(SHARED_LIBRARY)
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -flto=full")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto=full")
  endif()

  if(CMAKE_BUILD_TYPE MATCHES "Release|RELEASE")
    set(CMAKE_XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING YES)
    set(CMAKE_XCODE_ATTRIBUTE_DEAD_CODE_STRIPPING YES)
    set(CMAKE_XCODE_ATTRIBUTE_SEPARATE_STRIP YES)
    set(CMAKE_XCODE_ATTRIBUTE_STRIP_STYLE "non-global")
    set(CMAKE_XCODE_ATTRIBUTE_GENERATE_MASTER_OBJECT_FILE YES)
  endif()
endif(APPLE)

if(ANDROID)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -ftemplate-depth=1024 -fexceptions -frtti -fvisibility=hidden -fvisibility-inlines-hidden")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os -flto=full -fomit-frame-pointer")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os -flto=full -fomit-frame-pointer")
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -s -fuse-ld=lld -Wl,-plugin-opt=O3 -Wl,--gc-sections -Wl,-icf=all -Wl,--threads=4 -Wl,--as-needed -Wl,--build-id -Wl,--version-script=${PROJECT_SOURCE_DIR}/../android/version-script")
endif(ANDROID)

# Make common libraries available to all subprojects
include_directories("${SDK_EXTERNAL_LIBS_DIR}/boost")

if(APPLE)
  include_directories("${SDK_BASE_DIR}/ios/stdlib")
  if(SDK_CPP_DEFINES MATCHES ".*_CARTO_USE_METALANGLE.*")
    include_directories("${SDK_EXTERNAL_LIBS_DIR}/angle-metal/include")
  else()
    include_directories("${SDK_BASE_DIR}/ios/glwrapper")
  endif()
endif(APPLE)

if(WIN32)
  include_directories("${SDK_EXTERNAL_LIBS_DIR}/angle-uwp/include" "${SDK_EXTERNAL_LIBS_DIR}/zlib/config" "${SDK_EXTERNAL_LIBS_DIR}/zlib/zlib")
endif(WIN32)

# Subprojects
set(SDK_CARTO_SUBPROJECTS vt mapnikvt mbvtbuilder cartocss nml)
set(SDK_EXTERNAL_SUBPROJECTS botan pugixml bidi freetype harfbuzz brotli miniz libjpeg libpng libwebp tess2 pvrt rg_etc1)

if(WIN32)
  list(APPEND SDK_EXTERNAL_SUBPROJECTS "zlib")
endif(WIN32)

if(NOT (WIN32 OR APPLE OR ANDROID))
  list(APPEND SDK_EXTERNAL_SUBPROJECTS "pion")
endif()

if(INCLUDE_GEOCODING)
  list(APPEND SDK_CARTO_SUBPROJECTS "geocoding")
endif(INCLUDE_GEOCODING)

if(INCLUDE_ROUTING)
  list(APPEND SDK_CARTO_SUBPROJECTS "osrm" "sgre")
endif(INCLUDE_ROUTING)

if(INCLUDE_SQLITE)
  list(APPEND SDK_EXTERNAL_SUBPROJECTS "sqlite" "sqlite3pp")
endif(INCLUDE_SQLITE)

if(INCLUDE_VALHALLA)
  list(APPEND SDK_EXTERNAL_SUBPROJECTS "date" "protobuf" "valhalla")
endif(INCLUDE_VALHALLA)

foreach(SDK_SUBPROJECT IN ITEMS ${SDK_CARTO_SUBPROJECTS})
  add_subdirectory("${SDK_CARTO_LIBS_DIR}/${SDK_SUBPROJECT}" ${SDK_SUBPROJECT})
endforeach()

foreach(SDK_SUBPROJECT IN ITEMS ${SDK_EXTERNAL_SUBPROJECTS})
  add_subdirectory("${SDK_EXTERNAL_LIBS_DIR}/${SDK_SUBPROJECT}" ${SDK_SUBPROJECT})
endforeach()

set(SDK_SUBPROJECTS "${SDK_CARTO_SUBPROJECTS}" "${SDK_EXTERNAL_SUBPROJECTS}")

# When in single library mode, apply compilation flags to individual projects.
if(SINGLE_LIBRARY)
  foreach(SDK_SUBPROJECT IN ITEMS ${SDK_SUBPROJECTS})
    set(SUBPROJECT_FLAGS "${${SDK_SUBPROJECT}_FLAGS}")
    if(CMAKE_BUILD_TYPE MATCHES "Release|RELEASE")
      if(ANDROID OR APPLE)
        if(SDK_SUBPROJECT MATCHES "^(vt|mapnikvt|cartocss|freetype|tess2|zlib|brotli)$")
          set(SUBPROJECT_FLAGS ${SUBPROJECT_FLAGS} "-O2")
        else()
          set(SUBPROJECT_FLAGS ${SUBPROJECT_FLAGS} "-Oz")
        endif()
      elseif(WIN32)
        if(SDK_SUBPROJECT MATCHES "^(vt|mapnikvt|cartocss|freetype|tess2|zlib|brotli)$")
          set(SUBPROJECT_FLAGS ${SUBPROJECT_FLAGS} "/Ox")
        else()
          set(SUBPROJECT_FLAGS ${SUBPROJECT_FLAGS} "/Os")
        endif()
      endif()
    endif()
    if(SUBPROJECT_FLAGS)
      set_source_files_properties(${${SDK_SUBPROJECT}_SRC_FILES} PROPERTIES COMPILE_OPTIONS "${SUBPROJECT_FLAGS}")
    endif()
  endforeach()
endif(SINGLE_LIBRARY)

# Source files and properties
file(GLOB SDK_SRC_FILES
  "${SDK_SRC_DIR}/*/*.cpp"
  "${SDK_SRC_DIR}/*/*/*.cpp"
  "${SDK_SRC_DIR}/*/*.h"
  "${SDK_SRC_DIR}/*/*/*.h"
)
set_source_files_properties("${SDK_SRC_DIR}/utils/PlatformUtils.cpp" PROPERTIES COMPILE_OPTIONS "-D_CARTO_MOBILE_SDK_PLATFORM=\"${SDK_PLATFORM}\";-D_CARTO_MOBILE_SDK_VERSION=\"${SDK_VERSION}\"")

if(WIN32 OR APPLE OR ANDROID)
  list(REMOVE_ITEM SDK_SRC_FILES "${SDK_SRC_DIR}/network/HTTPClientPionImpl.cpp")
endif()

if(ANDROID)
  file(GLOB SDK_ANDROID_SRC_FILES
    "${WRAPPER_DIR}/*.cpp"
    "${WRAPPER_DIR}/*.h"
    "${SDK_BASE_DIR}/android/native/*/*.cpp"
    "${SDK_BASE_DIR}/android/native/*/*.h"
    "${SDK_BASE_DIR}/android/native/*/*/*.cpp"
    "${SDK_BASE_DIR}/android/native/*/*/*.h"
  )
  set(SDK_SRC_FILES "${SDK_SRC_FILES}" "${SDK_ANDROID_SRC_FILES}")
elseif(APPLE)
  if(INCLUDE_OBJC)
    file(GLOB SDK_OBJC_SRC_FILES
      "${SDK_BASE_DIR}/ios/objc/*/*.cpp"
      "${SDK_BASE_DIR}/ios/objc/*/*.mm"
      "${SDK_BASE_DIR}/ios/objc/*/*.h"
      "${SDK_BASE_DIR}/ios/objc/*.h"
    )
    set(SDK_SRC_FILES "${SDK_SRC_FILES}" "${SDK_OBJC_SRC_FILES}")
  endif(INCLUDE_OBJC)
  file(GLOB SDK_IOS_SRC_FILES
    "${WRAPPER_DIR}/*.cpp"
    "${WRAPPER_DIR}/*.mm"
    "${WRAPPER_DIR}/*.h"
    "${SDK_BASE_DIR}/ios/native/*/*.cpp"
    "${SDK_BASE_DIR}/ios/native/*/*.mm"
    "${SDK_BASE_DIR}/ios/native/*/*.h"
  )
  set(SDK_SRC_FILES "${SDK_SRC_FILES}" "${SDK_IOS_SRC_FILES}")
elseif(WIN32)
  file(GLOB SDK_WINRT_SRC_FILES
    "${WRAPPER_DIR}/*.cpp"
    "${WRAPPER_DIR}/*.h"
    "${SDK_BASE_DIR}/winphone/native/*/*.cpp"
    "${SDK_BASE_DIR}/winphone/native/*/*.h"
    "${SDK_BASE_DIR}/winphone/native/*/*/*.cpp"
    "${SDK_BASE_DIR}/winphone/native/*/*/*.h"
  )
  set(SDK_SRC_FILES "${SDK_SRC_FILES}" "${SDK_WINRT_SRC_FILES}")

  foreach(WINRT_SRC_FILE IN ITEMS "utils/AssetUtils.cpp" "utils/PlatformUtils.cpp" "utils/EGLContextWrapper.cpp" "components/Task.cpp")
    set_source_files_properties("${SDK_BASE_DIR}/winphone/native/${WINRT_SRC_FILE}" PROPERTIES COMPILE_OPTIONS "/ZW")
  endforeach()
endif()

# Group wrapper files into different groups, to reduce clutter
source_group("Wrapper Sources" REGULAR_EXPRESSION ".*_wrap.cpp")
source_group("Wrapper Headers" REGULAR_EXPRESSION ".*_wrap.h")

# Include directories for main SDK project
include_directories("${SDK_SRC_DIR}")

foreach(SDK_HEADER_LIB IN ITEMS "cglib" "stdext" "picojson" "pbf" "rapidjson/include" "tinyformat" "nanosvg/src" "utf8/source")
  include_directories("${SDK_EXTERNAL_LIBS_DIR}/${SDK_HEADER_LIB}")
endforeach()

foreach(SDK_SUBPROJECT IN ITEMS ${SDK_SUBPROJECTS})
  set(SUBPROJECT_INCLUDE_DIRS ${${SDK_SUBPROJECT}_INCLUDE_DIRS})
  if(SUBPROJECT_INCLUDE_DIRS)
    include_directories("${SUBPROJECT_INCLUDE_DIRS}")
  endif()
endforeach()

if(ANDROID)
  include_directories("${SDK_BASE_DIR}/android/native")
elseif(APPLE)
  include_directories("${SDK_BASE_DIR}/ios/native")
  if(INCLUDE_OBJC)
    include_directories("${SDK_BASE_DIR}/generated/ios-objc/proxies")
    include_directories("${SDK_BASE_DIR}/generated/ios-objc/wrappers")
    include_directories("${SDK_BASE_DIR}/ios/objc")
  endif(INCLUDE_OBJC)
elseif(WIN32)
  include_directories("${SDK_BASE_DIR}/winphone/native")
endif()

# Linking
set(SDK_OBJECTS "")
if(SINGLE_LIBRARY)
  foreach(SDK_SUBPROJECT IN ITEMS ${SDK_SUBPROJECTS})
    set(SDK_SRC_FILES "${SDK_SRC_FILES}" "${${SDK_SUBPROJECT}_SRC_FILES}")
  endforeach()
else()
  foreach(SDK_SUBPROJECT IN ITEMS ${SDK_SUBPROJECTS})
    set(SDK_OBJECTS "${SDK_OBJECTS}" "$<TARGET_OBJECTS:${SDK_SUBPROJECT}>")
  endforeach()
endif()

add_library(carto_mobile_sdk ${SDK_LIBRARY_TYPE} ${SDK_SRC_FILES} ${SDK_OBJECTS})

if(ANDROID)
  target_link_libraries(carto_mobile_sdk EGL GLESv2 z log android jnigraphics)
elseif(APPLE)
  target_link_libraries(carto_mobile_sdk "-framework Foundation" "-framework CoreGraphics" "-framework CoreText" "-framework CFNetwork" "-framework UIKit")
  if(SDK_CPP_DEFINES MATCHES ".*_CARTO_USE_METALANGLE.*")
    target_link_libraries(carto_mobile_sdk "-weak_framework OpenGLES" "-weak_framework Metal" "-framework QuartzCore")
  else()
    target_link_libraries(carto_mobile_sdk "-framework OpenGLES" "-framework GLKit")
  endif()
elseif(WIN32)
  target_link_libraries(carto_mobile_sdk "${SDK_EXTERNAL_LIBS_DIR}/angle-uwp/${SDK_WINPHONE_ARCH}/libEGL.dll.lib" "${SDK_EXTERNAL_LIBS_DIR}/angle-uwp/${SDK_WINPHONE_ARCH}/libGLESv2.dll.lib")
  target_link_libraries(carto_mobile_sdk "msxml6.lib" "d3d11.lib" "dwrite.lib" "d2d1.lib")
endif()
