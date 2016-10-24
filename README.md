# Carto Mobile SDK 

## Current build status

| Platforms    | Status |
|:-------------|:-------|
| Android, iOS |  [![Build Status](https://travis-ci.org/CartoDB/mobile-sdk.svg?branch=master)](https://travis-ci.org/CartoDB/mobile-sdk)|

# Building

*We strongly suggest to use the precompiled SDK versions that can be found in
the samples below (in 'Usage' section).* The precompiled libraries include 
support for CARTO online services (basemaps, offline map packages, routing, etc).
Also, getting all the SDK dependencies resolved and waiting for the build
to complete can be very time-consuming.

## Dependencies
Get the prepackaged dependencies from github **mobile-external-libs** repository (https://github.com/CartoDB/mobile-external-libs). Add links to external-libs/prebuilt at the project root directory level.

In addition, **boost** library sources should be downloaded from boost.org and configured as described in mobile-external-libs documentation.

Special **swig** version (swig-2.0.11-nutiteq branch) is needed for generating language-specific wrappers, this can be downloaded from https://github.com/CartoDB/swig. Clone it and compile it using usual `./configure;make` routine. Make sure build script refers to this one.

**Python 2.7.x** is used for build scripts

Android build requires **Android SDK** and **Android NDK r11** or later.

iOS build requires **XCode 7.3** or later.

Windows Phone build requires Visual Studio 2015 AND Visual Studio 2015 SDK.

## Building process

Be patient - full build will take 1+ hours. You can speed it up by limiting architectures and platforms where it is built.

```
git clone https://github.com/CartoDB/mobile-external-libs.git
cd mobile-external-libs/libs-external
ln -s ../../sdk_3d/all/libs/boost # change to folder where you have boost libs!
git clone https://github.com/CartoDB/mobile-sdk.git
cd mobile-sdk
ln -s ../mobile-external-libs/libs-external/
cd mobile-sdk/scripts
```

## Android build 
```
python swigpp-java.py --profile standard
python build-android.py --profile standard --javac /Library/Java/JavaVirtualMachines/1.6.0.jdk/Contents/Commands/javac
```

## iOS build:
```
python swigpp-objc.py --profile standard
python build-ios.py --profile standard
```

## Xamarin Android build:
```
python swigpp-csharp.py --profile standard android
python build-xamarin.py --profile standard android
```

## Xamarin iOS build:
```
python swigpp-csharp.py --profile standard ios
python build-xamarin.py --profile standard ios
```

## Windows Phone build
```
python swigpp-csharp.py --profile standard winphone
python build-winphone.py --profile standard
```

# Usage

* Developer docs: https://carto.com/docs/carto-engine/mobile-sdk/
* Android sample app: https://github.com/CartoDB/mobile-android-samples
* iOS sample app: https://github.com/CartoDB/mobile-ios-samples
* .NET (Xamarin and Windows 10) sample app: https://github.com/CartoDB/mobile-dotnet-samples
