import os
import sys
import re
import shutil
import itertools
import argparse
import string
from build.sdk_build_utils import *

IOS_ARCHS = ['i386', 'x86_64', 'armv7', 'arm64', 'arm64-simulator', 'x86_64-maccatalyst', 'arm64-maccatalyst']

def getFinalBuildDir(target, arch=None):
  return getBuildDir(('%s_metal' % target) if args.metalangle else target, arch)

def getFinalDistDir(args):
  return getDistDir('ios_metal' if args.metalangle else 'ios')

def getPlatformArch(baseArch):
  if baseArch.endswith('-maccatalyst'):
    return 'MACCATALYST', baseArch[:-12]
  if baseArch.endswith('-simulator'):
    return 'SIMULATOR', baseArch[:-10]
  return ('OS' if baseArch.startswith('arm') else 'SIMULATOR'), baseArch

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
    f.write('    link "c++"\n')
    f.write('    link "z"\n')
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

def buildIOSLib(args, baseArch, outputDir=None):
  platform, arch = getPlatformArch(baseArch)
  version = getVersion(args.buildnumber) if args.configuration == 'Release' else 'Devel'
  baseDir = getBaseDir()
  buildDir = outputDir or getFinalBuildDir('ios', '%s-%s' % (platform, arch))
  defines = ["-D%s" % define for define in args.defines.split(';') if define]
  options = ["-D%s" % option for option in args.cmakeoptions.split(';') if option]

  if not cmake(args, buildDir, options + [
    '-G', 'Xcode',
    '-DCMAKE_SYSTEM_NAME=%s' % ('Darwin' if platform == 'MACCATALYST' else 'iOS'),
    '-DWRAPPER_DIR=%s' % ('%s/generated/ios-objc/proxies' % baseDir),
    '-DINCLUDE_OBJC:BOOL=ON',
    '-DSINGLE_LIBRARY:BOOL=ON',
    '-DSHARED_LIBRARY:BOOL=%s' % ('ON' if args.sharedlib else 'OFF'),
    '-DCMAKE_OSX_ARCHITECTURES=%s' % arch,
    '-DCMAKE_OSX_SYSROOT=%s' % ('macosx' if platform == 'MACCATALYST' else 'iphone%s' % platform.lower()),
    '-DCMAKE_OSX_DEPLOYMENT_TARGET=%s' % ('11.3' if platform == 'MACCATALYST' else ('10.0' if arch == 'i386' else '9.0')),
    '-DCMAKE_BUILD_TYPE=%s' % args.configuration,
    "-DSDK_CPP_DEFINES=%s" % " ".join(defines),
    "-DSDK_DEV_TEAM='%s'" % (args.devteam if args.devteam else ""),
    "-DSDK_VERSION='%s'" % version,
    "-DSDK_PLATFORM='iOS'",
    "-DSDK_IOS_ARCH='%s'" % arch,
    "-DSDK_IOS_BASEARCH='%s'" % baseArch,
    '%s/scripts/build' % baseDir
  ]):
    return False

  bitcodeOptions = ['ENABLE_BITCODE=NO']
  if not args.stripbitcode and baseArch in ('armv7', 'arm64'):
    bitcodeOptions = ['ENABLE_BITCODE=YES', 'BITCODE_GENERATION_MODE=bitcode']
  return execute('xcodebuild', buildDir,
    '-project', 'carto_mobile_sdk.xcodeproj', '-arch', arch, '-configuration', args.configuration, 'archive',
    *list(bitcodeOptions)
  )

def buildIOSFramework(args, baseArchs, outputDir=None):
  baseDir = getBaseDir()
  distDir = outputDir or getFinalDistDir(args)
  if args.sharedlib:
    frameworkDir = '%s/CartoMobileSDK.framework' % distDir
  else:
    frameworkDir = '%s/CartoMobileSDK.framework/Versions/A' % distDir
  shutil.rmtree(distDir, True)
  makedirs(frameworkDir)

  libFilePaths = []
  for baseArch in baseArchs:
    platform, arch = getPlatformArch(baseArch)
    if platform == 'MACCATALYST':
      libFilePath = "%s/%s/libcarto_mobile_sdk.%s" % (getFinalBuildDir('ios', '%s-%s' % (platform, arch)), args.configuration, 'dylib' if args.sharedlib else 'a')
    else:
      libFilePath = "%s/%s-%s/libcarto_mobile_sdk.%s" % (getFinalBuildDir('ios', '%s-%s' % (platform, arch)), args.configuration, ('iphone%s' % platform.lower()), 'dylib' if args.sharedlib else 'a')
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

def buildIOSXCFramework(args, baseArchs, outputDir=None):
  groupedPlatformArchs = {}
  for baseArch in baseArchs:
    platform, arch = getPlatformArch(baseArch)
    groupedPlatformArchs[platform] = groupedPlatformArchs.get(platform, []) + [baseArch]
  baseDir = getBaseDir()
  distDir = outputDir or getFinalDistDir(args)
  shutil.rmtree(distDir, True)
  makedirs(distDir)

  frameworkBuildDirs = []
  for platform, baseArchs in groupedPlatformArchs.items():
    frameworkBuildDir = getFinalBuildDir('ios-framework', '%s-%s' % (platform, '-'.join(baseArchs)))
    if not buildIOSFramework(args, baseArchs, frameworkBuildDir):
      return False
    frameworkBuildDirs.append(frameworkBuildDir)

  frameworkOptions = itertools.chain(*[['-framework', '%s/CartoMobileSDK.framework' % frameworkBuildDir] for frameworkBuildDir in frameworkBuildDirs])
  if not execute('xcodebuild', baseDir,
    '-create-xcframework', '-output', '%s/CartoMobileSDK.xcframework' % distDir,
    *list(frameworkOptions)
  ):
    return False

  if outputDir is None:
    print("Output available in:\n%s" % distDir)
  return True

def buildIOSPackage(args, buildCocoapod, buildSwiftPackage):
  baseDir = getBaseDir()
  distDir = getFinalDistDir(args)
  version = args.buildversion
  distName = 'sdk4-ios-%s%s.zip' % (("metal-" if args.metalangle else ""), version)
  frameworkName = 'CartoMobileSDK%s' % ("-Metal" if args.metalangle else "")
  frameworkDir = 'CartoMobileSDK.%s' % ("xcframework" if args.buildxcframework else "framework")
  frameworks = (["QuartzCore"] if args.metalangle else ["OpenGLES", "GLKit"]) + ["UIKit", "CoreGraphics", "CoreText", "CFNetwork", "Foundation"]
  weakFrameworks = (["Metal"] if args.metalangle else [])

  try:
    os.remove('%s/%s' % (distDir, distName))
  except:
    pass
  if not execute('zip', distDir, '-y', '-r', distName, frameworkDir):
    return False

  if buildCocoapod:
    with open('%s/scripts/ios-cocoapod/CartoMobileSDK.podspec.template' % baseDir, 'r') as f:
      cocoapodFile = string.Template(f.read()).safe_substitute({
        'baseDir': baseDir,
        'distDir': distDir,
        'distName': distName,
        'frameworkName': frameworkName,
        'frameworkDir': frameworkDir,
        'version': version,
        'license': readLicense(),
        'frameworks': ', '.join('"%s"' % framework for framework in frameworks) if frameworks else 'nil',
        'weakFrameworks': ', '.join('"%s"' % framework for framework in weakFrameworks) if weakFrameworks else 'nil'
      })
    with open('%s/%s.podspec' % (distDir, frameworkName), 'w') as f:
      f.write(cocoapodFile)

  if buildSwiftPackage:
    with open('%s/scripts/ios-swiftpackage/Package.swift.template' % baseDir, 'r') as f:
      packageFile = string.Template(f.read()).safe_substitute({
        'baseDir': baseDir,
        'distDir': distDir,
        'distName': distName,
        'frameworkName': frameworkName,
        'frameworkDir': frameworkDir,
        'version': version,
        'checksum': checksumSHA256('%s/%s' % (distDir, distName))
      })
    with open('%s/Package.swift' % distDir, 'w') as f:
      f.write(packageFile)

  print('Output available in:\n%s\n\nTo publish, use:\ncd %s\naws s3 cp %s s3://nutifront/sdk_snapshots/%s --grants read=uri=http://acs.amazonaws.com/groups/global/AllUsers\n' % (distDir, distDir, distName, distName))
  if buildCocoapod:
    print('pod trunk push\n')
  if buildSwiftPackage:
    spmPackageName = "mobile-sdk-ios-metal-swift-package" if args.metalangle else "mobile-sdk-ios-swift-package"
    print('rm -rf %s\ngit clone git@github.com:nutiteq/%s\ncp Package.swift %s\ncd %s\ngit add Package.swift\ngit commit -m "Version %s" && git tag %s\ngit push origin --tags\n' % (spmPackageName, spmPackageName, spmPackageName, spmPackageName, version, version))
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
parser.add_argument('--build-swiftpackage', dest='buildswiftpackage', default=False, action='store_true', help='Build Swift Package')
parser.add_argument('--use-metalangle', dest='metalangle', default=False, action='store_true', help='Use MetalANGLE instead of Apple GL')
parser.add_argument('--strip-bitcode', dest='stripbitcode', default=False, action='store_true', help='Strip bitcode from the built framework')
parser.add_argument('--shared-framework', dest='sharedlib', default=False, action='store_true', help='Build shared framework instead of static')

args = parser.parse_args()
if 'all' in args.iosarch or args.iosarch == []:
  args.iosarch = IOS_ARCHS
  if not args.buildxcframework:
    args.iosarch = filter(lambda arch: not (arch.endswith('-simulator') or arch.endswith('-maccatalyst')), args.iosarch)
  if not args.metalangle:
    args.iosarch = filter(lambda arch: not arch.endswith('-maccatalyst'), args.iosarch)
args.defines += ';' + getProfile(args.profile).get('defines', '')
if args.metalangle:
  args.defines += ';' + '_CARTO_USE_METALANGLE'
else:
  if filter(lambda arch: arch.endswith('-maccatalyst'), args.iosarch):
    print('Mac Catalyst builds are only supported with MetalANGLE')
    sys.exit(-1)
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

if args.buildcocoapod or args.buildswiftpackage:
  if not buildIOSPackage(args, args.buildcocoapod, args.buildswiftpackage):
    sys.exit(-1)
