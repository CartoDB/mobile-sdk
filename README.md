# Carto Mobile SDK readme

## Dependencies
Get the prepackaged dependencies from github CartoDB/mobile-external-libs
repository (https://github.com/CartoDB/mobile-external-libs). Add links
to external-libs/prebuilt at the project root directory level.

In addition, boost library should be downloaded and configured as described
in mobile-external-libs documentation.

Special swig version (swig-2.0.11-nutiteq branch) is needed for generating 
language-specific wrappers, this can be downloaded from https://github.com/CartoDB/swig

## Building

Be patient - full build will take 1+ hours. You can speed it up by limiting architectures and platforms where it is built.

```
git clone https://github.com/CartoDB/mobile-external-libs.git
cd mobile-external-libs/libs-external
ln -s ../../sdk_3d/all/libs/boost # change to folder where you have boost libs!
git clone https://github.com/CartoDB/mobile-sdk.git
cd mobile-sdk
ln -s ../mobile-external-libs/libs-external/
cd mobile-sdk/scripts


# Android SDK build 
python swigpp-java.py --profile standard
python build-android.py --profile standard --javac /Library/Java/JavaVirtualMachines/1.6.0.jdk/Contents/Commands/javac
```

