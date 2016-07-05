#ifndef _EGLCONTEXTWRAPPER_I
#define _EGLCONTEXTWRAPPER_I

%module EGLContextWrapper

!proxy_imports(carto::EGLContextWrapper)

%{
#include "utils/EGLContextWrapper.h"
%}

%typemap(imtype) const EGLSurface, EGLSurface "System.IntPtr"
%typemap(cstype) const EGLSurface, EGLSurface "System.IntPtr"
%typemap(imtype) void* "Windows.UI.Xaml.Controls.SwapChainPanel"
%typemap(cstype) void* "Windows.UI.Xaml.Controls.SwapChainPanel"
%typemap(csin) const EGLSurface, EGLSurface "$csinput"
%typemap(csin) void* "$csinput"
%typemap(csout, excode=SWIGEXCODE) EGLSurface {
    var surface = $imcall; $excode;
    return surface;
}

%include "utils/EGLContextWrapper.h"

#endif
