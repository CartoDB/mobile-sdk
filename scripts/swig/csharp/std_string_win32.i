/* -----------------------------------------------------------------------------
 * std_string_win32.i
 *
 * Typemaps for std::string and const std::string&
 * These are mapped to a C# String and are passed around by value.
 *
 * This is a UWP specific wrapper that uses wide string <-> UTF8 mapping, as
 * UTF8 strings are not directly supported via marshalling. For output strings,
 * we use BStr type.
 *
 * To use non-const std::string references use the following %apply.  Note 
 * that they are passed by value.
 * %apply const std::string & {std::string &};
 * ----------------------------------------------------------------------------- */

%insert(runtime) %{
#include <string>
#include <stdexcept>
#include <Windows.h>
#include <OleAuto.h>

static std::string convertWcharStrToUtf8Str(const wchar_t* wstr) {
  if (!wstr) {
    return std::string();
  }

  int size_needed = ::WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
  if (size_needed <= 0) {
    throw std::runtime_error("WideCharToMultiByte() failed: " + std::to_string(size_needed));
  }

  std::string result(size_needed, 0);
  ::WideCharToMultiByte(CP_UTF8, 0, wstr, -1, result.data(), size_needed, nullptr, nullptr);
  result.resize(size_needed - 1);
  return result;
}

static wchar_t* convertUtf8StrToBStr(const char* str) {
  if (!str) {
    return nullptr;
  }

  int size_needed = ::MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
  if (size_needed <= 0) {
    throw std::runtime_error("MultiByteToWideChar() failed: " + std::to_string(size_needed));
  }

  std::wstring wresult(size_needed, 0);
  ::MultiByteToWideChar(CP_UTF8, 0, str, -1, wresult.data(), size_needed);
  return ::SysAllocString(wresult.data());
}
%}

%{
#include <string>
%}

namespace std {

%naturalvar string;

class string;

// string
%typemap(ctype) string "wchar_t *"
%typemap(imtype,
         inattributes="[global::System.Runtime.InteropServices.MarshalAs(global::System.Runtime.InteropServices.UnmanagedType.LPWStr)]",
         outattributes="[return: global::System.Runtime.InteropServices.MarshalAs(global::System.Runtime.InteropServices.UnmanagedType.BStr)]",
         directorinattributes="[global::System.Runtime.InteropServices.MarshalAs(global::System.Runtime.InteropServices.UnmanagedType.LPWStr)]",
         directoroutattributes="[return: global::System.Runtime.InteropServices.MarshalAs(global::System.Runtime.InteropServices.UnmanagedType.BStr)]"
   ) string "string"
%typemap(cstype) string "string"

%typemap(csdirectorin) string "$iminput"
%typemap(csdirectorout) string "$cscall"

%typemap(in, canthrow=1) string 
%{ if (!$input) {
    SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpArgumentNullException, "null string", 0);
    return $null;
   }
   $1.assign(convertWcharStrToUtf8Str($input)); %}
%typemap(out) string
%{ $result = convertUtf8StrToBStr($1.c_str()); %}

%typemap(directorout, canthrow=1) string 
%{ if (!$input) {
    SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpArgumentNullException, "null string", 0);
    return $null;
   }
   $result.assign(convertWcharStrToUtf8Str($input)); %}

%typemap(directorin) string
%{ $input = convertUtf8StrToBStr($1.c_str()); %}

%typemap(csin) string "$csinput"
%typemap(csout, excode=SWIGEXCODE) string {
    string ret = $imcall;$excode
    return ret;
  }

%typemap(typecheck) string = char *;

%typemap(throws, canthrow=1) string
%{ SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, $1.c_str());
   return $null; %}

// const string &
%typemap(ctype) const string & "wchar_t *"
%typemap(imtype,
         inattributes="[global::System.Runtime.InteropServices.MarshalAs(global::System.Runtime.InteropServices.UnmanagedType.LPWStr)]",
         outattributes="[return: global::System.Runtime.InteropServices.MarshalAs(global::System.Runtime.InteropServices.UnmanagedType.BStr)]",
         directorinattributes="[global::System.Runtime.InteropServices.MarshalAs(global::System.Runtime.InteropServices.UnmanagedType.LPWStr)]",
         directoroutattributes="[return: global::System.Runtime.InteropServices.MarshalAs(global::System.Runtime.InteropServices.UnmanagedType.BStr)]"
   ) const string & "string"
%typemap(cstype) const string & "string"

%typemap(csdirectorin) const string & "$iminput"
%typemap(csdirectorout) const string & "$cscall"

%typemap(in, canthrow=1) const string &
%{ if (!$input) {
    SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpArgumentNullException, "null string", 0);
    return $null;
   }
   $*1_ltype $1_str(convertWcharStrToUtf8Str($input));
   $1 = &$1_str; %}
%typemap(out) const string &
%{ $result = convertUtf8StrToBStr($1->c_str()); %}

%typemap(csin) const string & "$csinput"
%typemap(csout, excode=SWIGEXCODE) const string & {
    string ret = $imcall;$excode
    return ret;
  }

%typemap(directorout, canthrow=1, warning=SWIGWARN_TYPEMAP_THREAD_UNSAFE_MSG) const string &
%{ if (!$input) {
    SWIG_CSharpSetPendingExceptionArgument(SWIG_CSharpArgumentNullException, "null string", 0);
    return $null;
   }
   /* possible thread/reentrant code problem */
   static $*1_ltype $1_str;
   $1_str = convertWcharStrToUtf8Str($input);
   $result = &$1_str; %}

%typemap(directorin) const string &
%{ $input = convertUtf8StrToBStr($1.c_str()); %}

%typemap(csvarin, excode=SWIGEXCODE2) const string & %{
    set {
      $imcall;$excode
    } %}
%typemap(csvarout, excode=SWIGEXCODE2) const string & %{
    get {
      string ret = $imcall;$excode
      return ret;
    } %}

%typemap(typecheck) const string & = char *;

%typemap(throws, canthrow=1) const string &
%{ SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, $1.c_str());
   return $null; %}

}
