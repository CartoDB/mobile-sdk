import os
import sys
import re
import shutil
import argparse
import string
from build.sdk_build_utils import *

IOS_ARCHS = ['i386', 'x86_64', 'armv7', 'arm64']

def updateUmbrellaHeader(filename, args):
  with open(filename, 'r') as f:
    lines = f.readlines()
    for i in range(0, len(lines)):
      match = re.search('^\s*#import\s+"(.*)".*', lines[i].rstrip('\n'))
      if match:
        lines[i] = '#import <CartoMobileSDK/%s>\n' % match.group(1)
    for i in range(0, len(lines)):
      if re.search('^\s*#define\s+.*$', lines[i].rstrip('\n')):
        break
    lines = lines[:i+1] + ['\n'] + ['#define %s\n' % define for define in args.defines.split(';') if define] + lines[i+1:]
  with open(filename, 'w') as f:
    f.writelines(lines)

def updatePrivateHeader(filename, args):
  with open(filename, 'r') as f:
    lines = f.readlines()
    for i in range(0, len(lines)):
      match = re.search('^\s*#include\s+"(.*)".*', lines[i].rstrip('\n'))
      if match:
        lines[i] = '#include <CartoMobileSDK/%s>\n' % match.group(1)
      match = re.search('^\s*#import\s+"(.*)".*', lines[i].rstrip('\n'))
      if match:
        lines[i] = '#import <CartoMobileSDK/%s>\n' % match.group(1)
  with open(filename, 'w') as f:
    f.writelines(lines)

def buildModuleMap(filename, publicHeaders, privateHeaders):
  with open(filename, 'w') as f:
    f.write('framework module CartoMobileSDK {\n')
    f.write('    umbrella header "CartoMobileSDK.h"\n')
    for header in publicHeaders:
      f.write('    header "%s"\n' % header)
    f.write('    export *\n')
    f.write('    module * { export * }\n')
    f.write('    explicit module Private {\n')
    f.write('        requires cplusplus\n')
    for header in privateHeaders:
      f.write('        header "%s"\n' % header)
    f.write('    }\n')
    f.write('}\n')

def buildIOSLib(args, arch):
  platform = 'OS' if arch.startswith('arm') else 'SIMULATOR'
  version = getVersion(args.buildnumber) if args.configuration == 'Release' else 'Devel'
  baseDir = getBaseDir()
  buildDir = getBuildDir('ios', '%s-%s' % (platform, arch))
  defines = ["-D%s" % define for define in args.defines.split(';') if define]
  options = ["-D%s" % option for option in args.cmakeoptions.split(';') if option]

  if not cmake(args, buildDir, options + [
    '-G', 'Xcode',
    '-DCMAKE_SYSTEM_NAME=iOS',
    '-DWRAPPER_DIR=%s' % ('%s/generated/ios-objc/proxies' % baseDir),
    '-DINCLUDE_OBJC:BOOL=ON',
    '-DSINGLE_LIBRARY:BOOL=ON',
    '-DENABLE_BITCODE:BOOL=%s' % ('OFF' if args.stripbitcode else 'ON'),
    '-DSHARED_LIBRARY:BOOL=%s' % ('ON' if args.sharedlib else 'OFF'),
    '-DCMAKE_OSX_ARCHITECTURES=%s' % arch,
    '-DCMAKE_OSX_SYSROOT=iphone%s' % platform.lower(),
    '-DCMAKE_OSX_DEPLOYMENT_TARGET=%s' % ('9.0' if args.metalangle else '7.0'),
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
  if args.sharedlib:
    outputDir = '%s/CartoMobileSDK.framework' % distDir
  else:
    outputDir = '%s/CartoMobileSDK.framework/Versions/A' % distDir
  makedirs(outputDir)

  libFilePaths = []
  for platform, arch in platformArchs:
    libFilePath = "%s/%s-%s/libcarto_mobile_sdk.%s" % (getBuildDir('ios', '%s-%s' % (platform, arch)), args.configuration, 'iphoneos' if arch.startswith("arm") else 'iphonesimulator', 'dylib' if args.sharedlib else 'a')
    if args.metalangle:
      mergedLibFilePath = '%s_merged.%s' % tuple(libFilePath.rsplit('.', 1))
      angleLibFilePath = "%s/libs-external/angle-metal/%s/libangle.a" % (baseDir, arch)
      if not execute('libtool', baseDir,
        '-o', mergedLibFilePath, libFilePath, angleLibFilePath
      ):
        return False
      libFilePath = mergedLibFilePath
    libFilePaths.append(libFilePath)

  if not execute('lipo', baseDir,
    '-output', '%s/CartoMobileSDK' % outputDir,
    '-create', *libFilePaths
  ):
    return False

  if not copyfile('%s/scripts/ios/Info.plist' % baseDir, '%s/Info.plist' % outputDir):
      return False

  if args.sharedlib:
    if not execute('install_name_tool', outputDir,
      '-id', '@rpath/CartoMobileSDK.framework/CartoMobileSDK',
      'CartoMobileSDK'
    ):
      return False
    

  makedirs('%s/Headers' % outputDir)
  if not args.sharedlib:
    if not makesymlink('%s/CartoMobileSDK.framework/Versions' % distDir, 'A', 'Current'):
      return False
    if not makesymlink('%s/CartoMobileSDK.framework' % distDir, 'Versions/A/Modules', 'Modules'):
      return False
    if not makesymlink('%s/CartoMobileSDK.framework' % distDir, 'Versions/A/Headers', 'Headers'):
      return False
    if not makesymlink('%s/CartoMobileSDK.framework' % distDir, 'Versions/A/PrivateHeaders', 'PrivateHeaders'):
      return False
    if not makesymlink('%s/CartoMobileSDK.framework' % distDir, 'Versions/A/CartoMobileSDK', 'CartoMobileSDK'):
      return False

  publicHeaders = []
  privateHeaders = []
  headerDirTemplates = ['%s/all/native', '%s/ios/native', '%s/ios/objc', '%s/generated/ios-objc/proxies', '%s/libs-external/cglib']
  if args.metalangle:
    headerDirTemplates.append('%s/libs-external/angle-metal/include')
  for headerDirTemplate in headerDirTemplates:
    headerDir = headerDirTemplate % baseDir
    if not os.path.exists(headerDir):
      continue
    currentDir = os.getcwd()
    os.chdir(headerDir)
    for dirpath, dirnames, filenames in os.walk('.'):
      for filename in filenames:
        if filename.endswith('.h'):
          destDir = '%s/Headers/%s' % (outputDir, dirpath)
          if headerDirTemplate.find('objc') == -1:
            destDir = '%s/PrivateHeaders/%s' % (outputDir, dirpath)
            privateHeaders.append(os.path.normpath(os.path.join(dirpath, filename)))
          elif filename != 'CartoMobileSDK.h':
            publicHeaders.append(os.path.normpath(os.path.join(dirpath, filename)))
          if not (makedirs(destDir) and copyfile(os.path.join(dirpath, filename), '%s/%s' % (destDir, filename))):
            os.chdir(currentDir)
            return False
          if filename == 'CartoMobileSDK.h':
            updateUmbrellaHeader('%s/%s' % (destDir, filename), args)
          else:
            updatePrivateHeader('%s/%s' % (destDir, filename), args)
    os.chdir(currentDir)
  makedirs('%s/Modules' % outputDir)
  buildModuleMap('%s/Modules/module.modulemap' % outputDir, publicHeaders, privateHeaders)

  print("Output available in:\n%s" % distDir)
  return True

def buildIOSCocoapod(args, buildpackage):
  baseDir = getBaseDir()
  distDir = getDistDir('ios')
  version = args.buildversion
  distName = 'sdk4-ios-%s.zip' % version
  iosversion = '9.0' if args.metalangle else '7.0'
  frameworks = (["IOSurface"] if args.metalangle else ["OpenGLES", "GLKit"]) + ["UIKit", "CoreGraphics", "CoreText", "CFNetwork", "Foundation", "CartoMobileSDK"]

  with open('%s/scripts/ios-cocoapod/Akylas-CartoMobileSDK.podspec.template' % baseDir, 'r') as f:
    cocoapodFile = string.Template(f.read()).safe_substitute({ 'baseDir': baseDir, 'distDir': distDir, 'distName': distName, 'version': version, 'iosversion': iosversion, 'frameworks': ', '.join('"%s"' % framework for framework in frameworks) })
  with open('%s/CartoMobileSDK.podspec' % distDir, 'w') as f:
    f.write(cocoapodFile)

  if buildpackage:
    try:
      os.remove('%s/%s' % (distDir, distName))
    except:
      pass
    if not execute('zip', distDir, '-y', '-r', distName, 'CartoMobileSDK.framework'):
      return False
    print("Output available in:\n%s\n\nTo publish, use:\ncd %s\naws s3 cp %s s3://nutifront/sdk_snapshots/%s --grants read=uri=http://acs.amazonaws.com/groups/global/AllUsers\npod trunk push\n" % (distDir, distDir, distName, distName))
  return True

parser = argparse.ArgumentParser()
parser.add_argument('--profile', dest='profile', default=getDefaultProfileId(), type=validProfile, help='Build profile')
parser.add_argument('--ios-arch', dest='iosarch', default=[], choices=IOS_ARCHS + ['all'], action='append', help='iOS target architectures')
parser.add_argument('--defines', dest='defines', default='', help='Defines for compilation')
parser.add_argument('--cmake', dest='cmake', default='cmake', help='CMake executable')
parser.add_argument('--cmake-options', dest='cmakeoptions', default='', help='CMake options')
parser.add_argument('--configuration', dest='configuration', default='Release', choices=['Release', 'RelWithDebInfo', 'Debug'], help='Configuration')
parser.add_argument('--build-number', dest='buildnumber', default='', help='Build sequence number, goes to version str')
parser.add_argument('--build-version', dest='buildversion', default='%s-devel' % SDK_VERSION, help='Build version, goes to distributions')
parser.add_argument('--build-cocoapod', dest='buildcocoapod', default=False, action='store_true', help='Build CocoaPod')
parser.add_argument('--build-cocoapod-package', dest='buildcocoapodpackage', default=False, action='store_true', help='Build CocoaPod')
parser.add_argument('--metalangle', dest='metalangle', default=False, action='store_true', help='Use MetalANGLE instead of Apple GL')
parser.add_argument('--strip-bitcode', dest='stripbitcode', default=False, action='store_true', help='Strip bitcode from the built framework')
parser.add_argument('--shared-framework', dest='sharedlib', default=False, action='store_true', help='Build shared framework instead of static')

args = parser.parse_args()
if 'all' in args.iosarch or args.iosarch == []:
  args.iosarch = IOS_ARCHS
args.defines += ';' + getProfile(args.profile).get('defines', '')
if args.metalangle:
  args.defines += ';' + '_CARTO_USE_METALANGLE'
args.cmakeoptions += ';' + getProfile(args.profile).get('cmake-options', '')

if not checkExecutable(args.cmake, '--help'):
  print('Failed to find CMake executable. Use --cmake to specify its location')
  sys.exit(-1)

for arch in args.iosarch:
  if not buildIOSLib(args, arch):
    sys.exit(-1)

if not buildIOSFramework(args, args.iosarch):
  sys.exit(-1)

if args.buildcocoapod or args.buildcocoapodpackage:
  if not buildIOSCocoapod(args, args.buildcocoapodpackage):
    sys.exit(-1)
