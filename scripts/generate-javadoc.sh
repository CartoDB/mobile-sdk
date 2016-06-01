#!/bin/bash 

# Swig dir, if not specified use use without dir, the user might have the dir in it's env variable
javadocExec="javadoc"

# Set dirs
cmdDir=$(dirname $0)
baseDir="${cmdDir}/.."
javaDir="${baseDir}/android/java"
javaGenDir="${baseDir}/android/generated/proxies"
javadocDir="${baseDir}/dist/android/javadoc"
tempDir="${baseDir}/build/javadoc"

# Remove old temp
rm -rf ${tempDir}

# Copy java files, remove *JNI.java files
mkdir -p ${tempDir}
cp -r ${javaDir}/* ${tempDir}
cp -r ${javaGenDir}/* ${tempDir}
find ${tempDir} -name "*BaseMapView.java" -exec rm {} \;
find ${tempDir} -name "*JNI.java" -exec rm {} \;
find ${tempDir} -name "*MapTileQuadTreeNode.java" -exec rm {} \;
find ${tempDir} -name "*QuadTreeNodeType.java" -exec rm {} \;
find ${tempDir} -name "*QuadTreeRootType.java" -exec rm {} \;
find ${tempDir} -name "*RedrawRequestListener.java" -exec rm {} \;
find ${tempDir} -name "*ConfigChooser.java" -exec rm {} \;
find ${tempDir} -name "*GLUtils.java" -exec rm {} \;
find ${tempDir} -name "*AndroidUtils.java" -exec rm {} \;
find ${tempDir} -name "*LicenseType.java" -exec rm {} \;
find ${tempDir} -name "*.java" > ${tempDir}/files

# Execute JavaDoc
rm -rf ${javadocDir}
mkdir -p ${javadocDir}
${javadocExec} -source 1.6 -d ${javadocDir} -doctitle "Nutiteq SDK 3 for Android" "@${tempDir}/files"

# Finished
echo "Done!"
