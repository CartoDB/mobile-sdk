#ifndef _SOLIDLAYER_I
#define _SOLIDLAYER_I

%module SolidLayer

!proxy_imports(carto::SolidLayer, graphics.Color, graphics.Bitmap, layers.Layer)

%{
#include "layers/SolidLayer.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "layers/Layer.i"
%import "graphics/Color.i"
%import "graphics/Bitmap.i"

!polymorphic_shared_ptr(carto::SolidLayer, layers.SolidLayer)

%attributeval(carto::SolidLayer, carto::Color, Color, getColor, setColor)
%attributestring(carto::SolidLayer, std::shared_ptr<carto::Bitmap>, Bitmap, getBitmap, setBitmap)
%attribute(carto::SolidLayer, float, BitmapScale, getBitmapScale, setBitmapScale)

%include "layers/SolidLayer.h"

#endif
