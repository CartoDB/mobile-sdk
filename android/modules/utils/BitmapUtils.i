#ifndef _BITMAPUTILS_I
#define _BITMAPUTILS_I

%module BitmapUtils

!proxy_imports(carto::BitmapUtils, graphics.Bitmap)

%{
#include "utils/BitmapUtils.h"
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "graphics/Bitmap.i"

%typemap(in) jobject androidBitmap %{
    $1 = (jobject)$input;
%}

%typemap(out) jobject %{
    $result = $1;
%}

%typemap(javain) jobject "$javainput"
%typemap(csin) jobject "$csinput.Handle"
%typemap(javaout) jobject { return $jnicall; }
%typemap(csout, excode=SWIGEXCODE) jobject {
    var jbitmap = $imcall; $excode;
    return Java.Lang.Object.GetObject<Android.Graphics.Bitmap>(jbitmap, Android.Runtime.JniHandleOwnership.TransferLocalRef);
}

%typemap(jni) jobject "jobject"
%typemap(jtype) jobject "android.graphics.Bitmap"
%typemap(jstype) jobject "android.graphics.Bitmap"
%typemap(imtype) jobject "System.IntPtr"
%typemap(cstype) jobject "Android.Graphics.Bitmap"

%std_exceptions(carto::BitmapUtils::CreateBitmapFromAndroidBitmap)
%std_exceptions(carto::BitmapUtils::CreateAndroidBitmapFromBitmap)

%include "utils/BitmapUtils.h"

#endif
