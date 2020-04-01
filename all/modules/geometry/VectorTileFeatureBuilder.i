#ifndef _VECTORTILEFEATUREBUILDER_I
#define _VECTORTILEFEATUREBUILDER_I

%module VectorTileFeatureBuilder

!proxy_imports(carto::VectorTileFeatureBuilder, core.MapTile, core.Variant, geometry.Feature, geometry.FeatureBuilder, geometry.VectorTileFeature)

%{
#include "geometry/VectorTileFeatureBuilder.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "core/MapTile.i"
%import "geometry/FeatureBuilder.i"
%import "geometry/VectorTileFeature.i"

!polymorphic_shared_ptr(carto::VectorTileFeatureBuilder, geometry.VectorTileFeatureBuilder)

%attribute(carto::VectorTileFeatureBuilder, long long, Id, getId, setId)
%attributeval(carto::VectorTileFeatureBuilder, carto::MapTile, MapTile, getMapTile, setMapTile)
%attributestring(carto::VectorTileFeatureBuilder, std::string, LayerName, getLayerName, setLayerName)
!standard_equals(carto::VectorTileFeatureBuilder);

%include "geometry/VectorTileFeatureBuilder.h"

#endif
