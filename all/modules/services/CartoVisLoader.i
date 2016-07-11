#ifndef _CARTOVISLOADER_I
#define _CARTOVISLOADER_I

%module CartoVisLoader

!proxy_imports(carto::CartoVisLoader, services.CartoVisBuilder, ui.BaseMapView, utils.AssetPackage)

%{
#include "services/CartoVisLoader.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_string.i>
%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "services/CartoVisBuilder.i"
%import "utils/AssetPackage.i"

!shared_ptr(carto::CartoVisLoader, services.CartoVisLoader)

%attribute(carto::CartoVisLoader, bool, DefaultVectorLayerMode, isDefaultVectorLayerMode, setDefaultVectorLayerMode)
%attributestring(carto::CartoVisLoader, std::shared_ptr<carto::AssetPackage>, VectorTileAssetPackage, getVectorTileAssetPackage, setVectorTileAssetPackage)
%std_io_exceptions(carto::CartoVisLoader::loadVis)

!standard_equals(carto::CartoVisLoader);

%include "services/CartoVisLoader.h"

#endif
