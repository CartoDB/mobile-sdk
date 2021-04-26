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
find ${tempDir} -name "*BaseMapView.java" -exec rm {} \;
find ${tempDir} -name "*JNI.java" -exec rm {} \;
find ${tempDir} -name "*RedrawRequestListener.java" -exec rm {} \;
find ${tempDir} -name "*ConfigChooser.java" -exec rm {} \;
find ${tempDir} -name "*AndroidUtils.java" -exec rm {} \;
find ${tempDir} -name "*LicenseType.java" -exec rm {} \;
find ${tempDir} -name "*.java" > ${tempDir}/files

# Execute JavaDoc
rm -rf ${javadocDir}
mkdir -p ${javadocDir}
${javadocExec} -hdf project.name "CARTO Mobile SDK" -doclet com.google.doclava.Doclava -docletpath ${cmdDir}/javadoc/doclava-1.0.6.jar -public -classpath "${ANDROID_HOME}/platforms/android-9/android.jar" -source 1.6 -d "${javadocDir}" "@${tempDir}/files"

# Create .jar
cd ${javadocDir}
${jarExec} cf ../carto-mobile-sdk-javadoc.jar .
cd ${cmdDir}

# Finished
echo "Done!"
