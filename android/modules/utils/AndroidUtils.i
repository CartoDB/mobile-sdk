#ifndef _ANDROIDUTILS_I
#define _ANDROIDUTILS_I

#pragma SWIG nowarn=325

%module AndroidUtils

%{
#include "utils/AndroidUtils.h"	
%}

%include <std_string.i>

#ifdef SWIGJAVA
%typemap(in) JNIEnv* %{
    $1 = jenv;
%}
#endif
%typemap(javain) JNIEnv* "$javainput"
%typemap(csin) JNIEnv* "$csinput"
%typemap(jni) JNIEnv* "jobject"
%typemap(jtype) JNIEnv* "Object"
%typemap(jstype) JNIEnv* "Object"
%typemap(imtype) JNIEnv* "System.IntPtr"
%typemap(cstype) JNIEnv* "System.IntPtr"

%typemap(in) jobject context %{
        $1 = (jobject)$input;
%}
%typemap(javain) jobject context "$javainput"
%typemap(csin) jobject context "$csinput.Handle"
%typemap(jni) jobject context "jobject"
%typemap(jtype) jobject context "android.content.Context"
%typemap(jstype) jobject context "android.content.Context"
%typemap(imtype) jobject context "System.IntPtr"
%typemap(cstype) jobject context "Android.Content.Context"

%ignore carto::AndroidUtils::JNILocalFrame;
%ignore carto::AndroidUtils::GetJVM;
%ignore carto::AndroidUtils::GetCurrentThreadJNIEnv;
%ignore carto::AndroidUtils::GetPackageName;
%ignore carto::AndroidUtils::GetDeviceId;

%include "utils/AndroidUtils.h"

#endif
