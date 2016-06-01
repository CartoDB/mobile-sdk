#ifndef _POLYGON3D_I
#define _POLYGON3D_I

%module Polygon3D

!proxy_imports(carto::Polygon3D, core.MapPosVector, core.MapPosVectorVector, geometry.PolygonGeometry, styles.Polygon3DStyle, vectorelements.VectorElement)

%{
#include "vectorelements/Polygon3D.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/MapPos.i"
%import "geometry/PolygonGeometry.i"
%import "styles/Polygon3DStyle.i"
%import "vectorelements/VectorElement.i"

!polymorphic_shared_ptr(carto::Polygon3D, vectorelements.Polygon3D)

%attribute(carto::Polygon3D, float, Height, getHeight, setHeight)
%attributestring(carto::Polygon3D, std::shared_ptr<carto::Polygon3DStyle>, Style, getStyle, setStyle)
%csmethodmodifiers carto::Polygon3D::Geometry "public new";
!attributestring_polymorphic(carto::Polygon3D, geometry.PolygonGeometry, Geometry, getGeometry, setGeometry)
%ignore carto::Polygon3D::getDrawData;
%ignore carto::Polygon3D::setDrawData;

%include "vectorelements/Polygon3D.h"

#endif
