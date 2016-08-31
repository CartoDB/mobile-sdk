#ifndef _CARTOOFFLINEVECTORTILELAYER_I
#define _CARTOOFFLINEVECTORTILELAYER_I

%module CartoOfflineVectorTileLayer

!proxy_imports(carto::CartoOfflineVectorTileLayer, layers.CartoVectorTileLayer, packagemanager.CartoPackageManager, utils.AssetPackage)

%{
#include "layers/CartoOfflineVectorTileLayer.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "layers/CartoVectorTileLayer.i"
%import "packagemanager/CartoPackageManager.i"
%import "utils/AssetPackage.i"

!polymorphic_shared_ptr(carto::CartoOfflineVectorTileLayer, layers.CartoOfflineVectorTileLayer)

#ifdef _CARTO_CUSTOM_BASEMAP_SUPPORT
%std_exceptions(carto::CartoOfflineVectorTileLayer::CartoOfflineVectorTileLayer(const std::shared_ptr<carto::CartoPackageManager>&, const std::shared_ptr<carto::AssetPackage>&))
#else
%ignore carto::CartoOfflineVectorTileLayer::CartoOfflineVectorTileLayer(const std::shared_ptr<carto::CartoPackageManager>&, const std::shared_ptr<carto::AssetPackage>&);
#endif

%include "layers/CartoOfflineVectorTileLayer.h"

#endif
