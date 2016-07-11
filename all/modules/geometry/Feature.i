#ifndef _FEATURE_I
#define _FEATURE_I

%module Feature

!proxy_imports(carto::Feature, core.Variant, geometry.Geometry)

%{
#include "geometry/Feature.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_vector.i>
%include <cartoswig.i>

%import "core/variant.i"
%import "geometry/Geometry.i"

!shared_ptr(carto::Feature, geometry.Feature)
!value_type(std::vector<std::shared_ptr<carto::Feature> >, geometry.FeatureVector)

!attributestring_polymorphic(carto::Feature, geometry.Geometry, Geometry, getGeometry)
%attributeval(carto::Feature, carto::Variant, Properties, getProperties)
!standard_equals(carto::Feature);

%include "geometry/Feature.h"

!value_template(std::vector<std::shared_ptr<carto::Feature> >, geometry.FeatureVector)

#endif
