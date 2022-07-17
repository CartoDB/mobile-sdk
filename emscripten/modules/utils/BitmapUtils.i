#ifndef _BITMAPUTILS_I
#define _BITMAPUTILS_I

%module BitmapUtils

!proxy_imports(carto::BitmapUtils, graphics.Bitmap)

%{
#include "utils/BitmapUtils.h"
#include "components/Exceptions.h"
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "graphics/Bitmap.i"

%std_exceptions(carto::BitmapUtils::CreateBitmapFromUIImage)
%std_exceptions(carto::BitmapUtils::CreateUIImageFromBitmap)

%include "utils/BitmapUtils.h"

#endif
