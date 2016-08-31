#ifndef _CARTOVECTORTILELAYER_I
#define _CARTOVECTORTILELAYER_I

%module CartoVectorTileLayer

!proxy_imports(carto::CartoVectorTileLayer, datasources.TileDataSource, layers.VectorTileLayer, utils.AssetPackage)

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

!polymorphic_shared_ptr(carto::CartoVectorTileLayer, layers.CartoVectorTileLayer)

%attributestring(carto::CartoVectorTileLayer, std::string, Language, getLanguage, setLanguage)
#ifdef _CARTO_CUSTOM_BASEMAP_SUPPORT
%std_exceptions(carto::CartoVectorTileLayer::CartoVectorTileLayer(const std::shared_ptr<carto::TileDataSource>&, const std::shared_ptr<carto::AssetPackage>&))
#else
%ignore carto::CartoVectorTileLayer::CartoVectorTileLayer(const std::shared_ptr<carto::TileDataSource>&, const std::shared_ptr<carto::AssetPackage>&);
#endif

%include "layers/CartoVectorTileLayer.h"

#endif
