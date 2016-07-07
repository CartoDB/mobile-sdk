#ifndef _BITMAP_I
#define _BITMAP_I

%module Bitmap

!proxy_imports(carto::Bitmap, core.BinaryData)

%{
#include "graphics/Bitmap.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "core/BinaryData.i"

!shared_ptr(carto::Bitmap, graphics.Bitmap)

%attribute(carto::Bitmap, unsigned int, Width, getWidth)
%attribute(carto::Bitmap, unsigned int, Height, getHeight)
%attribute(carto::Bitmap, carto::ColorFormat::ColorFormat, ColorFormat, getColorFormat)
%attribute(carto::Bitmap, unsigned int, BytesPerPixel, getBytesPerPixel)
%std_exceptions(carto::Bitmap::CreateFromCompressed)
%ignore carto::Bitmap::Bitmap(const unsigned char*, std::size_t);
%ignore carto::Bitmap::Bitmap(const unsigned char*, unsigned int, unsigned int, ColorFormat::ColorFormat, unsigned int);
%ignore carto::Bitmap::getPixelData;
%rename(getPixelData) carto::Bitmap::getPixelDataPtr;
%ignore carto::Bitmap::CreateFromCompressed(const unsigned char*, std::size_t);
!standard_equals(carto::Bitmap);

%include "graphics/Bitmap.h"

#endif
