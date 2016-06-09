import os
import re
import sys
import argparse
import subprocess
import shutil
from build.sdk_build_utils import *

VALUE_TYPE_TEMPLATE = """
%typemap(out) $CLASSNAME$ "*($&1_ltype*)&$result = new $1_ltype($1);"
%typemap(directorin) $CLASSNAME$ "*($&1_ltype*)&$input = new $1_ltype($1);"
%typemap(objcdirectorin) $CLASSNAME$ "[[$TYPE$ alloc] initWithCptr:$iminput swigOwnCObject:YES]"
%typemap(objcout) $CLASSNAME$ { return [[$TYPE$ alloc] initWithCptr:$imcall swigOwnCObject:YES]; }
%typemap(objcdirectorout) $CLASSNAME$ "[($objccall) getCptr]"

%typemap(out) const $CLASSNAME$& "*($&1_ltype)&$result = new $*1_ltype(*$1);"
%typemap(directorin) const $CLASSNAME$& "*($&1_ltype)&$input = new $*1_ltype($1);"
%typemap(objcdirectorin) const $CLASSNAME$& "[[$TYPE$ alloc] initWithCptr:$iminput swigOwnCObject:YES]"
%typemap(objcout) const $CLASSNAME$& { return [[$TYPE$ alloc] initWithCptr:$imcall swigOwnCObject:YES]; }
%typemap(objcdirectorout) const $CLASSNAME$& "[($objccall) getCptr]"
"""

SHARED_PTR_TEMPLATE = """
%shared_ptr($CLASSNAME$)
"""

POLYMORPHIC_SHARED_PTR_TEMPLATE = SHARED_PTR_TEMPLATE + """
%{
#include "components/ClassRegistry.h"
#include "components/Director.h"
static carto::ClassRegistry::Entry $TYPE$RegistryEntry(typeid(const $CLASSNAME$&), "$RAWTYPE$");
%}

%extend $CLASSNAME$ {
  /**
    * Returns the actual class name of this object. This is used internally by the SDK.
    * @return The class name of this object.
    */
  std::string swigGetClassName() const {
    std::string className = carto::ClassRegistry::GetClassName(typeid(*$self));
    if (className.empty()) {
      className = "$RAWTYPE$";
    }
    return className;
  }

  /**
    * Returns the pointer to the connected director object. This is used internally by the SDK.
    * @return The pointer to the connected director object or null if director is not connected.
    */
  void* swigGetDirectorObject() const {
    if (auto director = dynamic_cast<const carto::Director*>($self)) {
      return director->getDirectorObject();
    }
    return 0;
  }
}

%typemap(objcout) std::shared_ptr<$CLASSNAME$>, const std::shared_ptr<$CLASSNAME$>&, std::shared_ptr<$CLASSNAME$>& {
  void* cPtr = $imcall;
  if (!cPtr) {
    return nil;
  }
  return [$TYPE$ swigCreatePolymorphicInstance:cPtr swigOwnCObject:YES];
}

%typemap(objcdirectorin) std::shared_ptr<$CLASSNAME$>, const std::shared_ptr<$CLASSNAME$>&, std::shared_ptr<$CLASSNAME$>& %{
  [$TYPE$ swigCreatePolymorphicInstance:$iminput swigOwnCObject:YES]
%}
"""

POLYMORPHIC_SHARED_PTR_INTERFACE_TEMPLATE = """
/**
 * Creates a polymorphic instance of the given native object. This is used internally by the SDK.
 */
+($TYPE$*)swigCreatePolymorphicInstance:(void*)cPtr swigOwnCObject:(BOOL)cMemoryOwn;
"""

POLYMORPHIC_SHARED_PTR_CODE_TEMPLATE = """
+($TYPE$*)swigCreatePolymorphicInstance:(void*)cPtr swigOwnCObject:(BOOL)cMemoryOwn {
  if (!cPtr) {
    return nil;
  }

  void* directorPtr = Objc_$RAWTYPE$_$TYPE$_swigGetDirectorObject(cPtr);
  if (directorPtr) {
    return ($TYPE$*)CFBridgingRelease(directorPtr);
  }

  NSString* objClassName = [NSString stringWithFormat:@"NT%@", Objc_$RAWTYPE$_$TYPE$_swigGetClassName(cPtr)];
  $TYPE$* objInstance = nil;
  @try {
    objInstance = [[NSClassFromString(objClassName) alloc] initWithCptr:cPtr swigOwnCObject:cMemoryOwn];
  }
  @catch (NSException* e) {
  }
  if (!objInstance) {
    NSLog(@"Carto Mobile SDK: Could not instantiate class: %@", objClassName);
  }
  return objInstance;
}
"""

VALUE_TEMPLATE_TEMPLATE = """
  %template($TYPE$) $CLASSNAME$;
"""

EQUALS_INTERFACE_TEMPLATE = """
/**  
 * Checks if this object is equal to the specified object.
 * @return True when objects are equal, false otherwise.
 */
-(BOOL)isEqual:(id)object;

/**
 * Returns the hash value of this object.
 * @return The hash value of this object.
 */
-(NSUInteger)hash;
"""

CUSTOM_EQUALS_TEMPLATE = """
%rename(isEqualInternal) $CLASSNAME$::operator ==;
%ignore $CLASSNAME$::operator !=;
%rename(hashInternal) $CLASSNAME$::hash() const;
"""

CUSTOM_EQUALS_CODE_TEMPLATE = """
-(BOOL)isEqual:(id)object {
  if (self == object) {
    return YES;
  }
  if (![object isKindOfClass:[$objcclassname class]]) {
    return NO;
  }
  return [self isEqualInternal:($objcclassname *)object];
}

-(NSUInteger)hash {
  return (NSUInteger)[self hashInternal];
}
"""

STANDARD_EQUALS_TEMPLATE = """
"""

STANDARD_EQUALS_CODE_TEMPLATE = """
-(BOOL)isEqual:(id)object {
  if (self == object) {
    return YES;
  }
  if (![object isKindOfClass:[$objcclassname class]]) {
    return NO;
  }
  return swigCPtr == [($objcclassname *)object getCptr];
}

-(NSUInteger)hash {
  return (NSUInteger)swigCPtr;
}
"""

def fixProxyCode(fileName):
  if not os.path.isfile(fileName):
    return

  lines_in = readLines(fileName)

  lines_out = []
  for line in lines_in:
    # Rename #import "XXX_proxy.h" -> #import "NTXXX.h" / same for #include
    line = re.sub('#(import|include)\s+"(.*)_proxy.h"', '#\\1 "NT\\2.h"', line)

    lines_out.append(line)

  with open(fileName, 'w') as f:
    f.writelines(lines_out)

def transformSwigFile(sourcePath, outPath, moduleDirs, headerDirs):
  lines_in = [line.rstrip('\n') for line in readUncommentedLines(sourcePath)]
  lines_out = []
  class_imports = {}
  class_code = {}
  class_data = {}
  class_interface = {}
  imports_linenum = None
  include_linenum = None
  stl_wrapper = False
  for line in lines_in:
    # Language-specific rename declarations
    match = re.search('^\s*!(java|cs|objc)_rename(.*)$', line)
    if match:
      lang = match.group(1)
      if lang != 'objc':
        continue
      line = '%%rename%s' % match.group(2)

    # Language-specific method modifiers
    match = re.search('^\s*%(java|cs|objc)methodmodifiers.*$', line)
    if match:
      lang = match.group(1)
      if lang != 'objc':
        continue

    # Attributes
    match = re.search('^\s*(%|!)(static|)attribute.*$', line)
    if match:
      continue

    # Detect value_type directive
    match = re.search('^\s*!value_type\s*[(]([^)]*),([^)]*)[)].*$', line)
    if match:
      className = match.group(1).strip()
      objcClass = 'NT%s' % match.group(2).strip().split(".")[-1]
      args = { 'CLASSNAME': match.group(1).strip(), 'TYPE': objcClass }
      lines_out += applyTemplate(VALUE_TYPE_TEMPLATE, args)
      continue

    # Detect shared_ptr directive
    match = re.search('^\s*!shared_ptr\s*[(]([^)]*),([^)]*)[)].*$', line)
    if match:
      className = match.group(1).strip()
      objcClass = 'NT%s' % match.group(2).strip().split(".")[-1]
      args = { 'CLASSNAME': match.group(1).strip(), 'TYPE': objcClass }
      lines_out += applyTemplate(SHARED_PTR_TEMPLATE, args)
      continue

    # Polymorphic shared_ptr
    match = re.search('^\s*!polymorphic_shared_ptr\s*[(]([^,]*),([^)]*)[)].*', line)
    if match:
      className = match.group(1).strip()
      objcClass = 'NT%s' % match.group(2).strip().split(".")[-1]
      args = { 'CLASSNAME': match.group(1).strip(), 'TYPE': objcClass, 'RAWTYPE': objcClass[2:] }
      interface = class_interface.get(className, [])
      interface += applyTemplate(POLYMORPHIC_SHARED_PTR_INTERFACE_TEMPLATE, args)
      class_interface[className] = interface
      code = class_code.get(className, [])
      code += applyTemplate(POLYMORPHIC_SHARED_PTR_CODE_TEMPLATE, args)
      class_code[className] = code
      lines_out += applyTemplate(POLYMORPHIC_SHARED_PTR_TEMPLATE, args)
      continue

    # Value-template
    match = re.search('^\s*!value_template\s*[(]([^)]*),([^)]*)[)].*$', line)
    if match:
      className = match.group(1).strip()
      objcClass = 'NT%s' % match.group(2).strip().split(".")[-1]
      args = { 'CLASSNAME': match.group(1).strip(), 'TYPE': objcClass }
      lines_out += applyTemplate(VALUE_TEMPLATE_TEMPLATE, args)
      continue

    # Standard equals
    match = re.search('^\s*!standard_equals\s*[(]([^)]*)[)].*', line)
    if match:
      className = match.group(1).strip()
      args = { 'CLASSNAME': match.group(1).strip() }
      interface = class_interface.get(className, [])
      interface += applyTemplate(EQUALS_INTERFACE_TEMPLATE, args)
      class_interface[className] = interface
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
      interface = class_interface.get(className, [])
      interface += applyTemplate(EQUALS_INTERFACE_TEMPLATE, args)
      class_interface[className] = interface
      code = class_code.get(className, [])
      code += applyTemplate(CUSTOM_EQUALS_CODE_TEMPLATE, args)
      class_code[className] = code
      lines_out += applyTemplate(CUSTOM_EQUALS_TEMPLATE, args)
      continue

    # Custom toString
    match = re.search('^\s*!custom_tostring\s*[(]([^)]*)[)].*', line)
    if match:
      # Automatically renamed by general rule declared below
      continue

    # Imports
    match = re.search('^\s*!(proxy|java|cs|objc)_imports\s*[(]([^)]*)[)](.*)$', line)
    if match:
      lang = match.group(1)
      parts = [part.strip() for part in match.group(2).split(",")]
      className = parts[0]
      if lang == 'proxy':
        import_decls = []
        class_decls = []
        for part in parts[1:]:
          # Check if corresponding module actually exists... Otherwise do not add #import
          found = [moduleDir for moduleDir in moduleDirs if os.path.isfile("%s/%s.i" % (moduleDir, part.replace('.', '/')))]
          if found:
            import_decls.append(part.split(".")[-1])
          else:
            class_decls.append(part.split(".")[-1])
        class_imports[className] = class_imports.get(className, []) + ['#import "%s_proxy.h"' % decl for decl in import_decls] + ['@class NT%s;' % decl for decl in class_decls]
      elif lang == 'objc':
        class_imports[className] = class_imports.get(className, []) + ['#import %s' % part for part in parts[1:]]
      else:
        continue
      line = match.group(3)
      if imports_linenum is None:
        imports_linenum = len(lines_out)

    # Check for STL templates
    match = re.search('^\s*%template\(.*\)\s*std::.*$', line)
    if match:
      stl_wrapper = True

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
        lines_out.append('%rename("description", fullname=1, regextarget=1, %$isfunction)  "^carto::.+::toString()$";')
        lines_out.append('%rename("NT%s", %$isclass) "";')
        lines_out.append('%rename("NT%s", %$isenum) "";')
        lines_out.append('%rename("NT_%s", %$isenumitem) "";')
        lines_out.append('')

    lines_out.append(line)

  # Add imports
  if imports_linenum is not None:
    lines_import = []
    for className, imports in class_imports.items():
      lines_import.append("%pragma(objc) declmoduleimports=%{")
      lines_import += imports
      lines_import.append("%}")
    lines_out = lines_out[:imports_linenum] + lines_import + lines_out[imports_linenum:]
    if include_linenum > imports_linenum:
      include_linenum += len(lines_import)

  # Add typemap code
  if include_linenum is not None:
    lines_code = []
    for className, code in class_interface.items():
      lines_code.append('%%typemap(objcinterfacecode) %s %%{' % className)
      lines_code.append('{')
      lines_code.append('  void *swigCPtr;')
      lines_code.append('  BOOL swigCMemOwn;')
      lines_code += class_data.get(className, [])
      lines_code.append('}')
      lines_code.append('-(void*)getCptr;')
      lines_code.append('-(id)initWithCptr: (void*)cptr swigOwnCObject: (BOOL)ownCObject;')
      lines_code += code
      lines_code.append('%}')
      
      lines_code.append('%%typemap(objcinterfacecode_derived) %s %%{' % className)
      lines_code.append('-(void*)getCptr;')
      lines_code.append('-(id)initWithCptr: (void*)cptr swigOwnCObject: (BOOL)ownCObject;')
      lines_code += code
      lines_code.append('%}')

    for className, code in class_code.items():
      lines_code.append('%%typemap(objcimplementationcode) %s %%{' % className)
      lines_code.append('-(void*)getCptr {')
      lines_code.append('  return swigCPtr;')
      lines_code.append('}')
      lines_code.append('-(id)initWithCptr: (void*)cptr swigOwnCObject: (BOOL)ownCObject {')
      lines_code.append('  swigCPtr = cptr;')
      lines_code.append('  swigCMemOwn = ownCObject;')
      lines_code.append('  self = [super init];')
      lines_code.append('  return self;')
      lines_code.append('}')
      lines_code += code
      lines_code.append('%}')

      lines_code.append('%%typemap(objcimplementationcode_derived) %s %%{' % className)
      lines_code.append('-(void*)getCptr {')
      lines_code.append('  return swigCPtr;')
      lines_code.append('}')
      lines_code.append('-(id)initWithCptr: (void*)cptr swigOwnCObject: (BOOL)ownCObject {')
      lines_code.append('  self = [super initWithCptr:cptr swigOwnCObject:ownCObject];')
      lines_code.append('  return self;')
      lines_code.append('}')
      lines_code += code
      lines_code.append('%}')

    lines_out = lines_out[:include_linenum] + lines_code + lines_out[include_linenum:]

  # Output
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
    moduleDirs = args.sourceDir.split(";")
    headerDirs = args.cppDir.split(";")
    transformSwigFile(sourcePath, outPath, moduleDirs, headerDirs)
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
    outPath = os.path.join(args.wrapperDir, fileNameWithoutExt) + "_wrap.mm"
    proxyDir = args.proxyDir
    proxyCPath = os.path.join(proxyDir, "NT%s.mm" % fileNameWithoutExt)
    proxyHPath = proxyCPath[:-3] + ".h"
    if not os.path.isfile(sourcePath):
      continue
    if not os.path.isdir(proxyDir):
      os.makedirs(proxyDir)
    if not os.path.isdir(args.wrapperDir):
      os.makedirs(args.wrapperDir)

    includes = ["-I%s" % dir for dir in ["../scripts/swig/objc", "../scripts/swig", args.moduleDir] + args.sourceDir.split(";") + [args.wrapperDir] + args.cppDir.split(";")]
    swigPath = os.path.dirname(args.swigExecutable)
    if swigPath:
      includes += ["-I%s/Lib/objc" % swigPath, "-I%s/Lib" % swigPath]
    defines = ["-D%s" % define for define in args.defines.split(';') if define]
    cmd = [args.swigExecutable, "-c++", "-doxygen", "-objc", "-outdir", proxyDir, "-o", outPath] + defines + includes + [sourcePath]
    if subprocess.call(cmd) != 0:
      print "Error in %s" % fileName
      return False
    shutil.move(os.path.join(proxyDir, fileNameWithoutExt + "_proxy.mm"), proxyCPath)
    shutil.move(os.path.join(proxyDir, fileNameWithoutExt + "_proxy.h"), proxyHPath)
    fixProxyCode(proxyCPath)
    fixProxyCode(proxyHPath)
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
parser.add_argument('--profile', dest='profile', default='standard', choices=getProfiles().keys(), help='Build profile')
parser.add_argument('--swig', dest='swigExecutable', default='swig', help='path to Swig executable')
parser.add_argument('--defines', dest='defines', default='', help='Defines for Swig')
parser.add_argument('--cppdir', dest='cppDir', default='../all/native;../all/libs;../ios/native', help='directories containing C++ headers')
parser.add_argument('--proxydir', dest='proxyDir', default='../generated/ios-objc/proxies', help='output directory for Objective C proxies')
parser.add_argument('--wrapperdir', dest='wrapperDir', default='../generated/ios-objc/wrappers', help='output directory for C++ wrappers')
parser.add_argument('--moduledir', dest='moduleDir', default='../generated/ios-objc/modules', help='output directory containing preprocessed Swig modules')
parser.add_argument('--sourcedir', dest='sourceDir', default='../all/modules;../ios/modules', help='input directories containing subdirectories of Swig wrappers')

args = parser.parse_args()
args.defines += ';' + getProfiles()[args.profile].get('defines', '')

if os.path.isdir(args.wrapperDir):
  shutil.rmtree(args.wrapperDir)
if os.path.isdir(args.moduleDir):
  shutil.rmtree(args.moduleDir)
if os.path.isdir(args.proxyDir):
  shutil.rmtree(args.proxyDir)
for sourceDir in args.sourceDir.split(";"):
  if not transformSwigPackages(args, sourceDir, args.moduleDir, ""):
    sys.exit(-1)
if not buildSwigPackages(args, args.moduleDir, ""):
  sys.exit(-1)
