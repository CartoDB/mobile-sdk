#ifndef _LABEL_I
#define _LABEL_I

%module Label

!proxy_imports(carto::Label, core.MapPos, graphics.Bitmap, geometry.Geometry, geometry.PointGeometry, styles.LabelStyle, vectorelements.Billboard)

%{
#include "vectorelements/Label.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "graphics/Bitmap.i"
%import "styles/LabelStyle.i"
%import "vectorelements/Billboard.i"

!polymorphic_shared_ptr(carto::Label, vectorelements.Label)

!attributestring_polymorphic(carto::Label, styles.LabelStyle, Style, getStyle, setStyle)
%std_exceptions(carto::Label::Label)
%std_exceptions(carto::Label::setStyle)

%include "vectorelements/Label.h"

#endif
