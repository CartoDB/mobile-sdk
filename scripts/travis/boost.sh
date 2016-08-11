echo '---- Downloading and installing boost ----'
set -ex
curl -o boost_1_61_9.tar.gz -L https://sourceforge.net/projects/boost/files/boost/1.61.0/boost_1_61_0.tar.gz
rm -r -f boost_1_61_0
tar xpfz boost_1_61_0.tar.gz
export BOOST_DIR=$PWD/boost_1_61_0
