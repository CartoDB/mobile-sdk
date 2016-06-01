#ifndef _GEOMETRYCOLLECTIONSTYLE_I
#define _GEOMETRYCOLLECTIONSTYLE_I

%module GeometryCollectionStyle

!proxy_imports(carto::GeometryCollectionStyle, styles.Style, styles.PointStyle, styles.LineStyle, styles.PolygonStyle, graphics.Color)

%{
#include "styles/GeometryCollectionStyle.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "styles/Style.i"
%import "styles/PointStyle.i"
%import "styles/LineStyle.i"
%import "styles/PolygonStyle.i"

!polymorphic_shared_ptr(carto::GeometryCollectionStyle, styles.GeometryCollectionStyle)

%attributestring(carto::GeometryCollectionStyle, std::shared_ptr<carto::PointStyle>, PointStyle, getPointStyle)
%attributestring(carto::GeometryCollectionStyle, std::shared_ptr<carto::LineStyle>, LineStyle, getLineStyle)
%attributestring(carto::GeometryCollectionStyle, std::shared_ptr<carto::PolygonStyle>, PolygonStyle, getPolygonStyle)

%include "styles/GeometryCollectionStyle.h"

#endif
