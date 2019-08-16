#ifndef _CARTOOFFLINEVECTORTILELAYER_I
#define _CARTOOFFLINEVECTORTILELAYER_I

%module CartoOfflineVectorTileLayer

#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

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

%std_exceptions(carto::CartoOfflineVectorTileLayer::CartoOfflineVectorTileLayer(const std::shared_ptr<CartoPackageManager>&, const std::shared_ptr<AssetPackage>&))

%include "layers/CartoOfflineVectorTileLayer.h"

#endif

#endif
