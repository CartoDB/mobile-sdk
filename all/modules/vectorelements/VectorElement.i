#ifndef _VECTORELEMENT_I
#define _VECTORELEMENT_I

#pragma SWIG nowarn=401

%module VectorElement

!proxy_imports(carto::VectorElement, core.MapBounds, core.Variant, core.StringVariantMap, geometry.Geometry)

%{
#include "vectorelements/VectorElement.h"
#include <memory>
%}

%include <std_string.i>
%include <std_shared_ptr.i>
%include <std_vector.i>
%include <cartoswig.i>

%import "core/MapBounds.i"
%import "core/Variant.i"
%import "geometry/Geometry.i"

!polymorphic_shared_ptr(carto::VectorElement, vectorelements.VectorElement)
!value_type(std::vector<std::shared_ptr<carto::VectorElement> >, vectorelements.VectorElementVector)

%attribute(carto::VectorElement, long long, Id, getId, setId)
%attributeval(carto::VectorElement, %arg(std::map<std::string, carto::Variant>), MetaData, getMetaData, setMetaData)
%attribute(carto::VectorElement, bool, Visible, isVisible, setVisible)
%csmethodmodifiers carto::VectorElement::Bounds "public virtual";
%attributeval(carto::VectorElement, carto::MapBounds, Bounds, getBounds)
%csmethodmodifiers carto::VectorElement::Geometry "public virtual";
!attributestring_polymorphic(carto::VectorElement, geometry.Geometry, Geometry, getGeometry)
!standard_equals(carto::VectorElement);

%include "vectorelements/VectorElement.h"

!value_template(std::vector<std::shared_ptr<carto::VectorElement> >, vectorelements.VectorElementVector);

#endif
