import os
import sys
import re
import shutil
import itertools
import argparse
import string
from build.sdk_build_utils import *

IOS_ARCHS = ['i386', 'x86_64', 'armv7', 'arm64']

def updateUmbrellaHeader(filename, defines):
  with open(filename, 'r') as f:
    lines = f.readlines()
    for i in range(0, len(lines)):
      match = re.search('^\s*#import\s+"(.*)".*', lines[i].rstrip('\n'))
      if match:
        headerFilename = match.group(1).split('/')[-1]
        if not headerFilename.startswith('NT'):
          headerFilename = 'NT%s' % headerFilename
        lines[i] = '#import <CartoMobileSDK/%s>\n' % headerFilename
    for i in range(0, len(lines)):
      if re.search('^\s*#define\s+.*$', lines[i].rstrip('\n')):
        break
    lines = lines[:i+1] + ['\n'] + ['#define %s\n' % define for define in defines.split(';') if define] + lines[i+1:]
  with open(filename, 'w') as f:
    f.writelines(lines)

def updatePublicHeader(filename):
  with open(filename, 'r') as f:
    lines = f.readlines()
    externCMode = False
    for i in range(0, len(lines)):
      if lines[i].find('extern "C"') != -1:
        externCMode = True
      match = re.search('^\s*#import\s+"(.*)".*', lines[i].rstrip('\n'))
      if match:
        headerFilename = match.group(1)
        if externCMode:
          lines[i] = '#ifdef __cplusplus\n}\n#endif\n#import "%s"\n#ifdef __cplusplus\nextern "C" {\n#endif\n' % headerFilename
        else:
          lines[i] = '#import "%s"\n' % headerFilename
  with open(filename, 'w') as f:
    f.writelines(lines)

def buildModuleMap(filename, publicHeaders):
  with open(filename, 'w') as f:
    f.write('framework module CartoMobileSDK {\n')
    f.write('    umbrella header "CartoMobileSDK.h"\n')
    for header in publicHeaders:
      f.write('    header "%s"\n' % header)
    f.write('    export *\n')
    f.write('    module * { export * }\n')
    f.write('}\n')

def copyHeaders(args, baseDir, outputDir):
  proxyHeaderDir = '%s/generated/ios-objc/proxies' % baseDir
  destDir = '%s/Headers' % outputDir
  publicHeaders = []
  makedirs(destDir) 

  currentDir = os.getcwd()
  os.chdir(proxyHeaderDir)
  for dirpath, dirnames, filenames in os.walk('.'):
    for filename in filenames:
      if filename.endswith('.h'):
        publicHeaders.append(filename)
        if not copyfile(os.path.join(dirpath, filename), '%s/%s' % (destDir, filename)):
          os.chdir(currentDir)
          return False
        updatePublicHeader('%s/%s' % (destDir, filename))
  os.chdir(currentDir)

  extraHeaders = ['%s/ios/objc/utils/ExceptionWrapper.h', '%s/ios/objc/ui/MapView.h']
  if args.metalangle:
    for extraHeader in ['MGLKit.h', 'MGLKitPlatform.h', 'MGLContext.h', 'MGLKView.h', 'MGLLayer.h', 'MGLKViewController.h']:
      extraHeaders += ['%s/libs-external/angle-metal/include/' + extraHeader]
  for extraHeader in extraHeaders:
    dirpath, filename = (extraHeader % baseDir).rsplit('/', 1)
    destFilename = filename if filename.startswith('MGL') else 'NT%s' % filename
    publicHeaders.append(destFilename)
    if not copyfile(os.path.join(dirpath, filename), '%s/%s' % (destDir, destFilename)):
      return False  

  if not copyfile('%s/ios/objc/CartoMobileSDK.h' % baseDir, '%s/CartoMobileSDK.h' % destDir):
    return False
  updateUmbrellaHeader('%s/CartoMobileSDK.h' % destDir, args.defines)

  makedirs('%s/Modules' % outputDir)
  buildModuleMap('%s/Modules/module.modulemap' % outputDir, publicHeaders)
  return True

def buildIOSLib(args, arch, outputDir=None):
  platform = 'OS' if arch.startswith('arm') else 'SIMULATOR'
  version = getVersion(args.buildnumber) if args.configuration == 'Release' else 'Devel'
  baseDir = getBaseDir()
  buildDir = outputDir or getBuildDir('ios', '%s-%s' % (platform, arch))
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
    '-DCMAKE_OSX_DEPLOYMENT_TARGET=9.0',
    '-DCMAKE_BUILD_TYPE=%s' % args.configuration,
    "-DSDK_CPP_DEFINES=%s" % " ".join(defines),
    "-DSDK_DEV_TEAM='%s'" % (args.devteam if args.devteam else ""),
    "-DSDK_VERSION='%s'" % version,
    "-DSDK_PLATFORM='iOS'",
    '%s/scripts/build' % baseDir
  ]):
    return False
  return cmake(args, buildDir, [
    '--build', '.',
    '--parallel', '4',
    '--config', args.configuration
  ])

def buildIOSFramework(args, archs, outputDir=None):
  platformArchs = [('OS' if arch.startswith('arm') else 'SIMULATOR', arch) for arch in archs]
  baseDir = getBaseDir()
  distDir = outputDir or getDistDir('ios')
  if args.sharedlib:
    frameworkDir = '%s/CartoMobileSDK.framework' % distDir
  else:
    frameworkDir = '%s/CartoMobileSDK.framework/Versions/A' % distDir
  shutil.rmtree(distDir, True)
  makedirs(frameworkDir)

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
    '-output', '%s/CartoMobileSDK' % frameworkDir,
    '-create', *libFilePaths
  ):
    return False

  if args.sharedlib:
    if not execute('install_name_tool', frameworkDir,
      '-id', '@rpath/CartoMobileSDK.framework/CartoMobileSDK',
      'CartoMobileSDK'
    ):
      return False
    if not copyfile('%s/scripts/ios/Info.plist' % baseDir, '%s/Info.plist' % frameworkDir):
      return False

  makedirs('%s/Headers' % frameworkDir)
  if not args.sharedlib:
    if not makesymlink('%s/CartoMobileSDK.framework/Versions' % distDir, 'A', 'Current'):
      return False
    if not makesymlink('%s/CartoMobileSDK.framework' % distDir, 'Versions/A/Modules', 'Modules'):
      return False
    if not makesymlink('%s/CartoMobileSDK.framework' % distDir, 'Versions/A/Headers', 'Headers'):
      return False
    if not makesymlink('%s/CartoMobileSDK.framework' % distDir, 'Versions/A/CartoMobileSDK', 'CartoMobileSDK'):
      return False
  if not copyHeaders(args, baseDir, frameworkDir):
    return False

  if outputDir is None:
    print("Output available in:\n%s" % distDir)
  return True

def buildIOSXCFramework(args, archs, outputDir=None):
  platformArchs = [('OS' if arch.startswith('arm') else 'SIMULATOR', arch) for arch in archs]
  groupedPlatformArchs = {}
  for platform, arch in platformArchs:
    groupedPlatformArchs[platform] = groupedPlatformArchs.get(platform, []) + [arch]
  baseDir = getBaseDir()
  distDir = outputDir or getDistDir('ios')
  shutil.rmtree(distDir, True)
  makedirs(distDir)

  frameworkBuildDirs = []
  for platform, archs in groupedPlatformArchs.items():
    frameworkBuildDir = getBuildDir('ios-framework', '%s-%s' % (platform, '-'.join(archs)))
    if not buildIOSFramework(args, archs, frameworkBuildDir):
      return False
    frameworkBuildDirs.append(frameworkBuildDir)

  if not execute('xcodebuild', baseDir,
    '-create-xcframework', '-output', '%s/CartoMobileSDK.xcframework' % distDir,
    *list(itertools.chain(*[['-framework', '%s/CartoMobileSDK.framework' % frameworkBuildDir] for frameworkBuildDir in frameworkBuildDirs]))
  ):
    return False

  if outputDir is None:
    print("Output available in:\n%s" % distDir)
  return True

def buildIOSCocoapod(args, buildpackage):
  baseDir = getBaseDir()
  distDir = getDistDir('ios')
  version = args.buildversion
  distName = 'sdk4-ios-%s%s.zip' % (("metal-" if args.metalangle else ""), version)
  frameworkName = 'CartoMobileSDK%s' % ("-Metal" if args.metalangle else "")
  iosversion = '9.0'
  frameworks = (["Metal", "MetalKit"] if args.metalangle else ["OpenGLES", "GLKit"]) + ["UIKit", "CoreGraphics", "CoreText", "CFNetwork", "Foundation"]
  xcframeworks = []
  if args.buildxcframework:
    xcframeworks += [frameworkName]

  with open('%s/scripts/ios-cocoapod/CartoMobileSDK.podspec.template' % baseDir, 'r') as f:
    cocoapodFile = string.Template(f.read()).safe_substitute({
      'baseDir': baseDir,
      'distDir': distDir,
      'distName': distName,
      'frameworkName': frameworkName,
      'version': version,
      'license': readLicense(),
      'iosversion': iosversion,
      'frameworks': ', '.join('"%s"' % framework for framework in frameworks),
      'vendoredFrameworks': ', '.join('"%s.xcframework"' % framework for framework in xcframeworks) if xcframeworks else 'nil'
    })
  with open('%s/%s.podspec' % (distDir, frameworkName), 'w') as f:
    f.write(cocoapodFile)

  if buildpackage:
    try:
      os.remove('%s/%s' % (distDir, distName))
    except:
      pass
    frameworkDir = 'CartoMobileSDK.xcframework' if args.buildxcframework else 'CartoMobileSDK.framework'
    if not execute('zip', distDir, '-y', '-r', distName, frameworkDir):
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
parser.add_argument('--build-xcframework', dest='buildxcframework', default=False, action='store_true', help='Build XCFramework')
parser.add_argument('--build-cocoapod', dest='buildcocoapod', default=False, action='store_true', help='Build CocoaPod')
parser.add_argument('--build-cocoapod-package', dest='buildcocoapodpackage', default=False, action='store_true', help='Build CocoaPod')
parser.add_argument('--use-metalangle', dest='metalangle', default=False, action='store_true', help='Use MetalANGLE instead of Apple GL')
parser.add_argument('--strip-bitcode', dest='stripbitcode', default=False, action='store_true', help='Strip bitcode from the built framework')
parser.add_argument('--shared-framework', dest='sharedlib', default=False, action='store_true', help='Build shared framework instead of static')

args = parser.parse_args()
if 'all' in args.iosarch or args.iosarch == []:
  args.iosarch = IOS_ARCHS
args.defines += ';' + getProfile(args.profile).get('defines', '')
if args.metalangle:
  args.defines += ';' + '_CARTO_USE_METALANGLE'
args.cmakeoptions += ';' + getProfile(args.profile).get('cmake-options', '')

args.devteam = os.environ.get('IOS_DEV_TEAM', None)
if args.sharedlib:
  if args.devteam is None:
    print("Shared library requires development team, IOS_DEV_TEAM variable not set")
    sys.exit(-1)

if not os.path.exists("%s/generated/ios-objc/proxies" % getBaseDir()):
  print("Proxies/wrappers not generated yet, run swigpp script first.")
  sys.exit(-1)

if not checkExecutable(args.cmake, '--help'):
  print('Failed to find CMake executable. Use --cmake to specify its location')
  sys.exit(-1)

for arch in args.iosarch:
  if not buildIOSLib(args, arch):
    sys.exit(-1)

if args.buildxcframework:
  if not buildIOSXCFramework(args, args.iosarch):
    sys.exit(-1)
else:
  if not buildIOSFramework(args, args.iosarch):
    sys.exit(-1)

if args.buildcocoapod or args.buildcocoapodpackage:
  if not buildIOSCocoapod(args, args.buildcocoapodpackage):
    sys.exit(-1)
