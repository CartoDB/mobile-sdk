#ifndef _CARTOONLINEVECTORTILELAYER_I
#define _CARTOONLINEVECTORTILELAYER_I

%module CartoOnlineVectorTileLayer

!proxy_imports(carto::CartoOnlineVectorTileLayer, datasources.TileDataSource, layers.CartoVectorTileLayer, utils.AssetPackage)

%{
#include "layers/CartoOnlineVectorTileLayer.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "datasources/TileDataSource.i"
%import "layers/CartoVectorTileLayer.i"
%import "utils/AssetPackage.i"

!polymorphic_shared_ptr(carto::CartoOnlineVectorTileLayer, layers.CartoOnlineVectorTileLayer)

%std_exceptions(carto::CartoOnlineVectorTileLayer::CartoOnlineVectorTileLayer(const std::string&, const std::shared_ptr<AssetPackage>&))

%include "layers/CartoOnlineVectorTileLayer.h"

#endif
