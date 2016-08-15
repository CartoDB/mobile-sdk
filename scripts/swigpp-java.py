import os
import re
import sys
import argparse
import subprocess
import shutil
from build.sdk_build_utils import *

VALUE_TYPE_TEMPLATE = """
%typemap(out) $CLASSNAME$ "*($&1_ltype*)&$result = new $1_ltype($1);"
%typemap(directorin, descriptor="$DESCRIPTOR$") $CLASSNAME$ "*($&1_ltype*)&$input = new $1_ltype($1);"
%typemap(javadirectorin) $CLASSNAME$ "new $TYPE$($jniinput, true)"
%typemap(javaout) $CLASSNAME$ { return new $TYPE$($jnicall, true); }
%typemap(javadirectorout) $CLASSNAME$ "$TYPE$.getCPtr($javacall)"

%typemap(out) const $CLASSNAME$& "*($&1_ltype)&$result = new $*1_ltype(*$1);"
%typemap(directorin, descriptor="$DESCRIPTOR$") const $CLASSNAME$& "*($&1_ltype)&$input = new $*1_ltype($1);"
%typemap(javadirectorin) const $CLASSNAME$& "new $TYPE$($jniinput, true)"
%typemap(javaout) const $CLASSNAME$& { return new $TYPE$($jnicall, true); }
%typemap(javadirectorout) const $CLASSNAME$& "$TYPE$.getCPtr($javacall)"
"""

SHARED_PTR_TEMPLATE = """
%shared_ptr($CLASSNAME$)

%typemap(directorin, descriptor="$DESCRIPTOR$") std::shared_ptr< $CLASSNAME$ > "*($&1_ltype*)&$input = new $1_ltype(*$1);"
%typemap(directorin, descriptor="$DESCRIPTOR$") std::shared_ptr< $CLASSNAME$ >& "*($&1_ltype)&$input = new $*1_ltype($1);"
"""

POLYMORPHIC_SHARED_PTR_TEMPLATE = SHARED_PTR_TEMPLATE + """
%{
#include "components/ClassRegistry.h"
#include "components/Director.h"
static carto::ClassRegistry::Entry $TYPE$RegistryEntry(typeid(const $CLASSNAME$&), "$TYPE$");
%}

%extend $CLASSNAME$ {
  /**
    * Returns the actual class name of this object. This is used internally by the SDK.
    * @return The class name of this object.
    */
  std::string swigGetClassName() const {
    std::string className = carto::ClassRegistry::GetClassName(typeid(*$self));
    if (className.empty()) {
      className = "$TYPE$";
    }
    return className;
  }

  /**
    * Returns the connected director object. This is used internally by the SDK.
    * @return The connected director object or null if director is not connected.
    */
  jobject swigGetDirectorObject() const {
    if (auto director = dynamic_cast<const carto::Director*>($self)) {
      return static_cast<jobject>(director->getDirectorObject());
    }
    return 0;
  }
}

%typemap(javaout) std::shared_ptr<$CLASSNAME$>, const std::shared_ptr<$CLASSNAME$>&, std::shared_ptr<$CLASSNAME$>& {
  long cPtr = $jnicall;
  if (cPtr == 0) {
    return null;
  }
  return $PACKAGE$.$TYPE$.swigCreatePolymorphicInstance(cPtr, true);
}

%typemap(javadirectorin) std::shared_ptr<$CLASSNAME$>, const std::shared_ptr<$CLASSNAME$>&, std::shared_ptr<$CLASSNAME$>& %{
  $PACKAGE$.$TYPE$.swigCreatePolymorphicInstance($jniinput, true)
%}
"""

POLYMORPHIC_SHARED_PTR_CODE_TEMPLATE = """
  /**
   * Creates a polymorphic instance of the given native object. This is used internally by the SDK.
   * @param cPtr The native pointer of the instance.
   * @param cMemoryOwn The ownership flag.
   * @return The created instance.
   */
  public static $PACKAGE$.$TYPE$ swigCreatePolymorphicInstance(long cPtr, boolean cMemoryOwn) {
    if (cPtr == 0) {
      return null;
    }

    Object director = $PACKAGE$.$TYPE$ModuleJNI.$TYPE$_swigGetDirectorObject(cPtr, null);
    if (director != null) {
      return ($PACKAGE$.$TYPE$) director;
    }
	
    String objClassName = $PACKAGE$.$TYPE$ModuleJNI.$TYPE$_swigGetClassName(cPtr, null);
    $PACKAGE$.$TYPE$ objInstance = null;
    try {
      Class<?> objClass = Class.forName("$PACKAGE$." + objClassName);
      Class<?>[] argTypes = new Class<?>[] {long.class, boolean.class};
      Object[] args = new Object[] {cPtr, cMemoryOwn};
      java.lang.reflect.Constructor<?> constructor = objClass.getDeclaredConstructor(argTypes);
      objInstance = ($PACKAGE$.$TYPE$) constructor.newInstance(args);
    } catch (Exception e) {
      com.carto.utils.Log.error("Carto Mobile SDK: Could not instantiate class: " + objClassName + " error: " + e.getMessage());
    }
    return objInstance;
  }
"""

VALUE_TEMPLATE_TEMPLATE = """
  %template($TYPE$) $CLASSNAME$;
"""

STANDARD_EQUALS_TEMPLATE = """
"""

STANDARD_EQUALS_CODE_TEMPLATE = """
  /**
   * Checks if this object is equal to the specified object.
   * @param obj The reference object.
   * @return True when objects are equal, false otherwise.
   */
  public boolean equals(Object obj) {
    if (obj instanceof $javaclassname) {
      return (($javaclassname)obj).swigCPtr == swigCPtr;
    }
    return false;
  }

  /**
   * Returns the hash value of this object.
   * @return The hash value of this object.
   */
  public int hashCode() {
    return (int)swigCPtr;
  }
"""

CUSTOM_EQUALS_TEMPLATE = """
%javamethodmodifiers $CLASSNAME$::operator == "private";
%javamethodmodifiers $CLASSNAME$::hash "private";
%rename(equalsInternal) $CLASSNAME$::operator ==;
%ignore $CLASSNAME$::operator !=;
%rename(hashCodeInternal) $CLASSNAME$::hash() const;
"""

CUSTOM_EQUALS_CODE_TEMPLATE = """
  /**
   * Checks if this object is equal to the specified object.
   * @param obj The reference object.
   * @return True when objects are equal, false otherwise.
   */
  public boolean equals(Object obj) {
    if (obj instanceof $javaclassname) {
      return equalsInternal(($javaclassname)obj);
    }
    return false;
  }

  /**
   * Returns the hash value of this object.
   * @return The hash value of this object.
   */
  public int hashCode() {
    return hashCodeInternal();
  }
"""

def transformSwigFile(sourcePath, outPath, headerDirs):
  lines_in = [line.rstrip('\n') for line in readUncommentedLines(sourcePath)]
  lines_out = []
  class_imports = {}
  class_code = {}
  imports_linenum = None
  include_linenum = None
  stl_wrapper = False
  for line in lines_in:
    # Rename module
    match = re.search('^\s*(%module(?:[(].*[)]|)\s+)([^\s]*)\s*$', line)
    if match:
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
      if include_linenum is None: # use proper enums for Java
        lines_out.append('%include "enums.swg"')
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
      lines_code.append('%%typemap(javacode) %s %%{' % className)
      lines_code += code
      lines_code.append("%}")
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
    if fileName == 'NutiSwig.i':
      continue
    fileNameWithoutExt = fileName.split(".")[0]
    sourcePath = os.path.join(sourceDir, fileName)
    outPath = os.path.join(args.wrapperDir, fileNameWithoutExt) + "_wrap.cpp"
    proxyDir = os.path.join(args.proxyDir, ("com.carto.%s" % packageName).replace(".", "/"))
    if not os.path.isfile(sourcePath):
      continue
    if not os.path.isdir(proxyDir):
      os.makedirs(proxyDir)
    if not os.path.isdir(args.wrapperDir):
      os.makedirs(args.wrapperDir)

    includes = ["-I%s" % dir for dir in ["../scripts/swig/java", "../scripts/swig", args.moduleDir] + args.sourceDir.split(";") + [args.wrapperDir] + args.cppDir.split(";")]
    swigPath = os.path.dirname(args.swigExecutable)
    if swigPath:
      includes += ["-I%s/Lib/java" % swigPath, "-I%s/Lib" % swigPath]
    defines = ["-D%s" % define for define in args.defines.split(';') if define]
    cmd = [args.swigExecutable, "-c++", "-java", "-package", "com.carto.%s" % packageName, "-outdir", proxyDir, "-o", outPath, "-doxygen"] + defines + includes + [sourcePath]
    if subprocess.call(cmd) != 0:
      print "Error in %s" % fileName
      return False
    os.remove(os.path.join(proxyDir, fileNameWithoutExt + "Module.java"))
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
parser.add_argument('--profile', dest='profile', default=getDefaultProfile(), choices=getProfiles().keys(), help='Build profile')
parser.add_argument('--swig', dest='swigExecutable', default='swig', help='path to Swig executable')
parser.add_argument('--defines', dest='defines', default='', help='Defines for Swig')
parser.add_argument('--cppdir', dest='cppDir', default='../all/native;../extensions/all/native;../android/native', help='directories containing C++ headers')
parser.add_argument('--proxydir', dest='proxyDir', default='../generated/android-java/proxies', help='output directory for Java proxies')
parser.add_argument('--wrapperdir', dest='wrapperDir', default='../generated/android-java/wrappers', help='output directory for C++ wrappers')
parser.add_argument('--moduledir', dest='moduleDir', default='../generated/android-java/modules', help='output directory containing preprocessed Swig modules')
parser.add_argument('--sourcedir', dest='sourceDir', default='../all/modules;../extensions/all/modules;../android/modules', help='input directories containing subdirectories of Swig wrappers')

args = parser.parse_args()
args.defines += ';' + getProfiles()[args.profile].get('defines', '')

if os.path.isdir(args.wrapperDir):
  shutil.rmtree(args.wrapperDir)
if os.path.isdir(args.moduleDir):
  shutil.rmtree(args.moduleDir)
if os.path.isdir(args.proxyDir):
  shutil.rmtree(args.proxyDir)
for sourceDir in args.sourceDir.split(";"):
  if os.path.exists(sourceDir) and not transformSwigPackages(args, sourceDir, args.moduleDir, ""):
    sys.exit(-1)
if not buildSwigPackages(args, args.moduleDir, ""):
  sys.exit(-1)
