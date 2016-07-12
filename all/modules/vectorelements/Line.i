#ifndef _LINE_I
#define _LINE_I

%module Line

!proxy_imports(carto::Line, core.MapPosVector, geometry.LineGeometry, styles.LineStyle, vectorelements.VectorElement)

%{
#include "vectorelements/Line.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "geometry/LineGeometry.i"
%import "styles/LineStyle.i"
%import "vectorelements/VectorElement.i"

!polymorphic_shared_ptr(carto::Line, vectorelements.Line)

%csmethodmodifiers carto::Line::Geometry "public new";
!attributestring_polymorphic(carto::Line, geometry.LineGeometry, Geometry, getGeometry, setGeometry)
%attributestring(carto::Line, std::shared_ptr<carto::LineStyle>, Style, getStyle, setStyle)
%std_exceptions(carto::Line::Line)
%std_exceptions(carto::Line::setGeometry)
%std_exceptions(carto::Line::setStyle)
%ignore carto::Line::getDrawData;
%ignore carto::Line::setDrawData;

%include "vectorelements/Line.h"

#endif
