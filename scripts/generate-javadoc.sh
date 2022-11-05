#!/bin/bash 

# Javadoc executable to use
javadocExec="javadoc"
jarExec="jar"

# Set dirs
cmdDir=$(dirname $0)
baseDir="${cmdDir}/.."
javaDir="${baseDir}/android/java"
javaGenDir="${baseDir}/generated/android-java/proxies"
javadocDir="${baseDir}/dist/android/javadoc"
tempDir="${baseDir}/build/javadoc"

# Remove old temp
rm -rf ${tempDir}

# Copy java files, remove *JNI.java files
mkdir -p ${tempDir}
cp -r ${javaDir}/* ${tempDir}
cp -r ${javaGenDir}/* ${tempDir}
find ${tempDir} -name "*JNI.java" -exec rm {} \;
find ${tempDir} -name "*.java" > ${tempDir}/files

# Execute JavaDoc
rm -rf ${javadocDir}
mkdir -p ${javadocDir}
${javadocExec} -public -doctitle "CARTO Mobile SDK" -classpath "${ANDROID_HOME}/platforms/android-21/android.jar" -d "${javadocDir}" -sourcepath "@${tempDir}/files"

# Create .jar
cd ${javadocDir}
${jarExec} cf ../carto-mobile-sdk-javadoc.jar .
cd ${cmdDir}

# Finished
echo "Done!"
