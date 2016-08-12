export DIR=`pwd`
source scripts/travis/cmake.sh;
cd $DIR;
source scripts/travis/swig.sh;
cd $DIR;
source scripts/travis/boost.sh;
cd $DIR;
source scripts/travis/externallibs.sh;
cd $DIR;
source scripts/travis/link-dirs.sh;
cd $DIR

if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  curl -L http://dl.google.com/android/ndk/android-ndk-r12b-linux-x86_64.bin -O
  chmod u+x android-ndk-r12b-linux-x86_64.bin
  ./android-ndk-r12b-linux-x86_64.bin > /dev/null
  rm android-ndk-r12b-linux-x86_64.bin
  export ANDROID_NDK_HOME=`pwd`/android-ndk-r12b;
fi
