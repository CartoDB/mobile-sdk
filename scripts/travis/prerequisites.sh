set -e

rvm get head

echo '---- Updating submodules ----'
git submodule update --init --remote --recursive

echo '---- Setting up boost ----'
cd libs-external/boost
./bootstrap.sh
./b2 headers
cd ../..

echo '---- Downloading and installing CMake ----'
curl -o cmake-3.10.2.tar.gz -L https://cmake.org/files/v3.10/cmake-3.10.2.tar.gz
rm -rf cmake-3.10.2
tar xpfz cmake-3.10.2.tar.gz
cd cmake-3.10.2
./configure --prefix=`pwd`/dist
make
make install
export PATH=$PWD/dist/bin:$PATH
cd ..

echo '---- Downloading and installing SWIG ----'
rm -rf mobile-swig
git clone https://github.com/CartoDB/mobile-swig.git
cd mobile-swig
cd pcre
aclocal
automake
./configure --prefix=`pwd`/pcre-swig-install --disable-shared
make
make install
cd ..
./autogen.sh
./configure --disable-ccache --prefix=`pwd`/dist
make
make install || true
export PATH=$PWD/dist/bin:$PATH
cd ..

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  echo '---- Downloading and installing Android NDK r22b ----'
  curl -L https://dl.google.com/android/repository/android-ndk-r22b-linux-x86_64.zip -O
  rm -r -f android-ndk-r22b
  unzip -q android-ndk-r22b-linux-x86_64.zip
  rm android-ndk-r22b-linux-x86_64.zip
  export ANDROID_NDK_HOME=`pwd`/android-ndk-r22b;
#  export ANDROID_HOME=/usr/local/android-sdk
fi
