import os
import re
import shutil
import argparse
import string
from build.sdk_build_utils import *

IOS_ARCHS = ['i386', 'x86_64', 'armv7', 'arm64']

def updateUmbrellaHeaderDefinitions(filename, args):
  with open(filename, 'r') as f:
    lines = f.readlines()
    for i in range(0, len(lines)):
      if re.search('^\s*#define\s+.*$', lines[i].rstrip('\n')):
        break
    lines = lines[:i+1] + ['\n'] + ['#define %s\n' % define for define in args.defines.split(';') if define] + lines[i+1:]
  with open(filename, 'w') as f:
    f.writelines(lines)

def buildIOSLib(args, arch):
  platform = 'OS' if arch.startswith('arm') else 'SIMULATOR'
  version = getVersion(args.buildnumber) if args.configuration == 'Release' else 'Devel'
  baseDir = getBaseDir()
  buildDir = getBuildDir('ios', '%s-%s' % (platform, arch))
  defines = ["-D%s" % define for define in args.defines.split(';') if define]
  options = ["-D%s" % option for option in args.cmakeoptions.split(';') if option]

  if not cmake(args, buildDir, options + [
    '-G', 'Xcode',
    '-DCMAKE_TOOLCHAIN_FILE=%s/scripts/ios-cmake/iOS.cmake' % baseDir,
    '-DIOS_PLATFORM=%s' % platform,
    '-DWRAPPER_DIR=%s' % ('%s/generated/ios-objc/proxies' % baseDir),
    '-DINCLUDE_OBJC:BOOL=ON',
    '-DSINGLE_LIBRARY:BOOL=ON',
    '-DCMAKE_OSX_ARCHITECTURES=%s' % arch,
    '-DCMAKE_BUILD_TYPE=%s' % args.configuration,
    "-DSDK_CPP_DEFINES=%s" % " ".join(defines),
    "-DSDK_VERSION='%s'" % version,
    "-DSDK_PLATFORM='iOS'",
    '%s/scripts/build' % baseDir
  ]):
    return False
  return cmake(args, buildDir, [
    '--build', '.',
    '--config', args.configuration
  ])

def buildIOSFramework(args, archs):
  shutil.rmtree(getDistDir('ios'), True)

  platformArchs = [('OS' if arch.startswith('arm') else 'SIMULATOR', arch) for arch in archs]
  baseDir = getBaseDir()
  distDir = getDistDir('ios')
  outputDir = '%s/CartoMobileSDK.framework/Versions/A' % distDir
  makedirs(outputDir)

  if not execute('lipo', baseDir,
    '-output', '%s/CartoMobileSDK' % outputDir,
    '-create', *["%s/%s-%s/libcarto_mobile_sdk.a" % (getBuildDir('ios', '%s-%s' % (platform, arch)), args.configuration, 'iphoneos' if arch.startswith("arm") else "iphonesimulator") for platform, arch in platformArchs]
  ):
    return False

  makedirs('%s/Headers' % outputDir)
  if not makesymlink('%s/CartoMobileSDK.framework/Versions' % distDir, 'A', 'Current'):
    return False
  if not makesymlink('%s/CartoMobileSDK.framework' % distDir, 'Versions/A/Headers', 'Headers'):
    return False
  if not makesymlink('%s/CartoMobileSDK.framework' % distDir, 'Versions/A/CartoMobileSDK', 'CartoMobileSDK'):
    return False

  for dir in ['%s/all/native', '%s/extensions/native', '%s/ios/native', '%s/ios/objc', '%s/generated/ios-objc/proxies', '%s/libs-external/cglib']:
    if not os.path.exists(dir % baseDir):
      continue
    currentDir = os.getcwd()
    os.chdir(dir % baseDir)
    for dirpath, dirnames, filenames in os.walk('.'):
      for filename in filenames:
        if filename.endswith('.h'):
          destDir = '%s/Headers/%s' % (outputDir, dirpath)
          if not (makedirs(destDir) and copyfile(os.path.join(dirpath, filename), '%s/%s' % (destDir, filename))):
            os.chdir(currentDir)
            return False
          if filename == 'CartoMobileSDK.h':
            updateUmbrellaHeaderDefinitions('%s/%s' % (destDir, filename), args)
    os.chdir(currentDir)
  print "Output available in:\n%s" % distDir
  return True

def buildIOSCocoapod(args, buildpackage):
  baseDir = getBaseDir()
  distDir = getDistDir('ios')
  version = args.buildversion
  distName = 'sdk4-ios-%s.zip' % version
  
  with open('%s/extensions/scripts/ios-cocoapod/CartoMobileSDK.podspec.template' % baseDir, 'r') as f:
    cocoapodFile = string.Template(f.read()).safe_substitute({ 'baseDir': baseDir, 'distDir': distDir, 'distName': distName, 'version': version })
  with open('%s/CartoMobileSDK.podspec' % distDir, 'w') as f:
    f.write(cocoapodFile)

  if buildpackage:
    try:
      os.remove('%s/%s' % (distDir, distName))
    except:
      pass
    if not execute('zip', distDir, '-y', '-r', distName, 'CartoMobileSDK.framework'):
      return False
    print "Output available in:\n%s\n\nTo publish, use:\ncd %s\naws s3 cp %s s3://nutifront/sdk_snapshots/%s\npod trunk push\n" % (distDir, distDir, distName, distName)

parser = argparse.ArgumentParser()
parser.add_argument('--profile', dest='profile', default=getDefaultProfile(), choices=getProfiles().keys(), help='Build profile')
parser.add_argument('--ios-arch', dest='iosarch', default=['all'], choices=IOS_ARCHS + ['all'], nargs='+', help='Windows phone target architectures')
parser.add_argument('--defines', dest='defines', default='', help='Defines for compilation')
parser.add_argument('--cmake', dest='cmake', default='cmake', help='CMake executable')
parser.add_argument('--cmake-options', dest='cmakeoptions', default='', help='CMake options')
parser.add_argument('--configuration', dest='configuration', default='Release', choices=['Release', 'Debug'], help='Configuration')
parser.add_argument('--build-number', dest='buildnumber', default='', help='Build sequence number, goes to version str')
parser.add_argument('--build-version', dest='buildversion', default='%s-devel' % SDK_VERSION, help='Build version, goes to distributions')
parser.add_argument('--build-cocoapod', dest='buildcocoapod', default=False, action='store_true', help='Build CocoaPod')
parser.add_argument('--build-cocoapod-package', dest='buildcocoapodpackage', default=False, action='store_true', help='Build CocoaPod')

args = parser.parse_args()
if 'all' in args.iosarch:
  args.iosarch = IOS_ARCHS
args.defines += ';' + getProfiles()[args.profile].get('defines', '')
args.cmakeoptions += ';' + getProfiles()[args.profile].get('cmake-options', '')

for arch in args.iosarch:
  if not buildIOSLib(args, arch):
    exit(-1)

if not buildIOSFramework(args, args.iosarch):
  exit(-1)

if args.buildcocoapod or args.buildcocoapodpackage:
  if not buildIOSCocoapod(args, args.buildcocoapodpackage):
    exit(-1)
