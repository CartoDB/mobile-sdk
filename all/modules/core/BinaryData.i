#ifndef _BINARYDATA_I
#define _BINARYDATA_I

%module BinaryData

!proxy_imports(carto::BinaryData)

%{
#include "core/BinaryData.h"
#include <memory>
%}

#ifdef SWIGCSHARP
%include <arrays_csharp.i>
#endif
%include <typemaps.i>
%include <std_shared_ptr.i>
%include <std_vector.i>
%include <carrays.i>
%include <cartoswig.i>

!shared_ptr(carto::BinaryData, core.BinaryData)

#ifdef SWIGCSHARP
%rename(GetData) carto::BinaryData::data;
#else
%rename(getData) carto::BinaryData::data;
#endif
%attribute(carto::BinaryData, std::size_t, Size, size)
%ignore carto::BinaryData::BinaryData(std::vector<unsigned char>);
%ignore carto::BinaryData::empty;
%ignore carto::BinaryData::getDataPtr;
!standard_equals(carto::BinaryData);

#if SWIGJAVA
%typemap(jtype) (const unsigned char* dataPtr, std::size_t size) "byte[]"
%typemap(jstype) (const unsigned char* dataPtr, std::size_t size) "byte[]"
%typemap(jni) (const unsigned char* dataPtr, std::size_t size) "jbyteArray"
%typemap(javain) (const unsigned char* dataPtr, std::size_t size) "$javainput"
%typemap(in) (const unsigned char* dataPtr, std::size_t size) {
  $1 = (unsigned char*)jenv->GetByteArrayElements($input, 0);
  $2 = (std::size_t)jenv->GetArrayLength($input);
}

%typemap(jtype) const unsigned char* data "byte[]"
%typemap(jstype) const unsigned char* data  "byte[]"
%typemap(jni) const unsigned char* data  "jbyteArray"
%typemap(javaout) const unsigned char* data {
  return $jnicall;
}
%typemap(out) const unsigned char* data {
  $result = jenv->NewByteArray(arg1->size());
  jenv->SetByteArrayRegion($result, 0, arg1->size(), (const jbyte*)$1);
}
#endif
#if SWIGCSHARP
%typemap(cscode) carto::BinaryData %{  public BinaryData(byte[] data) : this(data, (uint)data.Length) { } %}
%csmethodmodifiers carto::BinaryData::BinaryData(const unsigned char*, std::size_t) "private";
%apply unsigned char INPUT[] { const unsigned char* dataPtr }

%typemap(ctype) const unsigned char* data "void*"
%typemap(cstype) const unsigned char* data "byte[]"
%typemap(imtype) const unsigned char* "IntPtr"
%typemap(csout, excode=SWIGEXCODE) const unsigned char* data {
  $excode;
  int size = (int)Size;
  IntPtr ptr = $imcall;
  byte[] arr = new byte[size];
  System.Runtime.InteropServices.Marshal.Copy(ptr, arr, 0, size);
  return arr;
}
#endif

%include "core/BinaryData.h"

#endif
