set -e

rvm get head

echo '---- Downloading and installing CMake ----'
curl -o cmake-3.5.2.tar.gz -L https://cmake.org/files/v3.5/cmake-3.5.2.tar.gz
rm -rf cmake-3.5.2
tar xpfz cmake-3.5.2.tar.gz
cd cmake-3.5.2
./configure
make
sudo make install
cd ..

echo '---- Downloading and installing Swig ----'
rm -rf mobile-swig
git clone https://github.com/CartoDB/mobile-swig.git
cd mobile-swig
cd pcre
aclocal
automake
./configure --prefix=`pwd`/pcre-swig-install --disable-shared
make
make install
export PATH=$PATH:/usr/local/bin
cd ..
./autogen.sh
./configure --disable-ccache
make
sudo make install || true
cd ..

echo '---- Downloading and installing External libs ----'
rm -rf mobile-external-libs
git clone https://github.com/CartoDB/mobile-external-libs.git
ln -sf `pwd`/mobile-external-libs/libs-external ./libs-external

echo '---- Downloading and installing boost ----'
curl -o boost_1_61_0.tar.gz -L https://sourceforge.net/projects/boost/files/boost/1.61.0/boost_1_61_0.tar.gz
rm -r -f boost_1_61_0
tar xpfz boost_1_61_0.tar.gz
ln -sf `pwd`/boost_1_61_0 ./mobile-external-libs/libs-external/boost

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  echo '---- Downloading and installing Android NDK r12b ----'
  curl -L http://dl.google.com/android/repository/android-ndk-r12b-linux-x86_64.zip -O
  rm -r -f android-ndk-r12b
  unzip -q android-ndk-r12b-linux-x86_64.zip
  rm android-ndk-r12b-linux-x86_64.zip
  export ANDROID_NDK_HOME=`pwd`/android-ndk-r12b;
  export ANDROID_HOME=/usr/local/android-sdk
fi
