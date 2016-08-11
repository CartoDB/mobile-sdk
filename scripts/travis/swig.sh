echo '---- Downloading and installing Swig ----'
set -e
rm -rf mobile-swig
git clone https://github.com/CartoDB/mobile-swig.git
cd mobile-swig
cd pcre
cmake .
make
sudo make install
cd ..
./autogen.sh
./configure --disable-ccache
make
sudo make install
