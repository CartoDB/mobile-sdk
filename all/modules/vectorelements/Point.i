#ifndef _POINT_I
#define _POINT_I

%module Point

!proxy_imports(carto::Point, core.MapPos, geometry.PointGeometry, styles.PointStyle, vectorelements.VectorElement)

%{
#include "vectorelements/Point.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/MapPos.i"
%import "geometry/PointGeometry.i"
%import "styles/PointStyle.i"
%import "vectorelements/VectorElement.i"

!polymorphic_shared_ptr(carto::Point, vectorelements.Point)

%csmethodmodifiers carto::Point::Geometry "public new";
!attributestring_polymorphic(carto::Point, geometry.PointGeometry, Geometry, getGeometry, setGeometry)
%attributestring(carto::Point, std::shared_ptr<carto::PointStyle>, Style, getStyle, setStyle)
%std_exceptions(carto::Point::Point)
%std_exceptions(carto::Point::setGeometry)
%std_exceptions(carto::Point::setStyle)
%ignore carto::Point::getDrawData;
%ignore carto::Point::setDrawData;

%include "vectorelements/Point.h"

#endif
