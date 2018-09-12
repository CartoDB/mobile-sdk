#!/bin/bash

# mdoc executable to use, python executable to use
mdocExec="mdoc"
python="python"

# Set dirs
cmdDir=$(dirname $0)
baseDir="${cmdDir}/.."
buildDir="${baseDir}/build/xamarin_Android/bin/Release"
htmldocDir="${baseDir}/dist/xamarin/htmldoc"
tempDir="${baseDir}/build/htmldoc"
libraryDirs="-L /Developer/MonoAndroid/usr/lib/mandroid/platforms/android-10/ -L /Developer/MonoAndroid/usr/lib/mono/2.1 -L /Library/Frameworks/Mono.framework/Versions/Current/lib/mono/xbuild-frameworks/.NETPortable/v4.6"

# Remove old temp
rm -rf ${tempDir}

# Remove old html documentation
rm -rf ${htmldocDir}

# Build individual XML documentation files
${mdocExec} update -i "${buildDir}/CartoMobileSDK.Android.xml" -o ${tempDir} "${buildDir}/CartoMobileSDK.Android.dll" ${libraryDirs}

# Cleanup the documentation XML files
${python} "${cmdDir}/htmldoc/cleanup-htmldoc.py" ${tempDir}

# Build htmldoc
${mdocExec} export-html --template="${cmdDir}/htmldoc/template.xslt" --ignore -o ${htmldocDir} ${tempDir}

# Finished
echo "Done!"
