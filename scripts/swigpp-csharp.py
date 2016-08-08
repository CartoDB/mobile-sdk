import os
import re
import sys
import argparse
import subprocess
import shutil
from build.sdk_build_utils import *

PACKAGE_NAME_REMAP = {
  'datasources':    'DataSources',
  'packagemanager': 'PackageManager',
  'vectorelements': 'VectorElements',
  'vectortiles':    'VectorTiles',
  'wrappedcommons': 'WrappedCommons'
}

VALUE_TYPE_TEMPLATE = """
%typemap(out) $CLASSNAME$ "*($&1_ltype*)&$result = new $1_ltype($1);"
%typemap(directorin) $CLASSNAME$ "*($&1_ltype*)&$input = new $1_ltype($1);"
%typemap(csdirectorin) $CLASSNAME$ "new $TYPE$($iminput, true)"
%typemap(csout, excode=SWIGEXCODE) $CLASSNAME$ { $TYPE$ ret = new $TYPE$($imcall, true); $excode; return ret; }
%typemap(csdirectorout) $CLASSNAME$ "$TYPE$.getCPtr($cscall).Handle"

%typemap(out) const $CLASSNAME$& "*($&1_ltype)&$result = new $*1_ltype(*$1);"
%typemap(directorin) const $CLASSNAME$& "*($&1_ltype)&$input = new $*1_ltype($1);"
%typemap(csdirectorin) const $CLASSNAME$& "new $TYPE$($iminput, true)"
%typemap(csout, excode=SWIGEXCODE) const $CLASSNAME$& { $TYPE$ ret = new $TYPE$($imcall, true); $excode; return ret; }
%typemap(csdirectorout) const $CLASSNAME$& "$TYPE$.getCPtr($cscall).Handle"
"""

SHARED_PTR_TEMPLATE = """
%shared_ptr($CLASSNAME$)
"""

POLYMORPHIC_SHARED_PTR_TEMPLATE = SHARED_PTR_TEMPLATE + """
%shared_ptr($CLASSNAME$)
%apply void *VOID_INT_PTR { void * }

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
  std::string SwigGetClassName$TYPE$() const {
    std::string className = carto::ClassRegistry::GetClassName(typeid(*$self));
    if (className.empty()) {
      className = "$TYPE$";
    }
    return className;
  }

  /**
   * Returns the pointer to the connected director object. This is used internally by the SDK.
   * @return The pointer to the connected director object or null if director is not connected.
   */
  void* SwigGetDirectorObject$TYPE$() const {
    if (auto director = dynamic_cast<const carto::Director*>($self)) {
      return director->getDirectorObject();
    }
    return 0;
  }
}

%typemap(csout, excode=SWIGEXCODE) std::shared_ptr<$CLASSNAME$>, const std::shared_ptr<$CLASSNAME$>&, std::shared_ptr<$CLASSNAME$>& {
  System.IntPtr cPtr = $imcall; $excode;
  if (cPtr == System.IntPtr.Zero) {
    return null;
  }
  return $NAMESPACE$.$TYPE$.SwigCreatePolymorphicInstance$TYPE$(cPtr, true);
}

%typemap(csdirectorin) std::shared_ptr<$CLASSNAME$>, const std::shared_ptr<$CLASSNAME$>&, std::shared_ptr<$CLASSNAME$>& %{
  $NAMESPACE$.$TYPE$.SwigCreatePolymorphicInstance$TYPE$($iminput, true)
%}
"""

POLYMORPHIC_SHARED_PTR_CODE_TEMPLATE = """
  /// <summary>
  /// Creates a polymorphic instance of the given native object. This is used internally by the SDK.
  /// </summary>
  /// <param name="cPtr">The native pointer of the instance.</param>
  /// <param name="cMemoryOwn">The ownership flag.</param>
  /// <returns>The created instance.</returns>
  public static $NAMESPACE$.$TYPE$ SwigCreatePolymorphicInstance$TYPE$(System.IntPtr cPtr, bool cMemoryOwn) {
    if (cPtr == System.IntPtr.Zero) {
      return null;
    }

    HandleRef handleRef = new HandleRef(null, cPtr);
    System.IntPtr directorPtr = $NAMESPACE$.$TYPE$ModulePINVOKE.$TYPE$_SwigGetDirectorObject$TYPE$(handleRef);
    if (directorPtr != System.IntPtr.Zero) {
      return ($TYPE$)GCHandle.FromIntPtr(directorPtr).Target;
    }

    string objClassName = $NAMESPACE$.$TYPE$ModulePINVOKE.$TYPE$_SwigGetClassName$TYPE$(handleRef);
    $NAMESPACE$.$TYPE$ objInstance = null;
    try {
      #if WINDOWS_PHONE
      objInstance = ($NAMESPACE$.$TYPE$) Activator.CreateInstance(
        typeof($NAMESPACE$.$TYPE$).GetTypeInfo().Assembly.GetType("$NAMESPACE$." + objClassName),
        new object[] { cPtr, cMemoryOwn }
      );
      #else
      objInstance = ($NAMESPACE$.$TYPE$) System.Reflection.Assembly.GetExecutingAssembly().CreateInstance(
        typeName:"$NAMESPACE$." + objClassName,
        ignoreCase:false,
        bindingAttr:System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.Public | System.Reflection.BindingFlags.NonPublic,
        binder:null,
        args: new object[] { cPtr, cMemoryOwn },
        culture:null,
        activationAttributes:null
      );
      #endif
    } catch (System.Exception) { }
    if (objInstance == null) {
      Carto.Utils.Log.Error("Carto Mobile SDK: Could not instantiate class: " + objClassName);
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
  /// <summary>
  /// Checks if this object is equal to the specified object.
  /// </summary>
  /// <param name="obj">The reference object.</param>
  /// <returns>True when objects are equal, false otherwise.</returns>
  public override bool Equals(object obj) {
    if (obj is $csclassname) {
      return (($csclassname)obj).swigCPtr.Handle == swigCPtr.Handle;
    }
    return false;
  }

  /// <summary>
  /// Returns the hash value of this object.
  /// </summary>
  /// <returns>The hash value of this object.</returns>
  public override int GetHashCode() {
    return (int)swigCPtr.Handle;
  }
"""

CUSTOM_EQUALS_TEMPLATE = """
%csmethodmodifiers $CLASSNAME$::operator == "private";
%csmethodmodifiers $CLASSNAME$::hash "private";
%rename(EqualsInternal) $CLASSNAME$::operator ==;
%ignore $CLASSNAME$::operator !=;
%rename(GetHashCodeInternal) $CLASSNAME$::hash() const;
"""

CUSTOM_EQUALS_CODE_TEMPLATE = """
  /// <summary>
  /// Checks if this object is equal to the specified object.
  /// </summary>
  /// <param name="obj">The reference object.</param>
  /// <returns>True when objects are equal, false otherwise.</returns>
  public override bool Equals(object obj) {
    if (obj is $csclassname) {
      return EqualsInternal(($csclassname)obj);
    }
    return false;
  }

  /// <summary>
  /// Returns the hash value of this object.
  /// </summary>
  /// <returns>The hash value of this object.</returns>
  public override int GetHashCode() {
    return GetHashCodeInternal();
  }
"""

CUSTOM_TOSTRING_TEMPLATE = """
%csmethodmodifiers $CLASSNAME$::toString "public override";
"""

POLYMORPHIC_RO_ATTRIBUTE_TEMPLATE = """
%csmethodmodifiers $CLASSNAME$::$GETTER$ "private";
%rename($GETTER$Internal) $CLASSNAME$::$GETTER$;
"""

POLYMORPHIC_RO_ATTRIBUTE_CODE_TEMPLATE = """
  /// <summary>
  /// The $TYPE$ property.
  /// </summary>
  $MODIFIER$ $NAMESPACE$.$TYPE$ $NAME$ {
    get { return $GETTER$Internal(); }
  }
"""

POLYMORPHIC_RW_ATTRIBUTE_TEMPLATE = """
%csmethodmodifiers $CLASSNAME$::$GETTER$ "private";
%rename($GETTER$Internal) $CLASSNAME$::$GETTER$;
%csmethodmodifiers $CLASSNAME$::$SETTER$ "private";
%rename($SETTER$Internal) $CLASSNAME$::$SETTER$;
"""

POLYMORPHIC_RW_ATTRIBUTE_CODE_TEMPLATE = """
  /// <summary>
  /// The $TYPE$ property.
  /// </summary>
  $MODIFIER$ $NAMESPACE$.$TYPE$ $NAME$ {
    get { return $GETTER$Internal(); }
    set { $SETTER$Internal(value); }
  }
"""

def getNamespace(packageName):
  names = packageName.split(".")
  return (".".join([PACKAGE_NAME_REMAP.get(name, name.capitalize()) for name in names[:-1]]))

def fixPackageName(packageName):
  return PACKAGE_NAME_REMAP.get(packageName, packageName.capitalize())

def transformSwigFile(sourcePath, outPath, headerDirs):
  lines_in = [line.rstrip('\n') for line in readUncommentedLines(sourcePath)]
  lines_out = []
  class_imports = {}
  class_code = {}
  method_modifiers = {}
  imports_linenum = None
  include_linenum = None
  for line in lines_in:
    # Rename module
    match = re.search('^\s*(%module(?:[(].*[)]|)\s+)([^\s]*)\s*$', line)
    if match:
      line = '%s%sModule' % (match.group(1), match.group(2))

    # Language-specific method modifiers
    match = re.search('^\s*%(java|cs|objc)methodmodifiers\s+(\S+)\s+"([^"]*)".*$', line)
    if match:
      lang = match.group(1)
      methodName = match.group(2)
      modifier = match.group(3)
      if lang != 'cs':
        continue 
      method_modifiers[methodName] = modifier

    # Language-specific rename declarations
    match = re.search('^\s*!(java|cs|objc)_rename(.*)$', line)
    if match:
      lang = match.group(1)
      if lang != 'cs':
        continue
      line = '%%rename%s' % match.group(2)

    # Polymorphic read-write attribute
    match = re.search('^\s*!attributestring_polymorphic\s*[(]([^,]*),([^,]*),([^,]*),([^,]*),([^)]*)[)].*', line)
    if match:
      className = match.group(1).strip()
      csNamespace = 'Carto.%s' % getNamespace(match.group(2).strip())
      csClass = match.group(2).strip().split(".")[-1]
      attrName = match.group(3).strip()
      modifier = method_modifiers.get("%s::%s" % (className, attrName), "public")
      args = { 'CLASSNAME': match.group(1).strip(), 'TYPE': csClass, 'NAMESPACE': csNamespace, 'NAME': attrName, 'GETTER': match.group(4).strip(), 'SETTER': match.group(5).strip(), 'MODIFIER': modifier }
      code = class_code.get(className, [])
      code += applyTemplate(POLYMORPHIC_RW_ATTRIBUTE_CODE_TEMPLATE, args)
      class_code[className] = code
      lines_out += applyTemplate(POLYMORPHIC_RW_ATTRIBUTE_TEMPLATE, args)
      continue

    # Polymorphic read-only attribute
    match = re.search('^\s*!attributestring_polymorphic\s*[(]([^,]*),([^,]*),([^,]*),([^)]*)[)].*', line)
    if match:
      className = match.group(1).strip()
      csNamespace = 'Carto.%s' % getNamespace(match.group(2).strip())
      csClass = match.group(2).strip().split(".")[-1]
      attrName = match.group(3).strip()
      modifier = method_modifiers.get("%s::%s" % (className, attrName), "public")
      args = { 'CLASSNAME': match.group(1).strip(), 'TYPE': csClass, 'NAMESPACE': csNamespace, 'NAME': attrName, 'GETTER': match.group(4).strip(), 'MODIFIER': modifier }
      code = class_code.get(className, [])
      code += applyTemplate(POLYMORPHIC_RO_ATTRIBUTE_CODE_TEMPLATE, args)
      class_code[className] = code
      lines_out += applyTemplate(POLYMORPHIC_RO_ATTRIBUTE_TEMPLATE, args)
      continue

    # Detect value_type directive
    match = re.search('^\s*!value_type\s*[(]([^)]*),([^)]*)[)].*$', line)
    if match:
      className = match.group(1).strip()
      csClass = match.group(2).strip().split(".")[-1]
      args = { 'CLASSNAME': match.group(1).strip(), 'TYPE': csClass }
      lines_out += applyTemplate(VALUE_TYPE_TEMPLATE, args)
      continue

    # Detect shared_ptr directive
    match = re.search('^\s*!shared_ptr\s*[(]([^)]*),([^)]*)[)].*$', line)
    if match:
      className = match.group(1).strip()
      csClass = match.group(2).strip().split(".")[-1]
      args = { 'CLASSNAME': match.group(1).strip(), 'TYPE': csClass }
      lines_out += applyTemplate(SHARED_PTR_TEMPLATE, args)
      continue

    # Polymorphic shared_ptr
    match = re.search('^\s*!polymorphic_shared_ptr\s*[(]([^,]*),([^)]*)[)].*', line)
    if match:
      className = match.group(1).strip()
      csNamespace = 'Carto.%s' % getNamespace(match.group(2).strip())
      csClass = match.group(2).strip().split(".")[-1]
      args = { 'CLASSNAME': match.group(1).strip(), 'TYPE': csClass, 'NAMESPACE': csNamespace }
      code = class_code.get(className, [])
      code += applyTemplate(POLYMORPHIC_SHARED_PTR_CODE_TEMPLATE, args)
      class_code[className] = code
      lines_out += applyTemplate(POLYMORPHIC_SHARED_PTR_TEMPLATE, args)
      continue

    # Value-template
    match = re.search('^\s*!value_template\s*[(]([^)]*),([^)]*)[)].*$', line)
    if match:
      className = match.group(1).strip()
      csClass = match.group(2).strip().split(".")[-1]
      args = { 'CLASSNAME': match.group(1).strip(), 'TYPE': csClass }
      lines_out += applyTemplate(VALUE_TEMPLATE_TEMPLATE, args)
      continue

    # Imports
    match = re.search('^\s*!(proxy|java|cs|objc)_imports\s*[(]([^)]*)[)](.*)$', line)
    if match:
      lang = match.group(1)
      parts = [part.strip() for part in match.group(2).split(",")]
      className = parts[0]
      if lang == 'proxy':
        class_imports[className] = class_imports.get(className, []) + ['using Carto.%s;' % getNamespace(part) for part in parts[1:]]
      elif lang == 'cs':
        class_imports[className] = class_imports.get(className, []) + ['using %s;' % part for part in parts[1:]]
      else:
        continue
      line = match.group(3)
      if imports_linenum is None:
        imports_linenum = len(lines_out)

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
      className = match.group(1).strip()
      args = { 'CLASSNAME': match.group(1).strip() }
      lines_out += applyTemplate(CUSTOM_TOSTRING_TEMPLATE, args)
      continue

    # cscode import
    match = re.search('^\s*%typemap[(]cscode[)]\s*(.*)\s*%{(.*)%}', line)
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

    # Rename all methods to CapCase, add this before including C++ code
    match = re.search('^\s*%include\s+"(.*)".*$', line)
    if match:
      includeName = match.group(1)
      if includeName != "NutiSwig.i":
        # This is a huge hack: we will capitalize all method names starting with lower case letters. But we will do this only for carto:: classes
        for n in range(ord('a'), ord('z') + 1):
          c = chr(n)
          lines_out.append('%%rename("%%(regex:/::%s([^:]*)$/%s\\\\1/)s", fullname=1, regextarget=1, %%$isfunction) "^carto::.+::%s[^:]*$";' % (c, c.upper(), c))
        lines_out.append('%rename("%(camelcase)s", sourcefmt="%(undercase)s", %$isenumitem) "";')
        lines_out.append('')

    lines_out.append(line)

  # Add imports
  if imports_linenum is not None:
    lines_import = []
    for className, imports in class_imports.items():
      lines_import.append("%%typemap(csimports) %s %%{" % className)
      lines_import += ['using System;', 'using System.Runtime.InteropServices;', '#if WINDOWS_PHONE', 'using System.Reflection;', '#endif'] + list(set(imports))
      lines_import.append("%}")
    lines_import.append("")
    lines_import.append("%pragma(csharp) imclassimports=%{")
    for className, imports in class_imports.items():
      lines_import += ['using System;', 'using System.Runtime.InteropServices;'] + list(set(imports))
    lines_import.append("%}")
    lines_out = lines_out[:imports_linenum] + lines_import + lines_out[imports_linenum:]
    if include_linenum > imports_linenum:
      include_linenum += len(lines_import)

  # Add typemap code
  if include_linenum is not None:
    lines_code = []
    for className, code in class_code.items():
      lines_code.append('%%typemap(cscode) %s %%{' % className)
      lines_code += code
      lines_code.append("%}")
    lines_out = lines_out[:include_linenum] + lines_code + lines_out[include_linenum:]

  # Output
  with open(outPath, 'w') as f:
    f.writelines([line + '\n' for line in lines_out])

def transformSwigPackage(args, sourceDir, outDir, packageName):
  for fileName in os.listdir(sourceDir):
    if fileName == 'NutiSwig.i' or not fileName.endswith(".i"):
      continue
    if fileName in ['AndroidUtils.i'] and args.target != 'android':
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
    packageName = (basePackageName + '.' if basePackageName else '') + fixPackageName(dirName)
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
    outExt = ".cpp"
    if args.target == "ios" and fileName in ("AssetUtils.i", "BitmapUtils.i"):
      outExt = ".mm"
    outPath = os.path.join(args.wrapperDir, fileNameWithoutExt) + "_wrap" + outExt
    if not os.path.isfile(sourcePath):
      continue
    if not os.path.isdir(args.proxyDir):
      os.makedirs(args.proxyDir)
    if not os.path.isdir(args.wrapperDir):
      os.makedirs(args.wrapperDir)

    includes = ["-I%s" % dir for dir in ["../scripts/swig/csharp", "../scripts/swig", args.moduleDir] + args.sourceDir.split(";") + [args.wrapperDir] + args.cppDir.split(";")]
    swigPath = os.path.dirname(args.swigExecutable)
    if swigPath:
      includes += ["-I%s/Lib/csharp" % swigPath, "-I%s/Lib" % swigPath]
    defines = ["-D%s" % define for define in args.defines.split(';') if define]
    cmd = [args.swigExecutable, "-c++", "-csharp", "-namespace", "Carto.%s" % packageName, "-dllimport", args.dllName, "-outdir", args.proxyDir, "-o", outPath, "-doxygen"] + defines + includes + [sourcePath]
    if subprocess.call(cmd) != 0:
      print "Error in %s" % fileName
      return False
    os.remove(os.path.join(args.proxyDir, fileNameWithoutExt + "Module.cs"))
  return True

def buildSwigPackages(args, sourceDir, basePackageName):
  for dirName in os.listdir(sourceDir):
    sourcePath = os.path.join(sourceDir, dirName)
    if not os.path.isdir(sourcePath) or dirName.startswith("."):
      continue
    packageName = (basePackageName + '.' if basePackageName else '') + fixPackageName(dirName)
    buildSwigPackages(args, sourcePath, packageName)
    if not buildSwigPackage(args, sourcePath, packageName):
      return False
  return True

parser = argparse.ArgumentParser()
parser.add_argument('--profile', dest='profile', default=getDefaultProfile(), choices=getProfiles().keys(), help='Build profile')
parser.add_argument('--swig', dest='swigExecutable', default='swig', help='path to Swig executable')
parser.add_argument('--dll', dest='dllName', default='carto_mobile_sdk', help='name of the DLL (Android only)')
parser.add_argument('--defines', dest='defines', default='', help='Defines for Swig')
parser.add_argument('--cppdir', dest='cppDir', default='../all/native;../extensions/native;../{target}/native', help='directories containing C++ headers')
parser.add_argument('--proxydir', dest='proxyDir', default='../generated/{target}-csharp/proxies', help='output directory for C# proxies')
parser.add_argument('--wrapperdir', dest='wrapperDir', default='../generated/{target}-csharp/wrappers', help='output directory for C++ wrappers')
parser.add_argument('--moduledir', dest='moduleDir', default='../generated/{target}-csharp/modules', help='output directory containing preprocessed Swig modules')
parser.add_argument('--sourcedir', dest='sourceDir', default='../all/modules;../extensions/modules;../{target}/modules', help='input directories containing subdirectories of Swig wrappers')
parser.add_argument(dest='target', choices=['android', 'ios', 'winphone'], help='target platform')

args = parser.parse_args()
if args.target == "ios":
  args.dllName = "__Internal"
if args.target == 'winphone':
  args.defines += ';WINDOWS_PHONE'
args.cppDir = args.cppDir.replace("{target}", args.target)
args.proxyDir = args.proxyDir.replace("{target}", args.target)
args.wrapperDir = args.wrapperDir.replace("{target}", args.target)
args.moduleDir = args.moduleDir.replace("{target}", args.target)
args.sourceDir = args.sourceDir.replace("{target}", args.target)
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
