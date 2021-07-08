#ifndef _CARTOVECTORTILELAYER_I
#define _CARTOVECTORTILELAYER_I

%module CartoVectorTileLayer

!proxy_imports(carto::CartoVectorTileLayer, datasources.TileDataSource, layers.VectorTileLayer, utils.AssetPackage, vectortiles.VectorTileDecoder)

%{
#include "layers/CartoVectorTileLayer.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "layers/VectorTileLayer.i"
%import "datasources/TileDataSource.i"
%import "utils/AssetPackage.i"
%import "vectortiles/VectorTileDecoder.i"

!enum(carto::CartoBaseMapStyle::CartoBaseMapStyle)
!enum(carto::CartoBaseMapPOIRenderMode::CartoBaseMapPOIRenderMode)
!enum(carto::CartoBaseMapBuildingRenderMode::CartoBaseMapBuildingRenderMode)
!polymorphic_shared_ptr(carto::CartoVectorTileLayer, layers.CartoVectorTileLayer)

%attributestring(carto::CartoVectorTileLayer, std::string, Language, getLanguage, setLanguage)
%attributestring(carto::CartoVectorTileLayer, std::string, FallbackLanguage, getFallbackLanguage, setFallbackLanguage)
%attribute(carto::CartoVectorTileLayer, carto::CartoBaseMapPOIRenderMode::CartoBaseMapPOIRenderMode, POIRenderMode, getPOIRenderMode, setPOIRenderMode)
%attribute(carto::CartoVectorTileLayer, carto::CartoBaseMapBuildingRenderMode::CartoBaseMapBuildingRenderMode, BuildingRenderMode, getBuildingRenderMode, setBuildingRenderMode)
%std_exceptions(carto::CartoVectorTileLayer::CartoVectorTileLayer(const std::shared_ptr<TileDataSource>&, const std::shared_ptr<AssetPackage>&))
%std_exceptions(carto::CartoVectorTileLayer::CreateTileDecoder(const std::shared_ptr<AssetPackage>&))
!objc_rename(createTileDecoderFromAssetPackage) carto::CartoVectorTileLayer::CreateTileDecoder(const std::shared_ptr<AssetPackage>&);

%ignore carto::CartoVectorTileLayer::CreateStyleAssetPackage;
%ignore carto::CartoVectorTileLayer::GetStyleName;
%ignore carto::CartoVectorTileLayer::GetStyleSource;

!objc_rename(createTileDecoderFromStyle) carto::CartoVectorTileLayer::CreateTileDecoder(CartoBaseMapStyle::CartoBaseMapStyle style);

%include "layers/CartoVectorTileLayer.h"

#endif
