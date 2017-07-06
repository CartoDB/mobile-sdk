#ifndef _VECTORTILEFEATURE_I
#define _VECTORTILEFEATURE_I

%module VectorTileFeature

!proxy_imports(carto::VectorTileFeature, core.MapTile, geometry.Feature)

%{
#include "geometry/VectorTileFeature.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_vector.i>
%include <std_string.i>
%include <cartoswig.i>

%import "core/MapTile.i"
%import "geometry/Feature.i"

!polymorphic_shared_ptr(carto::VectorTileFeature, geometry.VectorTileFeature)
!value_type(std::vector<std::shared_ptr<carto::VectorTileFeature> >, geometry.VectorTileFeatureVector)

%attribute(carto::VectorTileFeature, long long, Id, getId)
%attributeval(carto::VectorTileFeature, carto::MapTile, MapTile, getMapTile)
%attributestring(carto::VectorTileFeature, std::string, LayerName, getLayerName)
!standard_equals(carto::VectorTileFeature);

%include "geometry/VectorTileFeature.h"

!value_template(std::vector<std::shared_ptr<carto::VectorTileFeature> >, geometry.VectorTileFeatureVector)

#endif
