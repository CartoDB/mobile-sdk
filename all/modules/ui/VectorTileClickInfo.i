#ifndef _VECTORTILECLICKINFO_I
#define _VECTORTILECLICKINFO_I

%module VectorTileClickInfo

!proxy_imports(carto::VectorTileClickInfo, core.MapPos, core.MapTile, geometry.Feature, layers.Layer, ui.ClickType, vectortiles.VectorTileFeature)

%{
#include "ui/VectorTileClickInfo.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "ui/ClickType.i"
%import "core/MapPos.i"
%import "core/MapTile.i"
%import "geometry/Feature.i"
%import "layers/Layer.i"
%import "vectortiles/VectorTileFeature.i"

!shared_ptr(carto::VectorTileClickInfo, ui.VectorTileClickInfo)

%attribute(carto::VectorTileClickInfo, carto::ClickType::ClickType, ClickType, getClickType)
%attributeval(carto::VectorTileClickInfo, carto::MapPos, ClickPos, getClickPos)
%attributeval(carto::VectorTileClickInfo, carto::MapPos, FeatureClickPos, getFeatureClickPos)
%attributeval(carto::VectorTileClickInfo, carto::MapTile, MapTile, getMapTile)
%attribute(carto::VectorTileClickInfo, long long, FeatureId, getFeatureId)
%attributestring(carto::VectorTileClickInfo, std::shared_ptr<carto::VectorTileFeature>, VectorTileFeature, getVectorTileFeature)
%attributestring(carto::VectorTileClickInfo, std::shared_ptr<carto::Feature>, Feature, getFeature)
%attributestring(carto::VectorTileClickInfo, std::string, FeatureLayerName, getFeatureLayerName)
!attributestring_polymorphic(carto::VectorTileClickInfo, layers.Layer, Layer, getLayer)
!standard_equals(carto::VectorTileClickInfo);

%include "ui/VectorTileClickInfo.h"

#endif
