#ifndef _POLYGON_I
#define _POLYGON_I

%module Polygon

!proxy_imports(carto::Polygon, core.MapPosVector, core.MapPosVectorVector, geometry.PolygonGeometry, styles.PolygonStyle, vectorelements.VectorElement)

%{
#include "vectorelements/Polygon.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/MapPos.i"
%import "geometry/PolygonGeometry.i"
%import "styles/PolygonStyle.i"
%import "vectorelements/VectorElement.i"

!polymorphic_shared_ptr(carto::Polygon, vectorelements.Polygon)

%attributestring(carto::Polygon, std::shared_ptr<carto::PolygonStyle>, Style, getStyle, setStyle)
%csmethodmodifiers carto::Polygon::Geometry "public new";
!attributestring_polymorphic(carto::Polygon, geometry.PolygonGeometry, Geometry, getGeometry, setGeometry)
%std_exceptions(carto::Polygon::Polygon)
%std_exceptions(carto::Polygon::setGeometry)
%std_exceptions(carto::Polygon::setStyle)
%ignore carto::Polygon::getDrawData;
%ignore carto::Polygon::setDrawData;

%include "vectorelements/Polygon.h"

#endif
