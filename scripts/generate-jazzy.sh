#!/bin/bash 

# Set dirs
cmdDir=$(dirname $0)
baseDir="${cmdDir}/.."
tempDir="${baseDir}/build/jazzy"
distDir="${baseDir}/dist/ios"

# Copy proxy files to temp directory
rm -rf ${tempDir}
mkdir -p ${tempDir}
mkdir -p ${tempDir}/ui
cp -r ${distDir}/CartoMobileSDK.framework/Headers/CartoMobileSDK.h ${tempDir}/CartoMobileSDK.h
cp -r ${baseDir}/generated/ios-objc/proxies/* ${tempDir}
cp -r ${baseDir}/ios/objc/ui/MapView.h ${tempDir}/ui/MapView.h
cp -r ${baseDir}/ios/objc/ui/MapView.mm ${tempDir}/ui/MapView.mm
find ${tempDir} -name "*NTIOSUtils.*" -exec rm {} \;

# Execute Jazzy
rm -rf ${distDir}/docObjC
jazzy --clean --author CARTO --author_url https://www.carto.com --github_url https://github.com/cartodb/mobile-sdk --module CartoMobileSDK --output ${distDir}/docObjC --umbrella-header ${tempDir}/CartoMobileSDK.h --objc --sdk iphonesimulator

# Finished
echo "Done!"
