#!/bin/bash 

# Doxygen directory
doxygenDir="/usr/local/bin"

# Set dirs
cmdDir=$(dirname $0)
baseDir="${cmdDir}/.."
tempDir="${baseDir}/build/doxygen"
distDir="${baseDir}/dist/ios"

# Copy proxy files to temp directory
rm -rf ${tempDir}
mkdir -p ${tempDir}
cp -r ${baseDir}/ios/generated/proxies/* ${tempDir}
cp -r ${baseDir}/ios/objc/ui/MapView.h ${tempDir}
find ${tempDir} -name "*NTBaseMapView.*" -exec rm {} \;
find ${tempDir} -name "*NTMapTileQuadTreeNode.*" -exec rm {} \;
find ${tempDir} -name "*NTQuadTreeNodeType.*" -exec rm {} \;
find ${tempDir} -name "*NTQuadTreeRootType.*" -exec rm {} \;
find ${tempDir} -name "*NTRedrawRequestListener.*" -exec rm {} \;
find ${tempDir} -name "*NTGLUtils.*" -exec rm {} \;
find ${tempDir} -name "*NTIOSUtils.*" -exec rm {} \;

# Remove attributes unsupported by doxygen from source files
find ${tempDir} -name "*.h" -exec sed -i '' 's/__attribute__ ((visibility("default")))//g' {} \;

# Execute doxygen
rm -rf ${distDir}/docObjC
${doxygenDir}/doxygen "doxygen/doxygen-objc.conf"

# Finished
echo "Done!"
