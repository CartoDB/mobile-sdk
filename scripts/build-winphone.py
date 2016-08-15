import os
import string
import argparse
from build.sdk_build_utils import *

WINPHONE10_ARCHS = ['x86', 'x64', 'ARM']

DEFAULT_MSBUILD = "C:/Program Files (x86)/MSBuild/14.0/Bin/msbuild.exe"

def msbuild(args, dir, *cmdArgs):
  return execute(args.msbuild, dir, *cmdArgs)

def patchVcxprojFile(baseDir, fileName):
  with open(fileName, 'rb') as f:
    linesIn = f.readlines()
  linesOut = []
  patched1 = False
  patched2 = False
  for line in linesIn:
    if line.strip() == '<Import Project="$(VCTargetsPath)\\Microsoft.Cpp.targets" />' and not patched2:
      with open('%s/scripts/winphone10/carto_mobile_sdk.vcxproj.patch2' % baseDir, 'rb') as f:
        linesOut += f.readlines()
      patched2 = True
    linesOut.append(line)
    if line.strip() == '<Import Project="$(VCTargetsPath)\\Microsoft.Cpp.props" />' and not patched1:
      with open('%s/scripts/winphone10/carto_mobile_sdk.vcxproj.patch1' % baseDir, 'rb') as f:
        linesOut += f.readlines()
      patched1 = True
  with open(fileName, 'wb') as f:
    f.writelines(linesOut)

def buildWinPhoneNativeDLL(args, arch):
  version = getVersion(args.buildnumber) if args.configuration == 'Release' else 'Devel'
  platformArch = 'Win32' if arch == 'x86' else arch
  baseDir = getBaseDir()
  buildDir = getBuildDir('winphone_native10', platformArch)
  defines = ["-D%s" % define for define in args.defines.split(';') if define]
  options = ["-D%s" % option for option in args.cmakeoptions.split(';') if option]

  if not cmake(args, buildDir, options + [
    '-G', 'Visual Studio 14 2015',
    '-DCMAKE_SYSTEM_NAME=WindowsStore',
    '-DCMAKE_SYSTEM_VERSION=10.0',
    '-DCMAKE_GENERATOR_PLATFORM=%s' % platformArch,
    '-DCMAKE_BUILD_TYPE=%s' % args.nativeconfiguration,
    '-DWRAPPER_DIR=%s' % ('%s/generated/winphone-csharp/wrappers' % baseDir),
    '-DANGLE_LIB_DIR=%s' % ('%s/libs-external/angle/prebuilt/Lib/%s' % (baseDir, platformArch)),
    "-DSDK_CPP_DEFINES=%s" % " ".join(defines),
    "-DSDK_VERSION='%s'" % version,
    "-DSDK_PLATFORM='Windows Phone 10'",
    '%s/scripts/build' % baseDir
  ]):
    return False
  patchVcxprojFile(baseDir, '%s/carto_mobile_sdk.vcxproj' % buildDir)
  return cmake(args, buildDir, [
    '--build', '.',
    '--config', args.nativeconfiguration
  ])

def buildWinPhoneManagedDLL(args, arch):
  baseDir = getBaseDir()
  buildDir = getBuildDir('winphone_managed10', arch)

  proxyFiles = os.listdir("%s/generated/winphone-csharp/proxies" % baseDir)
  proxies = "\n".join(['<Compile Include="%s\\generated\\winphone-csharp\\proxies\\%s"><Link>Proxies\%s</Link></Compile>' % (baseDir, proxyFile, proxyFile) for proxyFile in proxyFiles])
  with open('%s/scripts/winphone10/CartoMobileSDK.WinPhone.csproj.template' % baseDir, 'r') as f:
    csProjFile = string.Template(f.read()).safe_substitute({ 'baseDir': baseDir, 'buildDir': buildDir, 'proxies': proxies })
  with open('%s/CartoMobileSDK.WinPhone.csproj' % buildDir, 'w') as f:
    f.write(csProjFile)

  copyfile('%s/scripts/winphone10/project.json' % baseDir, '%s/project.json' % buildDir)
  copyfile('%s/scripts/winphone10/project.lock.json' % baseDir, '%s/project.lock.json' % buildDir)

  return msbuild(args, buildDir,
    '/t:Build',
    '/p:Configuration=%s' % args.configuration,
    '/p:ProcessorArchitecture=%s' % arch,
    '%s/CartoMobileSDK.WinPhone.csproj' % buildDir
  )

def buildWinPhoneVSIX(args):
  baseDir = getBaseDir()
  buildDir = getBuildDir('winphone_vsix10')
  distDir = getDistDir('winphone10')

  with open('%s/scripts/winphone10/CartoMobileSDK.WinPhone.VSIX.csproj.template' % baseDir, 'r') as f:
    csProjFile = string.Template(f.read()).safe_substitute({ 'baseDir': baseDir, 'buildDir': buildDir, 'configuration': args.configuration, 'nativeConfiguration': args.nativeconfiguration })
  with open('%s/CartoMobileSDK.WinPhone.VSIX.csproj' % buildDir, 'w') as f:
    f.write(csProjFile)

  if not msbuild(args, buildDir,
    '/t:Build',
    '/p:Configuration=%s' % args.configuration,
    '%s/CartoMobileSDK.WinPhone.VSIX.csproj' % buildDir
  ):
    return False
  if copyfile('%s/bin/%s/CartoMobileSDK.WinPhone.VSIX.vsix' % (buildDir, args.configuration), '%s/CartoMobileSDK.WinPhone10.VSIX.vsix' % distDir):
    print "Output available in:\n%s" % distDir
    return True
  return False

parser = argparse.ArgumentParser()
parser.add_argument('--profile', dest='profile', default=getDefaultProfile(), choices=getProfiles().keys(), help='Build profile')
parser.add_argument('--winphone-arch', dest='winphonearch', default=['all'], choices=WINPHONE10_ARCHS + ['all'], nargs='+', help='Windows phone target architectures')
parser.add_argument('--defines', dest='defines', default='', help='Defines for compilation')
parser.add_argument('--msbuild', dest='msbuild', default='auto', help='WinPhone msbuild executable')
parser.add_argument('--cmake', dest='cmake', default='cmake', help='CMake executable')
parser.add_argument('--cmake-options', dest='cmakeoptions', default='', help='CMake options')
parser.add_argument('--configuration', dest='configuration', default='Release', choices=['Release', 'Debug'], help='Configuration')
parser.add_argument('--build-number', dest='buildnumber', default='', help='Build sequence number, goes to version str')

args = parser.parse_args()
if args.msbuild == 'auto':
  args.msbuild = DEFAULT_MSBUILD
if 'all' in args.winphonearch:
  args.winphonearch = WINPHONE10_ARCHS
args.defines += ';' + getProfiles()[args.profile].get('defines', '')
args.cmakeoptions += ';' + getProfiles()[args.profile].get('cmake-options', '')
args.nativeconfiguration = 'RelWithDebInfo' if args.configuration == 'Debug' else args.configuration

for arch in args.winphonearch:
  if not buildWinPhoneNativeDLL(args, arch):
    exit(-1)
  if not buildWinPhoneManagedDLL(args, arch):
    exit(-1)
if not buildWinPhoneVSIX(args):
  exit(-1)
