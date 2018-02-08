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
cp -r ${baseDir}/generated/ios-objc/proxies/* ${tempDir}
cp -r ${baseDir}/ios/objc/CartoMobileSDK.h ${tempDir}/CartoMobileSDK_base.h
cp -r ${baseDir}/ios/objc/ui/MapView.h ${tempDir}/ui/MapView.h
cp -r ${baseDir}/ios/objc/ui/MapView.mm ${tempDir}/ui/MapView.mm
find ${tempDir} -name "*NTIOSUtils.*" -exec rm {} \;

# Generate umbrella header with additional #define-s
echo "#define _CARTO_SEARCH_SUPPORT\n" >> ${tempDir}/CartoMobileSDK.h
echo "#define _CARTO_GEOCODING_SUPPORT\n" >> ${tempDir}/CartoMobileSDK.h
echo "#define _CARTO_ROUTING_SUPPORT\n" >> ${tempDir}/CartoMobileSDK.h
echo "#define _CARTO_OFFLINE_SUPPORT\n" >> ${tempDir}/CartoMobileSDK.h
echo "#define _CARTO_CUSTOM_BASEMAP_SUPPORT\n" >> ${tempDir}/CartoMobileSDK.h
echo "#define _CARTO_PACKAGEMANAGER_SUPPORT\n" >> ${tempDir}/CartoMobileSDK.h
echo "#define _CARTO_EDITABLE_SUPPORT\n" >> ${tempDir}/CartoMobileSDK.h
echo "#define _CARTO_WKBT_SUPPORT\n" >> ${tempDir}/CartoMobileSDK.h
echo "#define _CARTO_VALHALLA_ROUTING_SUPPORT\n" >> ${tempDir}/CartoMobileSDK.h
echo "#define _CARTO_NMLMODELLODTREE_SUPPORT\n" >> ${tempDir}/CartoMobileSDK.h
cat ${tempDir}/CartoMobileSDK_base.h >> ${tempDir}/CartoMobileSDK.h

# Execute Jazzy
rm -rf ${distDir}/docObjC
jazzy --clean --author CARTO --author_url https://www.carto.com --github_url https://github.com/cartodb/mobile-sdk --module CartoMobileSDK --output ${distDir}/docObjC --umbrella-header ${tempDir}/CartoMobileSDK.h --objc --sdk iphonesimulator

# Finished
echo "Done!"
