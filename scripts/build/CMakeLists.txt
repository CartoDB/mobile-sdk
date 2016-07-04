cmake_minimum_required(VERSION 3.1)
project(carto_mobile_sdk)

# General options
option(SINGLE_LIBRARY "Compile as single library" OFF)
option(INCLUDE_GDAL "Link with GDAL" OFF)
option(INCLUDE_OBJC "Include ObjC code on iOS" OFF)

if(IOS)
option(ENABLE_BITCODE "Enable bitcode support" ON)
endif(IOS)

# General platform-specific flags
if(WIN32)
set(CMAKE_VS_WINRT_EXTENSIONS ON)
set(VS_WINRT_EXTENSIONS ON)
endif(WIN32)

if(ANDROID)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s")
endif(ANDROID)

if(WIN32 OR ANDROID)
set(SDK_LIBRARY_TYPE SHARED)
else(WIN32 OR ANDROID)
set(SDK_LIBRARY_TYPE STATIC)
endif()

# C++ compiler flags
add_definitions("${SDK_CPP_DEFINES} -D_CARTO_MOBILE_SDK_PLATFORM=\"\\\"${SDK_PLATFORM}\\\"\" -D_CARTO_MOBILE_SDK_VERSION=\"\\\"${SDK_VERSION}\\\"\" -DHAVE_OT -DHAVE_UCDN -DHB_NO_MT -DFT2_BUILD_LIBRARY")
if(WIN32)
add_definitions("-DNOMINMAX -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -DBOOST_VARIANT_DO_NOT_USE_VARIADIC_TEMPLATES -DBOOST_MSVC_ENABLE_2014_JUN_CTP -DRAPIDJSON_ENDIAN=RAPIDJSON_LITTLEENDIAN -DSQLITE_OS_WINRT -DSQLITE_OMIT_WAL -DNO_OS_DEPENDENCE -DGL_GLEXT_PROTOTYPES")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /WINMD /NODEFAULTLIB:vccorlib /NODEFAULTLIB:msvcrt vccorlib.lib msvcrt.lib")
endif(WIN32)

if(IOS)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ftemplate-depth=1024") 
endif(IOS)

if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ftemplate-depth=1024") 
endif(CMAKE_COMPILER_IS_GNUCXX)

# Directories
set(SDK_BASE_DIR "${PROJECT_SOURCE_DIR}/../..")
set(SDK_SRC_DIR "${SDK_BASE_DIR}/all/native")
set(SDK_LIBS_DIR "${SDK_BASE_DIR}/libs" )
set(SDK_EXTERNAL_LIBS_DIR "${SDK_BASE_DIR}/libs-external")

set(ZLIB_INCLUDE_DIR "${SDK_EXTERNAL_LIBS_DIR}/zlib")
set(PNG_INCLUDE_DIR "${SDK_EXTERNAL_LIBS_DIR}/png")

include_directories(
    "${SDK_SRC_DIR}"
    "${SDK_LIBS_DIR}/vt/src"
    "${SDK_LIBS_DIR}/mapnikvt/src"
    "${SDK_LIBS_DIR}/cartocss/src"
    "${SDK_LIBS_DIR}/nml/src"
    "${SDK_LIBS_DIR}/routing/src"
    "${SDK_EXTERNAL_LIBS_DIR}/boost"
    "${SDK_EXTERNAL_LIBS_DIR}/asio"
    "${SDK_EXTERNAL_LIBS_DIR}/cglib"
    "${SDK_EXTERNAL_LIBS_DIR}/stdext"
    "${SDK_EXTERNAL_LIBS_DIR}/cryptopp"
    "${SDK_EXTERNAL_LIBS_DIR}/freetype/include"
    "${SDK_EXTERNAL_LIBS_DIR}/harfbuzz/src"
    "${SDK_EXTERNAL_LIBS_DIR}/bidi"
    "${SDK_EXTERNAL_LIBS_DIR}/jpeg"
    "${SDK_EXTERNAL_LIBS_DIR}/miniz"
    "${SDK_EXTERNAL_LIBS_DIR}/pion/include"
    "${SDK_EXTERNAL_LIBS_DIR}/png"
    "${SDK_EXTERNAL_LIBS_DIR}/protobuf"
    "${SDK_EXTERNAL_LIBS_DIR}/pugixml/src"
    "${SDK_EXTERNAL_LIBS_DIR}/pvrt"
    "${SDK_EXTERNAL_LIBS_DIR}/rapidjson/include"
    "${SDK_EXTERNAL_LIBS_DIR}/picojson"
    "${SDK_EXTERNAL_LIBS_DIR}/rg_etc1"
    "${SDK_EXTERNAL_LIBS_DIR}/sqlite"
    "${SDK_EXTERNAL_LIBS_DIR}/sqlite3pp"
    "${SDK_EXTERNAL_LIBS_DIR}/tinyformat"
    "${SDK_EXTERNAL_LIBS_DIR}/tess2/include"
    "${SDK_EXTERNAL_LIBS_DIR}/utf8/source"
    "${SDK_EXTERNAL_LIBS_DIR}/webp/src"
    "${PNG_INCLUDE_DIR}"
)

if(INCLUDE_GDAL)
include_directories(
    "${SDK_EXTERNAL_LIBS_DIR}/gdal/include"
)
endif(INCLUDE_GDAL)

if(INCLUDE_OBJC)
include_directories(
    "${SDK_SRC_DIR}/../../ios/objc"
)
endif(INCLUDE_OBJC)

if(NOT IOS)
include_directories(
    "${ZLIB_INCLUDE_DIR}"
)
endif(NOT IOS)

if(ANDROID)
include_directories(
    "${SDK_SRC_DIR}/../../android/native"
)
elseif(IOS)
include_directories(
    "${SDK_SRC_DIR}/../../ios/native"
    "${SDK_SRC_DIR}/../../generated/ios-objc/proxies"
    "${SDK_SRC_DIR}/../../generated/ios-objc/wrappers"
)
elseif(WIN32)
include_directories(
    "${SDK_EXTERNAL_LIBS_DIR}/angle/include"
    "${SDK_SRC_DIR}/../../winphone/native"
)
endif()

# Subprojects
add_subdirectory("${SDK_LIBS_DIR}/vt" vt)
add_subdirectory("${SDK_LIBS_DIR}/mapnikvt" mapnikvt)
add_subdirectory("${SDK_LIBS_DIR}/cartocss" cartocss)
add_subdirectory("${SDK_LIBS_DIR}/nml" nml)
add_subdirectory("${SDK_LIBS_DIR}/routing" routing)
add_subdirectory("${SDK_EXTERNAL_LIBS_DIR}/bidi" bidi)
add_subdirectory("${SDK_EXTERNAL_LIBS_DIR}/cryptopp" cryptopp)
add_subdirectory("${SDK_EXTERNAL_LIBS_DIR}/freetype" freetype)
add_subdirectory("${SDK_EXTERNAL_LIBS_DIR}/harfbuzz" harfbuzz)
add_subdirectory("${SDK_EXTERNAL_LIBS_DIR}/jpeg" jpeg)
add_subdirectory("${SDK_EXTERNAL_LIBS_DIR}/miniz" miniz)
add_subdirectory("${SDK_EXTERNAL_LIBS_DIR}/png" png)
add_subdirectory("${SDK_EXTERNAL_LIBS_DIR}/pugixml" pugixml)
add_subdirectory("${SDK_EXTERNAL_LIBS_DIR}/pvrt" pvrt)
add_subdirectory("${SDK_EXTERNAL_LIBS_DIR}/rg_etc1" rg_etc1)
add_subdirectory("${SDK_EXTERNAL_LIBS_DIR}/sqlite" sqlite)
add_subdirectory("${SDK_EXTERNAL_LIBS_DIR}/sqlite3pp" sqlite3pp)
add_subdirectory("${SDK_EXTERNAL_LIBS_DIR}/tess2" tess2)
add_subdirectory("${SDK_EXTERNAL_LIBS_DIR}/webp" webp)
if(NOT IOS)
add_subdirectory("${SDK_EXTERNAL_LIBS_DIR}/zlib" zlib)
endif(NOT IOS)
if(NOT (WIN32 OR IOS OR ANDROID))
add_subdirectory("${SDK_EXTERNAL_LIBS_DIR}/pion" pion)
endif()

# SDK files
file(GLOB SDK_SRC_FILES
    "${SDK_SRC_DIR}/*/*.cpp"
    "${SDK_SRC_DIR}/*/*/*.cpp"
    "${SDK_SRC_DIR}/*/*.h"
    "${SDK_SRC_DIR}/*/*/*.h"
)
if (WIN32 OR IOS OR ANDROID)
list(REMOVE_ITEM SDK_SRC_FILES "${SDK_SRC_DIR}/network/HTTPClientPionImpl.cpp")
endif()

if (ANDROID)
file(GLOB SDK_ANDROID_SRC_FILES
    "${WRAPPER_DIR}/*.cpp"
    "${WRAPPER_DIR}/*.h"
    "${SDK_SRC_DIR}/../../android/native/*/*.cpp"
    "${SDK_SRC_DIR}/../../android/native/*/*.h"
    "${SDK_SRC_DIR}/../../android/native/*/*/*.cpp"
    "${SDK_SRC_DIR}/../../android/native/*/*/*.h"
)
set(SDK_SRC_FILES "${SDK_SRC_FILES}" "${SDK_ANDROID_SRC_FILES}")
elseif(IOS)
if(INCLUDE_OBJC)
file(GLOB SDK_OBJC_SRC_FILES
    "${SDK_SRC_DIR}/../../ios/objc/*/*.cpp"
    "${SDK_SRC_DIR}/../../ios/objc/*/*.mm"
    "${SDK_SRC_DIR}/../../ios/objc/*/*.h"
    "${SDK_SRC_DIR}/../../ios/objc/*.h"
)
set(SDK_SRC_FILES "${SDK_SRC_FILES}" "${SDK_OBJC_SRC_FILES}")
endif(INCLUDE_OBJC)
file(GLOB SDK_IOS_SRC_FILES
    "${WRAPPER_DIR}/*.cpp"
    "${WRAPPER_DIR}/*.mm"
    "${WRAPPER_DIR}/*.h"
    "${SDK_SRC_DIR}/../../ios/native/*/*.cpp"
    "${SDK_SRC_DIR}/../../ios/native/*/*.mm"
    "${SDK_SRC_DIR}/../../ios/native/*/*.h"
)
set(SDK_SRC_FILES "${SDK_SRC_FILES}" "${SDK_IOS_SRC_FILES}")
elseif(WIN32)
file(GLOB SDK_WINRT_SRC_FILES
    "${WRAPPER_DIR}/*.cpp"
    "${WRAPPER_DIR}/*.h"
    "${SDK_SRC_DIR}/../../winphone/native/*/*.cpp"
    "${SDK_SRC_DIR}/../../winphone/native/*/*.h"
    "${SDK_SRC_DIR}/../../winphone/native/*/*/*.cpp"
    "${SDK_SRC_DIR}/../../winphone/native/*/*/*.h"
)
set(SDK_SRC_FILES "${SDK_SRC_FILES}" "${SDK_WINRT_SRC_FILES}")

set_source_files_properties("${SDK_SRC_DIR}/../../winphone/native/utils/AssetUtils.cpp" PROPERTIES COMPILE_FLAGS "/ZW")
set_source_files_properties("${SDK_SRC_DIR}/../../winphone/native/utils/PlatformUtils.cpp" PROPERTIES COMPILE_FLAGS "/ZW")
set_source_files_properties("${SDK_SRC_DIR}/../../winphone/native/utils/GLESContext.cpp" PROPERTIES COMPILE_FLAGS "/ZW")
set_source_files_properties("${SDK_SRC_DIR}/../../winphone/native/components/Task.cpp" PROPERTIES COMPILE_FLAGS "/ZW")
endif()

# Group wrapper files into different groups, to reduce clutter
source_group("Wrapper Sources"
    REGULAR_EXPRESSION ".*_wrap.cpp"
)
source_group("Wrapper Headers"
    REGULAR_EXPRESSION ".*_wrap.h"
)

# Linking
if(SINGLE_LIBRARY)

set(SDK_SRC_FILES
    "${SDK_SRC_FILES}"
    "${vt_SRC_FILES}"
    "${mapnikvt_SRC_FILES}"
    "${cartocss_SRC_FILES}"
    "${nml_SRC_FILES}"
    "${routing_SRC_FILES}"
    "${bidi_SRC_FILES}"
    "${cryptopp_SRC_FILES}"
    "${rg_etc1_SRC_FILES}"
    "${pvrt_SRC_FILES}"
    "${pugixml_SRC_FILES}"
    "${libtess2_SRC_FILES}"
    "${miniz_SRC_FILES}"
    "${png_SRC_FILES}"
    "${jpeg_SRC_FILES}"
    "${webp_SRC_FILES}"
    "${freetype_SRC_FILES}"
    "${harfbuzz_SRC_FILES}"
    "${sqlite3pp_SRC_FILES}"
    "${sqlite_SRC_FILES}"
)

if(NOT IOS)
set(SDK_SRC_FILES ${SDK_SRC_FILES} ${zlib_SRC_FILES})
endif()
if(NOT (WIN32 OR IOS OR ANDROID))
set(SDK_SRC_FILES ${SDK_SRC_FILES} ${pion_SRC_FILES})
endif()

set(SDK_OBJECTS "")

else(SINGLE_LIBRARY)

set(SDK_OBJECTS
    $<TARGET_OBJECTS:vt>
    $<TARGET_OBJECTS:mapnikvt>
    $<TARGET_OBJECTS:cartocss>
    $<TARGET_OBJECTS:nml>
    $<TARGET_OBJECTS:routing>
    $<TARGET_OBJECTS:bidi>
    $<TARGET_OBJECTS:cryptopp>
    $<TARGET_OBJECTS:rg_etc1>
    $<TARGET_OBJECTS:pvrt>
    $<TARGET_OBJECTS:pugixml>
    $<TARGET_OBJECTS:libtess2>
    $<TARGET_OBJECTS:miniz>
    $<TARGET_OBJECTS:png>
    $<TARGET_OBJECTS:jpeg>
    $<TARGET_OBJECTS:webp>
    $<TARGET_OBJECTS:freetype>
    $<TARGET_OBJECTS:harfbuzz>
    $<TARGET_OBJECTS:sqlite3pp>
    $<TARGET_OBJECTS:sqlite>
)

if(NOT IOS)
set(SDK_OBJECTS ${SDK_OBJECTS} $<TARGET_OBJECTS:zlib>)
endif()
if(NOT (WIN32 OR IOS OR ANDROID))
set(SDK_OBJECTS ${SDK_OBJECTS} $<TARGET_OBJECTS:pion>)
endif()

endif(SINGLE_LIBRARY)

add_library(carto_mobile_sdk ${SDK_LIBRARY_TYPE}
    ${SDK_SRC_FILES}
    ${SDK_OBJECTS}
)

if(INCLUDE_GDAL)
add_library(gdal SHARED IMPORTED)
if(ANDROID)
set_property(TARGET gdal PROPERTY IMPORTED_LOCATION "${SDK_BASE_DIR}/prebuilt/gdal/libs/${ANDROID_ABI}/libgdal.so")
endif(ANDROID)
target_link_libraries(carto_mobile_sdk gdal)
endif(INCLUDE_GDAL)

if(ANDROID)
target_link_libraries(carto_mobile_sdk EGL GLESv2 log android jnigraphics)
elseif(IOS)
target_link_libraries(carto_mobile_sdk "-framework Foundation" "-framework CoreGraphics" "-framework CoreText" "-framework OpenGLES" "-framework GLKit" "-framework UIKit" "-framework CFNetwork")
elseif(WIN32)
target_link_libraries(carto_mobile_sdk msxml6.lib d3d11.lib dwrite.lib d2d1.lib "${ANGLE_LIB_DIR}/libEGL.lib" "${ANGLE_LIB_DIR}/libGLESv2.lib")
endif()

if(IOS)
if(ENABLE_BITCODE)
set_target_properties(carto_mobile_sdk PROPERTIES
    XCODE_ATTRIBUTE_ENABLE_BITCODE YES
)
else()
set_target_properties(carto_mobile_sdk PROPERTIES
    XCODE_ATTRIBUTE_ENABLE_BITCODE NO
)
endif()
endif(IOS)
if(IOS AND SINGLE_LIBRARY)
set_target_properties(carto_mobile_sdk PROPERTIES
    XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "7.0"
    XCODE_ATTRIBUTE_OTHER_LDFLAGS "-ObjC"
    XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES YES
    XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN YES
    XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH YES
)
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
set_target_properties(carto_mobile_sdk PROPERTIES
    XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING YES
    XCODE_ATTRIBUTE_DEAD_CODE_STRIPPING YES
    XCODE_ATTRIBUTE_SEPARATE_STRIP YES
    XCODE_ATTRIBUTE_STRIP_STYLE "non-global"
    XCODE_ATTRIBUTE_GENERATE_MASTER_OBJECT_FILE YES
    XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS NO
)
endif()
endif()
