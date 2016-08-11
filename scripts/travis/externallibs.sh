echo '---- Downloading and installing External libs ----'
set -ex
rm -rf mobile-external-libs
git clone https://github.com/CartoDB/mobile-external-libs.git
export EXTERNALLIBS_DIR=$PWD/mobile-external-libs
