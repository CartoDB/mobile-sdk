import os
import sys
import shutil
import argparse
import string
from build.sdk_build_utils import *

def build(args):
  baseDir = getBaseDir()
  buildDir = getBuildDir('emscripten')
  distDir = getDistDir('emscripten')
  defines = ["-D%s" % define for define in args.defines.split(';') if define]
  defines.sort()
  options = ["-D%s" % option for option in args.cmakeoptions.split(';') if option]
  options.sort()

  if not cmake(args, buildDir, ["cmake"] + options + [
    "-DCMAKE_BUILD_TYPE=%s" % args.configuration,
    #"-DWRAPPER_DIR=%s" % ('%s/generated/android-java/wrappers' % baseDir),
    "-DSINGLE_LIBRARY:BOOL=ON",
    "-DSDK_CPP_DEFINES=%s" % " ".join(defines),
    "-DSDK_PLATFORM='Web'",
    '%s/scripts/build' % baseDir
  ]):
    return False
  if not execute(args.make, buildDir, "make", "-j4"):
    return False

  return makedirs('%s' % (distDir)) and execute(args.cc, buildDir, 
    "-lembind", "-Wl,--whole-archive",
    "%s/libcarto_mobile_sdk.a" % buildDir, "-o", "%s/CartoMobileSDK.js" % distDir,
    "-O3", "-s", "ERROR_ON_UNDEFINED_SYMBOLS=0", "-s", "FORCE_FILESYSTEM=1", "-s", "FULL_ES2=1",
    "-s", "FETCH=1", "-lworkerfs.js", "-s", "TOTAL_MEMORY=512MB", "-s", "DISABLE_EXCEPTION_CATCHING=0",
    "-s", "WASM=1", "-s", "MODULARIZE=1", "-s", "EXPORT_NAME=\"CModule\"",
    "-s", "RESERVED_FUNCTION_POINTERS=20", "-s", "USE_PTHREADS", "-s", "PTHREAD_POOL_SIZE=2",
    "-s", "OFFSCREENCANVAS_SUPPORT",
    # "-s", "OFFSCREEN_FRAMEBUFFER",
    "-s", "EXPORTED_RUNTIME_METHODS=['FS']"
  )

parser = argparse.ArgumentParser()
parser.add_argument('--profile', dest='profile', default=getDefaultProfileId(), type=validProfile, help='Build profile')
parser.add_argument('--defines', dest='defines', default='', help='Defines for compilation')
parser.add_argument('--make', dest='make', default='emmake', help='emmake executable')
parser.add_argument('--cmake', dest='cmake', default='emcmake', help='emcmake executable')
parser.add_argument('--cc', dest='cc', default='emcc', help='emcc executable')
parser.add_argument('--cmake-options', dest='cmakeoptions', default='', help='CMake options')
parser.add_argument('--configuration', dest='configuration', default='Release', choices=['Release', 'RelWithDebInfo', 'Debug'], help='Configuration')

args = parser.parse_args()
args.defines += ';' + getProfile(args.profile).get('defines', '')
args.cmakeoptions += ';' + getProfile(args.profile).get('cmake-options', '')

#if not os.path.exists("%s/generated/emscripten/proxies" % getBaseDir()):
#  print("Proxies/wrappers not generated yet, run swigpp script first.")
#  sys.exit(-1)

if not checkExecutable(args.cmake, 'cmake', '--help'):
  print(args.cmake)
  print('Failed to find emcmake executable. Use --cmake to specify its location')
  sys.exit(-1)

if not checkExecutable(args.make, 'make', '--help'):
  print('Failed to find emmake executable. Use --make to specify its location')
  sys.exit(-1)

if not checkExecutable(args.cc, '--help'):
  print('Failed to find emcc executable. Use --cc to specify its location')
  sys.exit(-1)

if not build(args):
    sys.exit(-1)