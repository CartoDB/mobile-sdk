import os
import sys
import shutil
import argparse
import string
from build.sdk_build_utils import *

ANDROID_TOOLCHAINS = {
  'armeabi-v7a': 'arm-linux-androideabi',
  'x86':         'x86',
  'arm64-v8a':   'aarch64-linux-android',
  'x86_64':      'x86_64'
}

ANDROID_ABIS = list(ANDROID_TOOLCHAINS.keys())

def javac(args, dir, *cmdArgs):
  return execute(args.javac, dir, *cmdArgs)

def jar(args, dir, *cmdArgs):
  return execute(args.jar, dir, *cmdArgs)

def gradle(args, dir, *cmdArgs):
  return execute(args.gradle, dir, *cmdArgs)

def zip(args, dir, *cmdArgs):
  return execute(args.zip, dir, *cmdArgs)

def detectAndroidAPIs(args):
  api32, api64 = None, None
  with open('%s/meta/platforms.json' % args.androidndkpath, 'rb') as f: 
    platforms = json.load(f)
    minapi = platforms.get('min', 1)
    maxapi = platforms.get('max', 0)
    for api in range(minapi, maxapi + 1):
      if api >= 11:
        api32 = min(api32 or api, api)
      if api >= 21:
        api64 = min(api64 or api, api)
  return api32, api64

def detectAndroidJavaAPI(args):
  apiJava = None
  for name in os.listdir('%s/platforms' % args.androidsdkpath):
    if name.startswith('android-'):
      api = int(name[8:])
      if api >= 11:
        apiJava = min(apiJava or api, api)
  return apiJava

def buildAndroidSO(args, abi):
  version = getVersion(args.buildnumber) if args.configuration == 'Release' else 'Devel'
  baseDir = getBaseDir()
  buildDir = getBuildDir('android', abi)
  distDir = getDistDir('android')
  defines = ["-D%s" % define for define in args.defines.split(';') if define]
  options = ["-D%s" % option for option in args.cmakeoptions.split(';') if option]
  api32, api64 = detectAndroidAPIs(args)
  if api32 is None or api64 is None:
    print('Failed to detect available platform APIs')
  print('Using API-%d for 32-bit builds, API-%d for 64-bit builds' % (api32, api64))

  if not cmake(args, buildDir, options + [
    '-G', 'Unix Makefiles',
    "-DCMAKE_TOOLCHAIN_FILE='%s/build/cmake/android.toolchain.cmake'" % args.androidndkpath,
    "-DCMAKE_SYSTEM_NAME=Android",
    "-DCMAKE_BUILD_TYPE=%s" % args.configuration,
    "-DCMAKE_MAKE_PROGRAM='%s'" % args.make,
    "-DWRAPPER_DIR=%s" % ('%s/generated/android-java/wrappers' % baseDir),
    "-DSINGLE_LIBRARY:BOOL=ON",
    "-DANDROID_STL='c++_static'",
    "-DANDROID_ABI='%s'" % abi,
    "-DANDROID_PLATFORM='%d'" % (api64 if '64' in abi else api32),
    "-DSDK_CPP_DEFINES=%s" % " ".join(defines),
    "-DSDK_VERSION='%s'" % version,
    "-DSDK_PLATFORM='Android'",
    '%s/scripts/build' % baseDir
  ]):
    return False
  if not cmake(args, buildDir, [
    '--build', '.',
    '--parallel', '4',
    '--config', args.configuration
  ]):
    return False
  return makedirs('%s/%s' % (distDir, abi)) and copyfile('%s/libcarto_mobile_sdk.so' % buildDir, '%s/%s/libcarto_mobile_sdk.so' % (distDir, abi))

def buildAndroidJAR(args):
  shutil.rmtree(getBuildDir('android_java'), True)

  baseDir = getBaseDir()
  buildDir = getBuildDir('android_java')
  distDir = getDistDir('android')
  apiJava = detectAndroidJavaAPI(args)
  if apiJava is None:
    print('Failed to detect available platform APIs')

  javaFiles = []
  for sourceDir in ["%s/generated/android-java/proxies" % baseDir, "%s/android/java" % baseDir]:
    for dirpath, dirnames, filenames in os.walk(sourceDir):
      for filename in [f for f in filenames if f.endswith(".java")]:
        if os.name == 'nt':
          javaFiles.append(os.path.join(dirpath, "*.java"))
          break
        javaFiles.append(os.path.join(dirpath, filename))

  if not javac(args, buildDir,
    '-g:vars',
    '-source', '1.7',
    '-target', '1.7',
    '-bootclasspath', '%s/scripts/android/rt.jar' % baseDir,
    '-classpath', '%s/platforms/android-%d/android.jar' % (args.androidsdkpath, apiJava),
    '-d', buildDir,
    *javaFiles
  ):
    return False

  currentDir = os.getcwd()
  os.chdir(buildDir)
  classFiles = []
  for dirpath, dirnames, filenames in os.walk("."):
    for filename in [f for f in filenames if f.endswith(".class")]:
      if os.name == 'nt':
        classFiles.append(os.path.join(dirpath[2:], "*.class"))
        break
      classFiles.append(os.path.join(dirpath[2:], filename))
  os.chdir(currentDir)

  if not jar(args, buildDir,
    'cf', 'carto-mobile-sdk.jar',
    *classFiles
  ):
    return False

  if makedirs(distDir) and \
     copyfile('%s/carto-mobile-sdk.jar' % buildDir, '%s/carto-mobile-sdk.jar' % distDir):
    print("Output available in:\n%s" % distDir)
    return True
  return False

def buildAndroidAAR(args):
  shutil.rmtree(getBuildDir('android-src'), True)

  baseDir = getBaseDir()
  srcDir = getBuildDir('android-src')
  buildDir = getBuildDir('android-aar')
  distDir = getDistDir('android')
  version = args.buildversion

  with open('%s/scripts/android-aar/carto-mobile-sdk.pom.template' % baseDir, 'r') as f:
    pomFile = string.Template(f.read()).safe_substitute({
      'baseDir': baseDir,
      'buildDir': buildDir,
      'distDir': distDir,
      'version': version
    })
  pomFileName = '%s/carto-mobile-sdk.pom' % buildDir
  with open(pomFileName, 'w') as f:
    f.write(pomFile)

  javaFiles = []
  for sourceDir in ["%s/generated/android-java/proxies" % baseDir, "%s/android/java" % baseDir]:
    for dirpath, dirnames, filenames in os.walk(sourceDir):
      relpath = os.path.relpath(dirpath, sourceDir)
      makedirs(os.path.join(srcDir, relpath))
      for filename in [f for f in filenames if f.endswith(".java")]:
        copyfile(os.path.join(dirpath, filename), os.path.join(srcDir, relpath, filename))

  srcFileName = '%s/sources.jar' % buildDir
  if not jar(args, srcDir,
    'cf', srcFileName, '.'):
    return False

  if not gradle(args, '%s/scripts' % baseDir,
    '-p', 'android-aar',
    '--project-cache-dir', buildDir,
    '--gradle-user-home', '%s/gradle' % buildDir,
    'assembleRelease'
  ):
    return False
  aarFileName = '%s/outputs/aar/carto-mobile-sdk-%s.aar' % (buildDir, args.configuration.lower())
  if not os.path.exists(aarFileName):
    aarFileName = '%s/outputs/aar/carto-mobile-sdk.aar' % buildDir
  if makedirs(distDir) and \
     copyfile(pomFileName, '%s/carto-mobile-sdk-%s.pom' % (distDir, version)) and \
     copyfile(aarFileName, '%s/carto-mobile-sdk-%s.aar' % (distDir, version)) and \
     copyfile(srcFileName, '%s/carto-mobile-sdk-%s-sources.jar' % (distDir, version)):
    zip(args, '%s/scripts/android-aar/src/main' % baseDir, '%s/carto-mobile-sdk-%s.aar' % (distDir, version), 'R.txt')
    print("Output available in:\n%s\n\nTo publish, use:\ngradle -p android-aar publishReleasePublicationToSonatypeRepository -Dbuild-version=%s\nThen log in to https://s01.oss.sonatype.org, 'Close' and then 'Release'.\n" % (distDir, version))
    return True
  return False

parser = argparse.ArgumentParser()
parser.add_argument('--profile', dest='profile', default=getDefaultProfileId(), type=validProfile, help='Build profile')
parser.add_argument('--android-abi', dest='androidabi', default=[], choices=ANDROID_ABIS + ['all'], action='append', help='Android target ABIs')
parser.add_argument('--android-ndk-path', dest='androidndkpath', default='auto', help='Android NDK path')
parser.add_argument('--android-sdk-path', dest='androidsdkpath', default='auto', help='Android SDK path')
parser.add_argument('--defines', dest='defines', default='', help='Defines for compilation')
parser.add_argument('--javac', dest='javac', default='javac', help='Java compiler executable')
parser.add_argument('--jar', dest='jar', default='jar', help='Jar executable')
parser.add_argument('--zip', dest='zip', default='zip', help='Zip executable')
parser.add_argument('--make', dest='make', default='make', help='Make executable')
parser.add_argument('--cmake', dest='cmake', default='cmake', help='CMake executable')
parser.add_argument('--cmake-options', dest='cmakeoptions', default='', help='CMake options')
parser.add_argument('--gradle', dest='gradle', default='gradle', help='Gradle executable')
parser.add_argument('--configuration', dest='configuration', default='Release', choices=['Release', 'RelWithDebInfo', 'Debug'], help='Configuration')
parser.add_argument('--build-number', dest='buildnumber', default='', help='Build sequence number, goes to version str')
parser.add_argument('--build-version', dest='buildversion', default='%s-devel' % SDK_VERSION, help='Build version, goes to distributions')
parser.add_argument('--build-aar', dest='buildaar', default=False, action='store_true', help='Build Android .aar package')
parser.add_argument('--build-jar', dest='buildjar', default=False, action='store_true', help='Build Android .jar package')
args = parser.parse_args()
if 'all' in args.androidabi or args.androidabi == []:
  args.androidabi = ANDROID_ABIS
if args.androidsdkpath == 'auto':
  args.androidsdkpath = os.environ.get('ANDROID_HOME', None)
  if args.androidsdkpath is None:
    print("ANDROID_HOME variable not set")
    sys.exit(-1)
if args.androidndkpath == 'auto':
  args.androidndkpath = os.environ.get('ANDROID_NDK_HOME', None)
  if args.androidndkpath is None:
    args.androidndkpath = os.path.join(args.androidsdkpath, 'ndk-bundle')
args.defines += ';' + getProfile(args.profile).get('defines', '')
args.cmakeoptions += ';' + getProfile(args.profile).get('cmake-options', '')

if not os.path.exists("%s/generated/android-java/proxies" % getBaseDir()):
  print("Proxies/wrappers not generated yet, run swigpp script first.")
  sys.exit(-1)

if not checkExecutable(args.cmake, '--help'):
  print('Failed to find CMake executable. Use --cmake to specify its location')
  sys.exit(-1)

if not checkExecutable(args.make, '--help'):
  print('Failed to find make executable. Use --make to specify its location')
  sys.exit(-1)

if not checkExecutable(args.javac, '-help'):
  print('Failed to find javac executable. Use --javac to specify its location')
  sys.exit(-1)

if args.buildaar:
  if not checkExecutable(args.zip, '-h'):
    print('Failed to find zip executable. Use --zip to specify its location')
    sys.exit(-1)
  if not checkExecutable(args.gradle, '--help'):
    print('Failed to find gradle executable. Use --gradle to specify its location')
    sys.exit(-1)

for abi in args.androidabi:
  if not buildAndroidSO(args, abi):
    sys.exit(-1)

if args.buildjar:
  if not buildAndroidJAR(args):
    sys.exit(-1)

if args.buildaar:
  if not buildAndroidAAR(args):
    sys.exit(-1)
