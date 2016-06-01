#ifndef _GEOMETRYCOLLECTIONSTYLEBUILDER_I
#define _GEOMETRYCOLLECTIONSTYLEBUILDER_I

%module GeometryCollectionStyleBuilder

!proxy_imports(carto::GeometryCollectionStyleBuilder, styles.GeometryCollectionStyle, styles.PointStyle, styles.LineStyle, styles.PolygonStyle, styles.StyleBuilder)

%{
#include "styles/GeometryCollectionStyleBuilder.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "styles/GeometryCollectionStyle.i"
%import "styles/PointStyle.i"
%import "styles/LineStyle.i"
%import "styles/PolygonStyle.i"
%import "styles/StyleBuilder.i"

!polymorphic_shared_ptr(carto::GeometryCollectionStyleBuilder, styles.GeometryCollectionStyleBuilder)

%attributestring(carto::GeometryCollectionStyleBuilder, std::shared_ptr<carto::PointStyle>, PointStyle, getPointStyle, setPointStyle)
%attributestring(carto::GeometryCollectionStyleBuilder, std::shared_ptr<carto::LineStyle>, LineStyle, getLineStyle, setLineStyle)
%attributestring(carto::GeometryCollectionStyleBuilder, std::shared_ptr<carto::PolygonStyle>, PolygonStyle, getPolygonStyle, setPolygonStyle)

%include "styles/GeometryCollectionStyleBuilder.h"

#endif
