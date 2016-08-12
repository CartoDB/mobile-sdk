echo '---- Downloading and installing CMake ----'
set -e
curl -o cmake-3.5.2.tar.gz -L https://cmake.org/files/v3.5/cmake-3.5.2.tar.gz
rm -rf cmake-3.5.2
tar xpfz cmake-3.5.2.tar.gz
cd cmake-3.5.2
./configure
make
sudo make install
cd ..

