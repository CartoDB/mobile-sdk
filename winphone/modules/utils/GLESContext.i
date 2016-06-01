#ifndef _GLESCONTEXT_I
#define _GLESCONTEXT_I

%module GLESContext

!proxy_imports(carto::GLESContext)

%{
#include "utils/GLESContext.h"
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

%include "utils/GLESContext.h"

#endif
