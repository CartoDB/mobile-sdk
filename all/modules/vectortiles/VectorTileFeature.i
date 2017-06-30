#ifndef _VECTORTILEFEATURE_I
#define _VECTORTILEFEATURE_I

%module VectorTileFeature

!proxy_imports(carto::VectorTileFeature, core.Variant, geometry.Geometry)

%{
#include "vectortiles/VectorTileFeature.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_vector.i>
%include <std_string.i>
%include <cartoswig.i>

%import "core/Variant.i"
%import "geometry/Geometry.i"

!shared_ptr(carto::VectorTileFeature, vectortile.VectorTileFeature)
!value_type(std::vector<std::shared_ptr<carto::VectorTileFeature> >, vectortiles.VectorTileFeatureVector)

%attribute(carto::VectorTileFeature, long long, Id, getId)
%attributestring(carto::VectorTileFeature, std::string, LayerName, getLayerName)
!attributestring_polymorphic(carto::VectorTileFeature, geometry.Geometry, Geometry, getGeometry)
%attributeval(carto::VectorTileFeature, carto::Variant, Properties, getProperties)
!standard_equals(carto::VectorTileFeature);

%include "vectortiles/VectorTileFeature.h"

!value_template(std::vector<std::shared_ptr<carto::VectorTileFeature> >, vectortiles.VectorTileFeatureVector)

#endif
