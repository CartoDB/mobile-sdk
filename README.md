# CARTO Mobile SDK 

CARTO Mobile SDK is a multi-platform mobile
mapping SDK written mostly in C++11 with bindings to numerous languages
(Java/C# for Android, ObjectiveC/C# for iOS and C# for Universal Windows Platform).

This project contains the core part of the SDK, for samples, look at
the ['Usage' section](#usage).

# Building
**We strongly suggest to use the precompiled SDK versions that can be found in
the [Releases](https://github.com/CartoDB/mobile-sdk/releases) section** 

Getting all the SDK dependencies resolved and waiting for the build to complete can be very time-consuming.

## Dependencies
Use `git submodule` to resolve all source-level dependencies

```
git submodule update --init --remote --recursive
```

Special **swig** version (swig-2.0.11-nutiteq branch) is needed for generating language-specific wrappers, this can be downloaded from https://github.com/CartoDB/swig. Clone it and compile it using usual `./autogen.sh; ./configure; make` routine. Make sure build script refers to this one.

**Python 2.7.x** is used for build scripts

**CMake 3.14 or later** is required by build scripts

Android build requires **Android SDK** and **Android NDK r19** or later.

iOS build requires **XCode 7.3** or later.

Universal Windows Platform build requires **Visual Studio 2019**.

## Building process
Be patient - full build will take 1+ hours. You can speed it up by limiting architectures and platforms where it is built.

Set up boost library:

```
cd libs-external/boost
./bootstrap.sh
./b2 headers
cd ../..
```

Go to 'scripts' library where the actual build scripts are located:

```
cd mobile-sdk/scripts
```

## Android build 
```
python swigpp-java.py --profile standard
python build-android.py --profile standard
```

## iOS build
```
python swigpp-objc.py --profile standard
python build-ios.py --profile standard
```

## Xamarin Android build
```
python swigpp-csharp.py --profile standard android
python build-xamarin.py --profile standard android
```

## Xamarin iOS build
```
python swigpp-csharp.py --profile standard ios
python build-xamarin.py --profile standard ios
```

## Universal Windows Platform build
```
python swigpp-csharp.py --profile standard winphone
python build-winphone.py --profile standard
```

# Usage
* Developer docs: https://carto.com/docs/carto-engine/mobile-sdk/
* Android sample app: https://github.com/CartoDB/mobile-android-samples
* iOS sample app: https://github.com/CartoDB/mobile-ios-samples
* .NET (Xamarin and UWP) sample app: https://github.com/CartoDB/mobile-dotnet-samples
* Scripts for preparing offline packages: https://github.com/nutiteq/mobile-sdk-scripts

# Support, Questions?
* Post an [issue](https://github.com/CartoDB/mobile-sdk/issues) to this project, submit a [Pull Request](https://github.com/CartoDB/mobile-sdk/pulls)
* Post to CARTO developer group for generic CARTO related questions: https://groups.google.com/forum/#!forum/cartodb
* Commercial support options: sales@carto.com
