# About

This is a project for creating VSIX installer for Visual Studio. Once generated, the .vsix file can be installed by simply double clicking on it.
After installing, it can be uninstalled from Visual Studio by selecting 'Tools/Extensions and Updates' menu and then 'SDKs/Carto Mobile SDK for Windows Phone' and pressing uninstall.

# Tools dependencies

1. Visual Studio 2015 RC or later AND Visual Studio 2013 update 4 or later.
2. Visual Studio SDK for 2015
3. CMake 3.2 or later
4. Boost 1.58 (must be unzipped in all/libs/boost)

# How to compile the VSIX from scratch. 

1. Go to all/native subdirectory
2. Execute
   * mkdir BUILD_ARM
   * cd BUILD_ARM
   * cmake -DBOOST_ROOT=/local/boost_1_58_0 -G "Visual Studio 14 2015" -DCMAKE_SYSTEM_NAME="WindowsPhone" -DCMAKE_SYSTEM_VERSION="8.1" -DCMAKE_GENERATOR_PLATFORM="ARM" -DAngle_INCLUDE_DIR="c:/projects/angle/include" -DAngle_LIB_DIR="c:/projects/angle/lib/ARM" ..

3. Load Visual Studio solution carto_mobile_sdk.sln solution from BUILD_ARM directory, set configuration to 'Release' and rebuild 'carto_mobile_sdk' project

4. Repeat steps 1-3, but for x86
   * mkdir BUILD_x86
   * cd BUILD_x86
   * cmake -DBOOST_ROOT=/local/boost_1_58_0 -G "Visual Studio 14 2015" -DCMAKE_SYSTEM_NAME="WindowsPhone" -DCMAKE_SYSTEM_VERSION="8.1" -DCMAKE_GENERATOR_PLATFORM="Win32" -DAngle_INCLUDE_DIR="c:/projects/angle/include" -DAngle_LIB_DIR="c:/projects/angle/lib" ..

7. Load Visual Studio solution dotnet/winphone/winphone.sln, set configuration to 'Release', rebuild both(!) 'x86' and 'ARM' targets

8. Load Visual Studio solution winphone_component/winphone_components.sln, rebuild using 'Release' configuration and 'Any CPU' target
