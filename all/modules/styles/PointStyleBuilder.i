#ifndef _POINTSTYLEBUILDER_I
#define _POINTSTYLEBUILDER_I

%module PointStyleBuilder

!proxy_imports(carto::PointStyleBuilder, graphics.Bitmap, styles.PointStyle, styles.StyleBuilder)

%{
#include "styles/PointStyleBuilder.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "styles/PointStyle.i"
%import "styles/StyleBuilder.i"

!polymorphic_shared_ptr(carto::PointStyleBuilder, styles.PointStyleBuilder)

%attribute(carto::PointStyleBuilder, float, Size, getSize, setSize)
%attribute(carto::PointStyleBuilder, float, ClickSize, getClickSize, setClickSize)
%attributestring(carto::PointStyleBuilder, std::shared_ptr<carto::Bitmap>, Bitmap, getBitmap, setBitmap)
%std_exceptions(carto::PointStyleBuilder::setBitmap)

%include "styles/PointStyleBuilder.h"

#endif
