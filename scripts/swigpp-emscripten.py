import os
import re
import sys
import argparse
import subprocess
import shutil
from build.sdk_build_utils import *

ENUM_TEMPLATE = """
"""

VALUE_TYPE_TEMPLATE = """
"""

SHARED_PTR_TEMPLATE = """
"""

SHARED_PTR_CODE_TEMPLATE = """
%feature("shared_ptr");
"""

POLYMORPHIC_SHARED_PTR_TEMPLATE = SHARED_PTR_TEMPLATE + """
"""

POLYMORPHIC_SHARED_PTR_CODE_TEMPLATE = SHARED_PTR_CODE_TEMPLATE + """
%feature("polymorphic_shared_ptr");
"""

VALUE_TEMPLATE_TEMPLATE = """
  %template($TYPE$) $CLASSNAME$;
"""

STANDARD_EQUALS_TEMPLATE = """
"""

STANDARD_EQUALS_CODE_TEMPLATE = """
"""

CUSTOM_EQUALS_TEMPLATE = """
"""

CUSTOM_EQUALS_CODE_TEMPLATE = """
"""

def fixProxyCode(fileName, className):
  if not os.path.isfile(fileName):
    return

  lines_in = readLines(fileName)

  lines_out = []
  for line in lines_in:
    # Add '@hidden' comment above the special SWIG-wrapper lines
    hide = line.strip() in []
    if line.find('swigCreatePolymorphicInstance') != -1:
      hide = True
    if hide:
      numSpaces = len(line) - len(line.lstrip())
      lines_out.append(line[:numSpaces] + '/** @hidden */\n')

    lines_out.append(line)

  with open(fileName, 'w') as f:
    f.writelines(lines_out)

def transformSwigFile(sourcePath, outPath, headerDirs):
  lines_in = [line.rstrip('\n') for line in readUncommentedLines(sourcePath)]
  lines_out = []
  class_imports = {}
  class_code = {}
  imports_linenum = None
  include_linenum = None
  stl_wrapper = False
  directors_module = False
  for line in lines_in:
    # Rename module
    match = re.search('^\s*(%module(?:[(].*[)]|)\s+)([^\s]*)\s*$', line)
    if match:
      if match.group(1):
        directors_module = 'directors' in match.group(1)
      line = '%s%sModule' % (match.group(1), match.group(2))

    # Language-specific method modifiers
    match = re.search('^\s*%(java|cs|objc)methodmodifiers.*$', line)
    if match:
      lang = match.group(1)
      if lang != 'java':
        continue

    # Language-specific rename declarations
    match = re.search('^\s*!(java|cs|objc)_rename(.*)$', line)
    if match:
      lang = match.group(1)
      if lang != 'java':
        continue
      line = '%%rename%s' % match.group(2)

    # Attributes
    match = re.search('^\s*(%|!)(static|)attribute.*$', line)
    if match:
      continue

    # Detect enum directive
    match = re.search('^\s*!enum\s*[(]([^)]*)[)].*$', line)
    if match:
      enumName = match.group(1).strip()
      args = { 'ENUMNAME': match.group(1).strip() }
      lines_out += applyTemplate(ENUM_TEMPLATE, args)
      continue

    # Detect value_type directive
    match = re.search('^\s*!value_type\s*[(]([^)]*),([^)]*)[)].*$', line)
    if match:
      className = match.group(1).strip()
      javaClass = match.group(2).strip().split(".")[-1]
      javaDescriptor = "Lcom/carto/%s;" % match.group(2).strip().replace('.', '/')
      args = { 'CLASSNAME': match.group(1).strip(), 'TYPE': javaClass, 'DESCRIPTOR': javaDescriptor }
      lines_out += applyTemplate(VALUE_TYPE_TEMPLATE, args)
      continue

    # Detect shared_ptr directive
    match = re.search('^\s*!shared_ptr\s*[(]([^)]*),([^)]*)[)].*$', line)
    if match:
      className = match.group(1).strip()
      javaClass = match.group(2).strip().split(".")[-1]
      javaDescriptor = "Lcom/carto/%s;" % match.group(2).strip().replace('.', '/')
      args = { 'CLASSNAME': match.group(1).strip(), 'TYPE': javaClass, 'DESCRIPTOR': javaDescriptor }
      code = class_code.get(className, [])
      code += applyTemplate(SHARED_PTR_CODE_TEMPLATE, args)
      class_code[className] = code
      lines_out += applyTemplate(SHARED_PTR_TEMPLATE, args)
      continue

    # Polymorphic shared_ptr
    match = re.search('^\s*!polymorphic_shared_ptr\s*[(]([^,]*),([^)]*)[)].*', line)
    if match:
      className = match.group(1).strip()
      javaPackage = 'com.carto.%s' % '.'.join(match.group(2).strip().split(".")[:-1])
      javaClass = match.group(2).strip().split(".")[-1]
      javaDescriptor = "Lcom/carto/%s;" % match.group(2).strip().replace('.', '/')
      args = { 'CLASSNAME': match.group(1).strip(), 'TYPE': javaClass, 'PACKAGE': javaPackage, 'DESCRIPTOR': javaDescriptor }
      code = class_code.get(className, [])
      code += applyTemplate(POLYMORPHIC_SHARED_PTR_CODE_TEMPLATE, args)
      class_code[className] = code
      lines_out += applyTemplate(POLYMORPHIC_SHARED_PTR_TEMPLATE, args)
      continue

    # Value-template
    match = re.search('^\s*!value_template\s*[(]([^)]*),([^)]*)[)].*$', line)
    if match:
      className = match.group(1).strip()
      javaClass = match.group(2).strip().split(".")[-1]
      args = { 'CLASSNAME': match.group(1).strip(), 'TYPE': javaClass }
      lines_out += applyTemplate(VALUE_TEMPLATE_TEMPLATE, args)
      continue

    # Standard equals
    match = re.search('^\s*!standard_equals\s*[(]([^)]*)[)].*', line)
    if match:
      className = match.group(1).strip()
      args = { 'CLASSNAME': match.group(1).strip() }
      code = class_code.get(className, [])
      code += applyTemplate(STANDARD_EQUALS_CODE_TEMPLATE, args)
      class_code[className] = code
      lines_out += applyTemplate(STANDARD_EQUALS_TEMPLATE, args)
      continue

    # Custom equals
    match = re.search('^\s*!custom_equals\s*[(]([^)]*)[)].*', line)
    if match:
      className = match.group(1).strip()
      args = { 'CLASSNAME': match.group(1).strip() }
      code = class_code.get(className, [])
      code += applyTemplate(CUSTOM_EQUALS_CODE_TEMPLATE, args)
      class_code[className] = code
      lines_out += applyTemplate(CUSTOM_EQUALS_TEMPLATE, args)
      continue

    # Custom toString
    match = re.search('^\s*!custom_tostring\s*[(]([^)]*)[)].*', line)
    if match:
      continue

    # Imports
    match = re.search('^\s*!(proxy|java|cs|objc)_imports\s*[(]([^)]*)[)](.*)$', line)
    if match:
      lang = match.group(1)
      parts = [part.strip() for part in match.group(2).split(",")]
      className = parts[0]
      if lang == 'proxy':
        class_imports[className] = class_imports.get(className, []) + ['import com.carto.%s;' % part for part in parts[1:]]
      elif lang == 'java':
        class_imports[className] = class_imports.get(className, []) + ['import %s;' % part for part in parts[1:]]
      else:
        continue
      line = match.group(3)
      if imports_linenum is None:
        imports_linenum = len(lines_out)

    # Check for STL templates
    match = re.search('^\s*%template\(.*\)\s*std::.*$', line)
    if match:
      stl_wrapper = True

    # javacode import
    match = re.search('^\s*%typemap[(]javacode[)]\s*(.*)\s*%{(.*)%}', line)
    if match:
      className = match.group(1).strip()
      code = class_code.get(className, [])
      code.append(match.group(2))
      class_code[className] = code
      continue

    # Includes
    match = re.search('^\s*%include\s+(.*)$', line)
    if match:
      include_linenum = len(lines_out)

    # Rename all methods to nonCapCase, add this before including C++ code
    match = re.search('^\s*%include\s+"(.*)".*$', line)
    if match:
      includeName = match.group(1)
      if not stl_wrapper and includeName != "NutiSwig.i":
        # This is a huge hack: we will capitalize all method names starting with lower case letters. But we will do this only for carto:: classes
        for n in range(ord('a'), ord('z') + 1):
          c = chr(n)
          lines_out.append('%%rename("%%(regex:/::%s([^:]*)$/%s\\\\1/)s", fullname=1, regextarget=1, %%$isfunction) "^carto::.+::%s[^:]*$";' % (c.upper(), c, c.upper()))
        lines_out.append('')

    lines_out.append(line)

  # Add imports
  if imports_linenum is not None:
    lines_import = []
    for className, imports in class_imports.items():
      lines_import.append("%%typemap(javaimports) %s %%{" % className)
      lines_import += imports
      lines_import.append("%}")
    lines_import.append("")
    lines_import.append("%pragma(java) jniclassimports=%{")
    for className, imports in class_imports.items():
      lines_import += imports
    lines_import.append("%}")
    lines_out = lines_out[:imports_linenum] + lines_import + lines_out[imports_linenum:]
    if include_linenum > imports_linenum:
      include_linenum += len(lines_import)

  # Add typemap code
  if include_linenum is not None:
    lines_code = []
    for className, code in class_code.items():
      lines_code += code
    lines_out = lines_out[:include_linenum] + lines_code + lines_out[include_linenum:]

  # Write processed module
  with open(outPath, 'w') as f:
    f.writelines([line + '\n' for line in lines_out])

def transformSwigPackage(args, sourceDir, outDir, packageName):
  for fileName in os.listdir(sourceDir):
    if fileName == 'NutiSwig.i':
      continue
    sourcePath = os.path.join(sourceDir, fileName)
    if not os.path.isfile(sourcePath):
      continue
    if not os.path.isdir(outDir):
      os.makedirs(outDir)
    outPath = os.path.join(outDir, fileName)
    headerDirs = args.cppDir.split(";")
    transformSwigFile(sourcePath, outPath, headerDirs)
  return True

def transformSwigPackages(args, sourceDir, outDir, basePackageName):
  for dirName in os.listdir(sourceDir):
    sourcePath = os.path.join(sourceDir, dirName)
    if not os.path.isdir(sourcePath) or dirName.startswith("."):
      continue
    outPath = os.path.join(outDir, dirName)
    packageName = (basePackageName + '.' if basePackageName else '') + dirName
    transformSwigPackages(args, sourcePath, outPath, packageName)
    if not transformSwigPackage(args, sourcePath, outPath, packageName):
      return False
  return True

def buildSwigPackage(args, sourceDir, packageName):
  for fileName in os.listdir(sourceDir):
    if fileName == 'NutiSwig.i' or fileName == 'BaseMapView.i':
      continue
    fileNameWithoutExt = fileName.split(".")[0]
    sourcePath = os.path.join(sourceDir, fileName)
    outPath = os.path.join(args.wrapperDir, fileNameWithoutExt) + "_wrap.cpp"
    if not os.path.isfile(sourcePath):
      continue
    if not os.path.isdir(args.wrapperDir):
      os.makedirs(args.wrapperDir)

    includes = ["-I%s" % dir for dir in ["../scripts/swig/java", "../scripts/swig", args.moduleDir] + args.sourceDir.split(";") + [args.wrapperDir] + args.cppDir.split(";")]
    swigPath = os.path.dirname(args.swig)
    if swigPath:
      includes += ["-I%s/Lib/emscripten" % swigPath, "-I%s/Lib" % swigPath]
    defines = ["-D%s" % define for define in args.defines.split(';') if define]
    cmd = [args.swig, "-c++", "-emscripten", "-o", outPath] + defines + includes + [sourcePath]
    if subprocess.call(cmd) != 0:
      print("Error in %s" % fileName)
      return False

    for line in [line.rstrip('\n') for line in readUncommentedLines(sourcePath)]:
      match = re.search('^\s*%template\((.*)\).*$', line)
      if match:
        templateFileNameWithoutExt = match.group(1)
  return True

def buildSwigPackages(args, sourceDir, basePackageName):
  for dirName in os.listdir(sourceDir):
    sourcePath = os.path.join(sourceDir, dirName)
    if not os.path.isdir(sourcePath) or dirName.startswith("."):
      continue
    packageName = (basePackageName + '.' if basePackageName else '') + dirName
    buildSwigPackages(args, sourcePath, packageName)
    if not buildSwigPackage(args, sourcePath, packageName):
      return False
  return True

parser = argparse.ArgumentParser()
parser.add_argument('--profile', dest='profile', default=getDefaultProfileId(), type=validProfile, help='Build profile')
parser.add_argument('--swig', dest='swig', default='swig', help='path to Swig executable')
parser.add_argument('--defines', dest='defines', default='', help='Defines for Swig')
parser.add_argument('--cppdir', dest='cppDir', default='../all/native;../emscripten/native', help='directories containing C++ headers')
parser.add_argument('--wrapperdir', dest='wrapperDir', default='../generated/emscripten-js/wrappers', help='output directory for C++ wrappers')
parser.add_argument('--moduledir', dest='moduleDir', default='../generated/emscripten-js/modules', help='output directory containing preprocessed Swig modules')
parser.add_argument('--sourcedir', dest='sourceDir', default='../all/modules;../emscripten/modules', help='input directories containing subdirectories of Swig wrappers')

args = parser.parse_args()
args.defines += ';' + getProfile(args.profile).get('defines', '')

if not checkExecutable(args.swig, '-help'):
  print('Unable to find SWIG executable. Use --swig argument to specify its location. The supported version is available from https://github.com/cartodb/mobile-swig')
  sys.exit(-1)

if os.path.isdir(args.wrapperDir):
  shutil.rmtree(args.wrapperDir)
if os.path.isdir(args.moduleDir):
  shutil.rmtree(args.moduleDir)
for sourceDir in args.sourceDir.split(";"):
  if os.path.exists(sourceDir) and not transformSwigPackages(args, sourceDir, args.moduleDir, ""):
    sys.exit(-1)
if not buildSwigPackages(args, args.moduleDir, ""):
  sys.exit(-1)
