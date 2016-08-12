set -e
cd scripts
if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
  python swigpp-java.py
  python build-android.py --android-abi=armeabi-v7a
fi
if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
  python swigpp-objc.py
  python build-ios.py --ios-arch=arm64 | egrep '^(/.+:[0-9+:[0-9]+:.(error|warning):|fatal|===)';
fi
