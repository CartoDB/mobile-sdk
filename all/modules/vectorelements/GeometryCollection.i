#ifndef _GEOMETRYCOLLECTION_I
#define _GEOMETRYCOLLECTION_I

%module GeometryCollection

!proxy_imports(carto::GeometryCollection, geometry.MultiGeometry, styles.GeometryCollectionStyle, vectorelements.VectorElement)

%{
#include "vectorelements/GeometryCollection.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "geometry/MultiGeometry.i"
%import "styles/GeometryCollectionStyle.i"
%import "vectorelements/VectorElement.i"

!polymorphic_shared_ptr(carto::GeometryCollection, vectorelements.GeometryCollection)

%csmethodmodifiers carto::GeometryCollection::Geometry "public new";
!attributestring_polymorphic(carto::GeometryCollection, geometry.MultiGeometry, Geometry, getGeometry, setGeometry)
%attributestring(carto::GeometryCollection, std::shared_ptr<carto::GeometryCollectionStyle>, Style, getStyle, setStyle)
%ignore carto::GeometryCollection::getDrawData;
%ignore carto::GeometryCollection::setDrawData;

%include "vectorelements/GeometryCollection.h"

#endif
