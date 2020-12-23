import os
import sys
import argparse
import string
from build.sdk_build_utils import *

ANDROID_ABIS = ['armeabi-v7a', 'x86', 'arm64-v8a', 'x86_64']
IOS_ARCHS = ['i386', 'x86_64', 'armv7', 'arm64']

def msbuild(args, dir, *cmdArgs):
  return execute(args.msbuild, dir, *cmdArgs)

def nuget(args, dir, *cmdArgs):
  return execute(args.nuget, dir, *cmdArgs)

def detectAndroidAPIs(args):
  api32, api64 = None, None  
  with open('%s/meta/platforms.json' % args.androidndkpath, 'rb') as f:
    platforms = json.load(f)
    minapi = platforms.get('min', 1)
    maxapi = platforms.get('max', 0)
    for api in range(minapi, maxapi + 1):
      if api >= 9:
        api32 = min(api32 or api, api)
      if api >= 21:
        api64 = min(api64 or api, api)
  return api32, api64

def buildAndroidSO(args, abi):
  version = getVersion(args.buildnumber) if args.configuration == 'Release' else 'Devel'
  baseDir = getBaseDir()
  buildDir = getBuildDir('xamarin_android', abi)
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
    "-DWRAPPER_DIR=%s" % ('%s/generated/android-csharp/wrappers' % baseDir),
    "-DANDROID_STL='c++_static'",
    "-DANDROID_ABI='%s'" % abi,
    "-DANDROID_PLATFORM='%d'" % (api64 if '64' in abi else api32),
    "-DSDK_CPP_DEFINES=%s" % " ".join(defines),
    "-DSDK_VERSION='%s'" % version,
    "-DSDK_PLATFORM='Xamarin Android'",
    '%s/scripts/build' % baseDir
  ]):
    return False
  return cmake(args, buildDir, [
    '--build', '.', '--', '-j4'
  ])

def buildIOSLib(args, arch):
  version = getVersion(args.buildnumber) if args.configuration == 'Release' else 'Devel'
  platform = 'OS' if arch.startswith('arm') else 'SIMULATOR'
  baseDir = getBaseDir()
  buildDir = getBuildDir('xamarin_ios', '%s-%s' % (platform, arch))
  defines = ["-D%s" % define for define in args.defines.split(';') if define]
  options = ["-D%s" % option for option in args.cmakeoptions.split(';') if option]

  if not cmake(args, buildDir, options + [
    '-G', 'Xcode',
    '-DCMAKE_SYSTEM_NAME=iOS',
    '-DWRAPPER_DIR=%s' % ('%s/generated/ios-csharp/wrappers' % baseDir),
    '-DINCLUDE_OBJC:BOOL=OFF',
    '-DSINGLE_LIBRARY:BOOL=ON',
    '-DENABLE_BITCODE:BOOL=OFF',
    '-DCMAKE_OSX_ARCHITECTURES=%s' % arch,
    '-DCMAKE_OSX_SYSROOT=iphone%s' % platform.lower(),
    '-DCMAKE_OSX_DEPLOYMENT_TARGET=7.0',
    '-DCMAKE_BUILD_TYPE=%s' % args.configuration,
    "-DSDK_CPP_DEFINES=%s" % " ".join(defines),
    "-DSDK_VERSION='%s'" % version,
    "-DSDK_PLATFORM='Xamarin iOS'",
    '%s/scripts/build' % baseDir
  ]):
    return False
  return cmake(args, buildDir, [
    '--build', '.',
    '--config', args.configuration
  ])

def buildIOSFatLib(args, archs):
  platformArchs = [('OS' if arch.startswith('arm') else 'SIMULATOR', arch) for arch in archs]
  baseDir = getBaseDir()
  buildDir = getBuildDir('xamarin_ios_unified')

  return execute('lipo', baseDir,
    '-output', '%s/libcarto_mobile_sdk.a' % buildDir,
    '-create', *["%s/%s-%s/libcarto_mobile_sdk.a" % (getBuildDir('xamarin_ios', '%s-%s' % (platform, arch)), args.configuration, 'iphoneos' if arch.startswith("arm") else "iphonesimulator") for platform, arch in platformArchs]
  )

def buildXamarinDLL(args, target):
  baseDir = getBaseDir()
  buildDir = getBuildDir('xamarin_%s' % target)
  distDir = getDistDir('xamarin')

  with open('%s/scripts/xamarin/CartoMobileSDK.%s.csproj.template' % (baseDir, target), 'r') as f:
    csProjFile = string.Template(f.read()).safe_substitute({ 'baseDir': baseDir, 'buildDir': buildDir, 'distDir': "%s/bin" % buildDir })
  with open('%s/CartoMobileSDK.%s.csproj' % (buildDir, target), 'w') as f:
    f.write(csProjFile)

  if args.msbuild is None:
    print("%s solution is in %s, please use Xamarin IDE to compile target DLL" % (target, buildDir))
    return True
    
  if not msbuild(args, buildDir,
    '/t:Build',
    '/p:Configuration=%s' % args.configuration,
    '/p:AndroidSdkDirectory=%s' % args.androidsdkpath,
    '%s/CartoMobileSDK.%s.csproj' % (buildDir, target)
  ):
    return False
  return makedirs(distDir) and \
    copyfile('%s/bin/%s/CartoMobileSDK.%s.dll' % (buildDir, args.configuration, target), '%s/CartoMobileSDK.%s.dll' % (distDir, target)) and \
    copyfile('%s/bin/%s/CartoMobileSDK.%s.xml' % (buildDir, args.configuration, target), '%s/CartoMobileSDK.%s.xml' % (distDir, target))

def buildXamarinNuget(args, target):
  baseDir = getBaseDir()
  buildDir = getBuildDir('xamarin_nuget')
  distDir = getDistDir('xamarin')
  version = args.buildversion

  with open('%s/scripts/nuget/CartoMobileSDK.%s.nuspec.template' % (baseDir, target), 'r') as f:
    nuspecFile = string.Template(f.read()).safe_substitute({ 'baseDir': baseDir, 'buildDir': buildDir, 'configuration': args.configuration, 'nativeConfiguration': args.nativeconfiguration, 'version': version })
  with open('%s/CartoMobileSDK.%s.nuspec' % (buildDir, target), 'w') as f:
    f.write(nuspecFile)

  if not nuget(args, buildDir,
    'pack',
    '%s/CartoMobileSDK.%s.nuspec' % (buildDir, target),
    '-BasePath', '/'
  ):
    return False

  if not copyfile('%s/CartoMobileSDK.%s.%s.nupkg' % (buildDir, target, version), '%s/CartoMobileSDK.%s.%s.nupkg' % (distDir, target, version)):
    return False
  print("Output available in:\n%s\n\nTo publish, use:\nnuget push %s/CartoMobileSDK.%s.%s.nupkg -Source https://www.nuget.org/api/v2/package\n" % (distDir, distDir, target, version))
  return True

parser = argparse.ArgumentParser()
parser.add_argument('--profile', dest='profile', default='standard', type=validProfile, help='Build profile')
parser.add_argument('--android-abi', dest='androidabi', default=[], choices=ANDROID_ABIS + ['all'], action='append', help='Android target ABIs')
parser.add_argument('--ios-arch', dest='iosarch', default=[], choices=IOS_ARCHS + ['all'], action='append', help='iOS target architectures')
parser.add_argument('--defines', dest='defines', default='', help='Defines for compilation')
parser.add_argument('--msbuild', dest='msbuild', default='msbuild', help='Xamarin msbuild executable')
parser.add_argument('--nuget', dest='nuget', default='nuget', help='nuget executable')
parser.add_argument('--android-ndk-path', dest='androidndkpath', default='auto', help='Android NDK path')
parser.add_argument('--android-sdk-path', dest='androidsdkpath', default='auto', help='Android SDK path')
parser.add_argument('--make', dest='make', default='make', help='Make executable for Android')
parser.add_argument('--cmake', dest='cmake', default='cmake', help='CMake executable')
parser.add_argument('--cmake-options', dest='cmakeoptions', default='', help='CMake options')
parser.add_argument('--configuration', dest='configuration', default='Release', choices=['Release', 'RelWithDebInfo', 'Debug'], help='Configuration')
parser.add_argument('--build-number', dest='buildnumber', default='', help='Build sequence number, goes to version str')
parser.add_argument('--build-version', dest='buildversion', default='%s-devel' % SDK_VERSION, help='Build version, goes to distributions')
parser.add_argument('--build-nuget', dest='buildnuget', default=False, action='store_true', help='Build Nuget package')
parser.add_argument(dest='target', choices=['android', 'ios'], help='Target platform')

args = parser.parse_args()
if 'all' in args.androidabi or args.androidabi == []:
  args.androidabi = ANDROID_ABIS
if 'all' in args.iosarch or args.iosarch == []:
  args.iosarch = IOS_ARCHS
if args.msbuild == 'none':
  args.msbuild = None
if args.androidsdkpath == 'auto' and args.target == 'android':
  args.androidsdkpath = os.environ.get('ANDROID_HOME', None)
  if args.androidsdkpath is None:
    print("ANDROID_HOME variable not set")
    exit(-1)
if args.androidndkpath == 'auto' and args.target == 'android':
  args.androidndkpath = os.environ.get('ANDROID_NDK_HOME', None)
  if args.androidndkpath is None:
    args.androidndkpath = os.path.join(args.androidsdkpath, 'ndk-bundle')
args.defines += ';' + getProfile(args.profile).get('defines', '')
args.defines += ';TARGET_XAMARIN'
args.cmakeoptions += ';' + getProfile(args.profile).get('cmake-options', '')
args.nativeconfiguration = args.configuration

if not checkExecutable(args.cmake, '--help'):
  print('Failed to find CMake executable. Use --cmake to specify its location')
  sys.exit(-1)

if args.target == 'android' and not checkExecutable(args.make, '--help'):
  print('Failed to find make executable. Use --make to specify its location')
  sys.exit(-1)

if not checkExecutable(args.msbuild, '/?'):
  print('Failed to find msbuild executable. Use --msbuild to specify its location')
  sys.exit(-1)

if args.buildnuget:
  if not checkExecutable(args.nuget, 'help'):
    print('Failed to find nuget executable. Use --nuget to specify its location')
    sys.exit(-1)

target = None
if args.target == 'android':
  target = 'Android'
  for abi in args.androidabi:
    if not buildAndroidSO(args, abi):
      exit(-1)
elif args.target == 'ios':
  target = 'iOS'
  for arch in args.iosarch:
    if not buildIOSLib(args, arch):
      exit(-1)
  if not buildIOSFatLib(args, args.iosarch):
    exit(-1)

if not buildXamarinDLL(args, target):
  exit(-1)

if args.buildnuget:
  if not buildXamarinNuget(args, target):
    exit(-1)
