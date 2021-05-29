cmake_minimum_required(VERSION 3.1)
project(carto_mobile_sdk)

# General options
option(SINGLE_LIBRARY "Compile as single library" OFF)

if(IOS)
  option(INCLUDE_OBJC "Include ObjC code on iOS" OFF)
  option(ENABLE_BITCODE "Enable bitcode support on iOS" ON)
  option(SHARED_LIBRARY "Build as shared library on iOS" OFF)
endif(IOS)

# General platform-specific flags
if(WIN32 OR ANDROID)
  set(SDK_LIBRARY_TYPE SHARED)
elseif(SHARED_LIBRARY)
  set(SDK_LIBRARY_TYPE SHARED)
elseif(NOT SHARED_LIBRARY)
  set(SDK_LIBRARY_TYPE STATIC)
endif()

# C++ compiler flags
add_definitions("${SDK_CPP_DEFINES} -DBOOST_ALL_NO_LIB")

if(CMAKE_BUILD_TYPE MATCHES "Release|RELEASE")
  add_definitions("-DNDEBUG")
endif()

if(WIN32)
  add_definitions("-DNOMINMAX -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DGL_GLEXT_PROTOTYPES")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4244")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /std:c++17 /Zc:__cplusplus /EHs /GR")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Os")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Os")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /WINMD /NODEFAULTLIB:vccorlib /NODEFAULTLIB:msvcrt vccorlib.lib msvcrt.lib")

  set(CMAKE_VS_WINRT_EXTENSIONS ON)
  set(VS_WINRT_EXTENSIONS ON)
endif(WIN32)

if(IOS)
  add_definitions("-DCRYPTOPP_NO_CXX17")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fobjc-arc -fmodules -fvisibility=hidden")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc -fmodules -stdlib=libc++ -std=c++17 -ftemplate-depth=1024 -fexceptions -frtti -fvisibility=hidden -fvisibility-inlines-hidden -fno-aligned-allocation")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os")
  set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -ObjC")

  set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH YES)

  if(ENABLE_BITCODE)
    set(CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE YES)
  else()
    set(CMAKE_XCODE_ATTRIBUTE_ENABLE_BITCODE NO)
  endif()

  if(CMAKE_BUILD_TYPE MATCHES "Release|RELEASE")
    set(CMAKE_XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING YES)
    set(CMAKE_XCODE_ATTRIBUTE_DEAD_CODE_STRIPPING YES)
    set(CMAKE_XCODE_ATTRIBUTE_SEPARATE_STRIP YES)
    set(CMAKE_XCODE_ATTRIBUTE_STRIP_STYLE "non-global")
    set(CMAKE_XCODE_ATTRIBUTE_GENERATE_MASTER_OBJECT_FILE YES)
  endif()
endif(IOS)

if(ANDROID)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -ftemplate-depth=1024 -fexceptions -frtti -fvisibility=hidden -fvisibility-inlines-hidden")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os -flto=full -fomit-frame-pointer")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os -flto=full -fomit-frame-pointer")
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -Wl,-plugin-opt=O2 -s -Wl,--gc-sections -Wl,--as-needed -fuse-ld=lld -Wl,--version-script=${PROJECT_SOURCE_DIR}/../android/version-script")

  if(CMAKE_BUILD_TYPE MATCHES "Release|RELEASE")
    string(REPLACE "-Wa,--noexecstack" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REPLACE "-Wa,--noexecstack" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  endif()
endif(ANDROID)

# Directories
set(SDK_BASE_DIR "${PROJECT_SOURCE_DIR}/../..")
set(SDK_SRC_DIR "${SDK_BASE_DIR}/all/native")
set(SDK_CARTO_LIBS_DIR "${SDK_BASE_DIR}/libs-carto" )
set(SDK_EXTERNAL_LIBS_DIR "${SDK_BASE_DIR}/libs-external")

# Make common libraries available to all subprojects
include_directories("${SDK_EXTERNAL_LIBS_DIR}/boost")

if(IOS)
  include_directories("${SDK_BASE_DIR}/ios/stdlib")
  if(SDK_CPP_DEFINES MATCHES ".*_CARTO_USE_METALANGLE.*")
    include_directories("${SDK_EXTERNAL_LIBS_DIR}/angle-metal/include")
  else()
    include_directories("${SDK_BASE_DIR}/ios/glwrapper")
  endif()
endif(IOS)

if(WIN32)
  include_directories("${SDK_EXTERNAL_LIBS_DIR}/angle-uwp/include" "${SDK_EXTERNAL_LIBS_DIR}/zlib/config" "${SDK_EXTERNAL_LIBS_DIR}/zlib/zlib")
endif(WIN32)

# Subprojects
set(SDK_CARTO_SUBPROJECTS vt mapnikvt mbvtbuilder cartocss nml osrm sgre geocoding)
set(SDK_EXTERNAL_SUBPROJECTS cryptopp pugixml sqlite sqlite3pp bidi freetype harfbuzz brotli miniz libjpeg libpng libwebp tess2 pvrt rg_etc1)

if(ANDROID)
  set(SDK_EXTERNAL_SUBPROJECTS "${SDK_EXTERNAL_SUBPROJECTS}" "androidcpufeatures")
endif(ANDROID)

if(WIN32)
  list(APPEND SDK_EXTERNAL_SUBPROJECTS "zlib")
endif(WIN32)

if(NOT (WIN32 OR IOS OR ANDROID))
  list(APPEND SDK_EXTERNAL_SUBPROJECTS "pion")
endif(NOT (WIN32 OR IOS OR ANDROID))

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

if(SINGLE_LIBRARY)
  foreach(SDK_SUBPROJECT IN ITEMS ${SDK_SUBPROJECTS})
    set(SUBPROJECT_FLAGS "${${SDK_SUBPROJECT}_FLAGS}")
    if (SUBPROJECT_FLAGS)
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

if(WIN32 OR IOS OR ANDROID)
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
elseif(IOS)
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

foreach (SDK_HEADER_LIB IN ITEMS "cglib" "stdext" "picojson" "pbf" "rapidjson/include" "tinyformat" "nanosvg/src" "utf8/source")
  include_directories("${SDK_EXTERNAL_LIBS_DIR}/${SDK_HEADER_LIB}")
endforeach()

foreach(SDK_SUBPROJECT IN ITEMS ${SDK_SUBPROJECTS})
  set(SUBPROJECT_INCLUDE_DIRS ${${SDK_SUBPROJECT}_INCLUDE_DIRS})
  if (SUBPROJECT_INCLUDE_DIRS)
    include_directories("${SUBPROJECT_INCLUDE_DIRS}")
  endif()
endforeach()

if(ANDROID)
  include_directories("${SDK_BASE_DIR}/android/native")
elseif(IOS)
  include_directories("${SDK_BASE_DIR}/ios/native")
  if(INCLUDE_OBJC)
    include_directories("${SDK_BASE_DIR}/generated/ios-objc/proxies")
    include_directories("${SDK_BASE_DIR}/generated/ios-objc/wrappers")
    include_directories("${SDK_BASE_DIR}/ios/objc")
  endif(INCLUDE_OBJC)
elseif(WIN32)
  link_directories("${SDK_EXTERNAL_LIBS_DIR}/angle-uwp/${WINPHONE_ARCH}")
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
elseif(IOS)
  if(SDK_CPP_DEFINES MATCHES ".*_CARTO_USE_METALANGLE.*")
    target_link_libraries(carto_mobile_sdk "-framework Foundation" "-framework CoreGraphics" "-framework CoreText" "-framework Metal" "-framework MetalKit" "-framework UIKit" "-framework CFNetwork")
  else()
    target_link_libraries(carto_mobile_sdk "-framework Foundation" "-framework CoreGraphics" "-framework CoreText" "-framework OpenGLES" "-framework GLKit" "-framework UIKit" "-framework CFNetwork")
  endif()
elseif(WIN32)
  target_link_libraries(carto_mobile_sdk msxml6.lib d3d11.lib dwrite.lib d2d1.lib libEGL.dll.lib libGLESv2.dll.lib)
endif()
