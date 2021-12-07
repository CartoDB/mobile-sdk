import os
import sys
import string
import argparse
from build.sdk_build_utils import *

WINPHONE10_ARCHS = ['x86', 'x64', 'ARM64']

DEFAULT_MSBUILD = "C:/Program Files (x86)/MSBuild/14.0/Bin/msbuild.exe"

def msbuild(args, dir, *cmdArgs):
  return execute(args.msbuild, dir, *cmdArgs)

def nuget(args, dir, *cmdArgs):
  return execute(args.nuget, dir, *cmdArgs)

def corflags(args, dir, *cmdArgs):
  return execute(args.corflags, dir, *cmdArgs)

def patchVcxprojFile(baseDir, fileName, patched1=False, patched2=False):
  with open(fileName, 'rb') as f:
    linesIn = f.readlines()
  linesOut = []
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
    '-G', 'Visual Studio 17 2022',
    '-DCMAKE_SYSTEM_NAME=WindowsStore',
    '-DCMAKE_SYSTEM_VERSION=10.0',
    '-DCMAKE_GENERATOR_PLATFORM=%s' % platformArch,
    '-DCMAKE_BUILD_TYPE=%s' % args.nativeconfiguration,
    '-DWRAPPER_DIR=%s' % ('%s/generated/winphone-csharp/wrappers' % baseDir),
    '-DSINGLE_LIBRARY:BOOL=ON',
    "-DSDK_CPP_DEFINES=%s" % " ".join(defines),
    "-DSDK_VERSION='%s'" % version,
    "-DSDK_PLATFORM='Windows Phone 10'",
    "-DSDK_WINPHONE_ARCH='%s'" % arch,
    '%s/scripts/build' % baseDir
  ]):
    return False
  patchVcxprojFile(baseDir, '%s/carto_mobile_sdk.vcxproj' % buildDir)
  return cmake(args, buildDir, [
    '--build', '.',
    '--parallel', '4',
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

  if not nuget(args, buildDir, 'restore', '%s/CartoMobileSDK.WinPhone.csproj' % buildDir):
    print("Failed to restore required nuget packages")
    return False

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

  with open('%s/scripts/winphone10-vsix/CartoMobileSDK.WinPhone.VSIX.csproj.template' % baseDir, 'r') as f:
    csProjFile = string.Template(f.read()).safe_substitute({ 'baseDir': baseDir, 'buildDir': buildDir, 'configuration': args.configuration, 'nativeConfiguration': args.nativeconfiguration })
  with open('%s/CartoMobileSDK.WinPhone.VSIX.csproj' % buildDir, 'w') as f:
    f.write(csProjFile)

  if not msbuild(args, buildDir,
    '/t:Build',
    '/p:Configuration=%s' % args.configuration,
    '%s/CartoMobileSDK.WinPhone.VSIX.csproj' % buildDir
  ):
    return False
  if not copyfile('%s/bin/%s/CartoMobileSDK.WinPhone.VSIX.vsix' % (buildDir, args.configuration), '%s/CartoMobileSDK.WinPhone10.VSIX.vsix' % distDir):
    return False

  print("VSIX output available in:\n%s" % distDir)
  return True

def buildWinPhoneNuget(args):
  baseDir = getBaseDir()
  buildDir = getBuildDir('winphone_nuget')
  distDir = getDistDir('winphone10')
  version = args.buildversion

  with open('%s/scripts/nuget/CartoMobileSDK.WinPhone.nuspec.template' % baseDir, 'r') as f:
    nuspecFile = string.Template(f.read()).safe_substitute({
      'baseDir': baseDir,
      'buildDir': buildDir,
      'configuration': args.configuration,
      'nativeConfiguration': args.nativeconfiguration,
      'version': version
    })
  with open('%s/CartoMobileSDK.WinPhone.nuspec' % buildDir, 'w') as f:
    f.write(nuspecFile)

  # A hack to generate non-arch dependent assembly, this is nuget peculiarity
  arch = 'x86' if 'x86' in args.winphonearch else args.winphonearch[0]
  if not copyfile('%s/../winphone_managed10-%s/bin/%s/CartoMobileSDK.WinPhone.dll' % (buildDir, arch, args.configuration), '%s/CartoMobileSDK.WinPhone.dll' % buildDir):
    return False
  if not corflags(args, buildDir,
    '/32BITREQ-',
    '%s/CartoMobileSDK.WinPhone.dll' % buildDir
  ):
    return False

  if not nuget(args, buildDir,
    'pack',
    '%s/CartoMobileSDK.WinPhone.nuspec' % buildDir,
    '-BasePath', '/'
  ):
    return False

  if not copyfile('%s/CartoMobileSDK.UWP.%s.nupkg' % (buildDir, version), '%s/CartoMobileSDK.UWP.%s.nupkg' % (distDir, version)):
    return False

  print("Nuget output available in:\n%s" % distDir)
  return True

parser = argparse.ArgumentParser()
parser.add_argument('--profile', dest='profile', default=getDefaultProfileId(), type=validProfile, help='Build profile')
parser.add_argument('--winphone-arch', dest='winphonearch', default=[], choices=WINPHONE10_ARCHS + ['all'], action='append', help='Windows phone target architectures')
parser.add_argument('--defines', dest='defines', default='', help='Defines for compilation')
parser.add_argument('--msbuild', dest='msbuild', default='auto', help='WinPhone msbuild executable')
parser.add_argument('--nuget', dest='nuget', default='nuget', help='nuget executable')
parser.add_argument('--corflags', dest='corflags', default='corflags', help='corflags executable')
parser.add_argument('--cmake', dest='cmake', default='cmake', help='CMake executable')
parser.add_argument('--cmake-options', dest='cmakeoptions', default='', help='CMake options')
parser.add_argument('--configuration', dest='configuration', default='Release', choices=['Release', 'RelWithDebInfo', 'Debug'], help='Configuration')
parser.add_argument('--build-number', dest='buildnumber', default='', help='Build sequence number, goes to version str')
parser.add_argument('--build-version', dest='buildversion', default='%s-devel' % SDK_VERSION, help='Build version, goes to distributions')
parser.add_argument('--build-vsix', dest='buildvsix', default=False, action='store_true', help='Build VSIX package')
parser.add_argument('--build-nuget', dest='buildnuget', default=False, action='store_true', help='Build Nuget package')

args = parser.parse_args()
if args.msbuild == 'auto':
  args.msbuild = DEFAULT_MSBUILD
if 'all' in args.winphonearch or args.winphonearch == []:
  args.winphonearch = WINPHONE10_ARCHS
args.defines += ';' + getProfile(args.profile).get('defines', '')
args.cmakeoptions += ';' + getProfile(args.profile).get('cmake-options', '')
args.nativeconfiguration = 'RelWithDebInfo' if args.configuration == 'Debug' else args.configuration

if not os.path.exists("%s/generated/winphone-csharp/proxies" % getBaseDir()):
  print("Proxies/wrappers not generated yet, run swigpp script first.")
  sys.exit(-1)

if not checkExecutable(args.cmake, '--help'):
  print('Failed to find CMake executable. Use --cmake to specify its location')
  sys.exit(-1)

if not checkExecutable(args.msbuild, '/?'):
  print('Failed to find msbuild executable. Use --msbuild to specify its location')
  sys.exit(-1)

if not checkExecutable(args.nuget, 'help'):
  print('Failed to find nuget executable. Use --nuget to specify its location')
  sys.exit(-1)

if args.buildnuget:
  if not checkExecutable(args.corflags, '/?'):
    print('Failed to find corflags executable. Use --corflags to specify its location')
    sys.exit(-1)
  if 'x86' not in args.winphonearch:
    print('Nuget package requires x86 architecture')
    sys.exit(-1)

for arch in args.winphonearch:
  if not buildWinPhoneNativeDLL(args, arch):
    sys.exit(-1)
  if not buildWinPhoneManagedDLL(args, arch):
    sys.exit(-1)
if args.buildvsix:
  if not buildWinPhoneVSIX(args):
    sys.exit(-1)
if args.buildnuget:
  if not buildWinPhoneNuget(args):
    sys.exit(-1)
