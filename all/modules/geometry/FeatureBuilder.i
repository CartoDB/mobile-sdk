#ifndef _FEATUREBUILDER_I
#define _FEATUREBUILDER_I

%module FeatureBuilder

!proxy_imports(carto::FeatureBuilder, core.Variant, geometry.Geometry, geometry.Feature)

%{
#include "geometry/FeatureBuilder.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/Variant.i"
%import "geometry/Geometry.i"
%import "geometry/Feature.i"

!polymorphic_shared_ptr(carto::FeatureBuilder, geometry.FeatureBuilder)

!attributestring_polymorphic(carto::FeatureBuilder, geometry.Geometry, Geometry, getGeometry, setGeometry)
!standard_equals(carto::FeatureBuilder);

%include "geometry/FeatureBuilder.h"

#endif
