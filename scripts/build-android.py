import os
import shutil
import argparse
from build.sdk_build_utils import *

ANDROID_TOOLCHAINS = {
  'armeabi':     'arm-linux-androideabi',
  'armeabi-v7a': 'arm-linux-androideabi',
  'x86':         'x86',
  'mips':        'mipsel-linux-android',
  'arm64-v8a':   'aarch64-linux-android',
  'x86_64':      'x86_64',
  'mips64':      'mips64el-linux-android'
}

ANDROID_ABIS = ANDROID_TOOLCHAINS.keys()

def javac(args, dir, *cmdArgs):
  return execute(args.javac, dir, *cmdArgs)

def jar(args, dir, *cmdArgs):
  return execute(args.jar, dir, *cmdArgs)

def gradle(args, dir, *cmdArgs):
  return execute(args.gradle, dir, *cmdArgs)

def buildAndroidSO(args, abi):
  version = getVersion(args.buildnumber) if args.configuration == 'Release' else 'Devel'
  baseDir = getBaseDir()
  buildDir = getBuildDir('android', abi)
  distDir = getDistDir('android')
  defines = ["-D%s" % define for define in args.defines.split(';') if define]
  options = ["-D%s" % option for option in args.cmakeoptions.split(';') if option]

  if not cmake(args, buildDir, options + [
    '-DCMAKE_TOOLCHAIN_FILE=%s/scripts/android-cmake/android.toolchain.cmake' % baseDir,
    '-DANDROID_NDK=%s' % args.androidndkpath,
    '-DCMAKE_BUILD_TYPE=%s' % args.configuration,
    '-DWRAPPER_DIR=%s' % ('%s/generated/android-java/wrappers' % baseDir),
    "-DANDROID_TOOLCHAIN_NAME='%s-%s'" % (ANDROID_TOOLCHAINS[abi], args.compiler if not args.compiler.startswith('gcc-') else args.compiler[4:]),
    "-DANDROID_ABI='%s'" % abi,
    "-DANDROID_STL='%s'" % ('c++_static' if args.compiler.startswith('clang') else 'gnustl_static'),
    "-DANDROID_NATIVE_API_LEVEL='%s'" % ('android-21' if '64' in abi else 'android-10'),
    '-DSDK_CPP_DEFINES=%s' % " ".join(defines),
    "-DSDK_VERSION='%s'" % version,
    "-DSDK_PLATFORM='Android'",
    '%s/scripts/build' % baseDir
  ]):
    return False
  if not cmake(args, buildDir, [
    '--build', '.', '--', '-j4'
  ]):
    return False
  return makedirs('%s/%s' % (distDir, abi)) and copyfile('%s/libcarto_mobile_sdk.so' % buildDir, '%s/%s/libcarto_mobile_sdk.so' % (distDir, abi))

def buildAndroidJAR(args):
  shutil.rmtree(getBuildDir('android_java'), True)

  baseDir = getBaseDir()
  buildDir = getBuildDir('android_java')
  distDir = getDistDir('android')

  javaFiles = []
  for dirpath, dirnames, filenames in os.walk("%s/generated/android-java/proxies" % baseDir):
    for filename in [f for f in filenames if f.endswith(".java")]:
      javaFiles.append(os.path.join(dirpath, filename))
  for dirpath, dirnames, filenames in os.walk("%s/android/java" % baseDir):
    for filename in [f for f in filenames if f.endswith(".java")]:
      javaFiles.append(os.path.join(dirpath, filename))

  if not javac(args, buildDir,
    '-source', '1.6',
    '-target', '1.6',
    '-bootclasspath', '%s/scripts/android/rt.jar' % baseDir,
    '-classpath', '%s/platforms/android-10/android.jar' % args.androidsdkpath,
    '-d', buildDir,
    *javaFiles
  ):
    return False
  
  currentDir = os.getcwd()
  os.chdir(buildDir)
  classFiles = []
  for dirpath, dirnames, filenames in os.walk("."):
    for filename in [f for f in filenames if f.endswith(".class")]:
      classFiles.append(os.path.join(dirpath[2:], filename))
  os.chdir(currentDir)

  if not jar(args, buildDir,
    'cf', 'carto-mobile-sdk.jar',
    *classFiles
  ):
    return False
  if makedirs(distDir) and copyfile('%s/carto-mobile-sdk.jar' % buildDir, '%s/carto-mobile-sdk.jar' % distDir):
    print "Output available in:\n%s" % distDir
    return True
  return False

def buildAndroidAAR(args):
  baseDir = getBaseDir()
  buildDir = getBuildDir('android-aar')
  distDir = getDistDir('android')

  if not gradle(args, '%s/scripts' % baseDir,
    '-p', 'android-aar',
    '--project-cache-dir', buildDir,
    'assemble%s' % args.configuration
  ):
    return False
  if makedirs(distDir) and copyfile('%s/outputs/aar/android-aar-%s.aar' % (buildDir, args.configuration.lower()), '%s/carto-mobile-sdk.aar' % distDir):
    print "Output available in:\n%s" % distDir
    return True
  return False

parser = argparse.ArgumentParser()
parser.add_argument('--profile', dest='profile', default=getDefaultProfile(), choices=getProfiles().keys(), help='Build profile')
parser.add_argument('--android-abi', dest='androidabi', default=['all'], choices=ANDROID_ABIS + ['all'], nargs='+', help='Android target ABIs')
parser.add_argument('--android-ndk-path', dest='androidndkpath', default='auto', help='Android NDK path')
parser.add_argument('--android-sdk-path', dest='androidsdkpath', default='auto', help='Android SDK path')
parser.add_argument('--defines', dest='defines', default='', help='Defines for compilation')
parser.add_argument('--javac', dest='javac', default='javac', help='Java compiler executable')
parser.add_argument('--jar', dest='jar', default='jar', help='Jar executable')
parser.add_argument('--cmake', dest='cmake', default='cmake', help='CMake executable')
parser.add_argument('--cmake-options', dest='cmakeoptions', default='', help='CMake options')
parser.add_argument('--gradle', dest='gradle', default='', help='Gradle executable')
parser.add_argument('--build-number', dest='buildnumber', default='', help='Build sequence number, goes to version str')
parser.add_argument('--compiler', dest='compiler', default='gcc-4.9', choices=['gcc-4.9', 'clang'], help='C++ compiler')
parser.add_argument('--configuration', dest='configuration', default='Release', choices=['Release', 'Debug'], help='Configuration')
args = parser.parse_args()
if 'all' in args.androidabi:
  args.androidabi = ANDROID_ABIS
if args.androidsdkpath == 'auto':
  args.androidsdkpath = os.environ.get('ANDROID_HOME', None)
  if args.androidsdkpath is None:
    print "ANDROID_HOME variable not set"
    exit(-1)
if args.androidndkpath == 'auto':
  args.androidndkpath = os.environ.get('ANDROID_NDK_HOME', None)
  if args.androidndkpath is None:
    print "ANDROID_NDK_HOME variable not set"
    exit(-1)
args.defines += ';' + getProfiles()[args.profile].get('defines', '')
args.cmakeoptions += ';' + getProfiles()[args.profile].get('cmake-options', '')

for abi in args.androidabi:
  if not buildAndroidSO(args, abi):
    exit(-1)
if not buildAndroidJAR(args):
  exit(-1)

if args.gradle:
  if not buildAndroidAAR(args):
    exit(-1)
