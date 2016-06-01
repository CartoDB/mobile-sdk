#!/bin/bash 

# Adb dir, if not specified use use without dir
androidAdbExec="$1"
# Remove last /
androidAdbExec="${androidAdbExec%/}"
if [ -z "${androidAdbExec}" ]; then
	androidAdbExec=adb
else
	androidAdbExec="${androidAdbExec}/platform-tools/adb"
fi
echo "Adb exec: ${androidAdbExec}"

# Ndk-stack dir, if not specified use use without dir
androidNdkStackExec="$2"
# Remove last /
androidNdkStackExec="${androidNdkStackExec%/}"
if [ -z "${androidNdkStackExec}" ]; then
	androidNdkStackExec=ndk-stack
else
	androidNdkStackExec="${androidNdkStackExec}/ndk-stack"
fi
echo "Ndk-stack exec: ${androidNdkStackExec}"

# Platform
platform="$3"
if [ -z "${platform}" ]; then
	platform="armeabi"
fi
echo "Platform: ${platform}"

# Set dirs
cmdDir="$(dirname $0)"

"${androidAdbExec}" logcat -v raw -d > "${cmdDir}/templog.txt" 
#tail -r "${cmdDir}/templog.txt" 
sed -i '' '1!G;h;$!d' "${cmdDir}/templog.txt"
sed -i '' '/\*\*\* \*\*\*/,$d' "${cmdDir}/templog.txt"
sed -i '' '1!G;h;$!d' "${cmdDir}/templog.txt"
(echo "*** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***"; cat "${cmdDir}/templog.txt") > tmpFile
mv tmpfile "${cmdDir}/templog.txt"
"${androidNdkStackExec}" -sym "${cmdDir}/obj/local/${platform}" -dump "${cmdDir}/templog.txt"
rm "${cmdDir}/templog.txt"



