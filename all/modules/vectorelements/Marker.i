#ifndef _MARKER_I
#define _MARKER_I

%module Marker

!proxy_imports(carto::Marker, core.MapPos, graphics.Bitmap, geometry.Geometry, styles.MarkerStyle, vectorelements.Billboard)

%{
#include "vectorelements/Marker.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "styles/MarkerStyle.i"
%import "vectorelements/Billboard.i"

!polymorphic_shared_ptr(carto::Marker, vectorelements.Marker)

%attributestring(carto::Marker, std::shared_ptr<carto::MarkerStyle>, Style, getStyle, setStyle)
%std_exceptions(carto::Marker::Marker)
%std_exceptions(carto::Marker::setStyle)

%include "vectorelements/Marker.h"

#endif
