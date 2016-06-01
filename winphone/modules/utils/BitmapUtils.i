#ifndef _BITMAPUTILS_I
#define _BITMAPUTILS_I

%module BitmapUtils

!proxy_imports(carto::BitmapUtils, graphics.Bitmap)

%{
#include "utils/BitmapUtils.h"
%}

%include <std_shared_ptr.i>
%include <std_string.i>

%import "graphics/Bitmap.i"

%include "utils/BitmapUtils.h"

#endif
