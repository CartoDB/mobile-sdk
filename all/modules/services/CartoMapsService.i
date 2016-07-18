#ifndef _CARTOMAPSSERVICE_I
#define _CARTOMAPSSERVICE_I

%module CartoMapsService

!proxy_imports(carto::CartoMapsService, core.Variant, core.StringVariantMap, core.StringMap, core.StringVector, layers.Layer, layers.LayerVector, utils.AssetPackage)

%{
#include "services/CartoMapsService.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_string.i>
%include <std_shared_ptr.i>
%include <std_map.i>
%include <cartoswig.i>

%import "core/Variant.i"
%import "core/StringVector.i"
%import "core/StringMap.i"
%import "layers/Layer.i"
%import "utils/AssetPackage.i"

!shared_ptr(carto::CartoMapsService, services.CartoMapsService)

%attributestring(carto::CartoMapsService, std::string, Username, getUsername, setUsername)
%attributestring(carto::CartoMapsService, std::string, APITemplate, getAPITemplate, setAPITemplate)
%attributeval(carto::CartoMapsService, %arg(std::vector<std::string>), AuthTokens, getAuthTokens, setAuthTokens)
%attribute(carto::CartoMapsService, bool, Interactive, isInteractive, setInteractive)
%attribute(carto::CartoMapsService, bool, DefaultVectorLayerMode, isDefaultVectorLayerMode, setDefaultVectorLayerMode)
%attribute(carto::CartoMapsService, bool, StrictMode, isStrictMode, setStrictMode)
%attributestring(carto::CartoMapsService, std::shared_ptr<carto::AssetPackage>, VectorTileAssetPackage, getVectorTileAssetPackage, setVectorTileAssetPackage)
%std_io_exceptions(carto::CartoMapsService::buildMap)
%std_io_exceptions(carto::CartoMapsService::buildNamedMap)
%ignore carto::CartoMapsService::getTilerURL;
%ignore carto::CartoMapsService::setTilerURL;
%ignore carto::CartoMapsService::getStatTag;
%ignore carto::CartoMapsService::setStatTag;
%ignore carto::CartoMapsService::getLayerFilter;
%ignore carto::CartoMapsService::setLayerFilter;
%ignore carto::CartoMapsService::getLayerIndices;
%ignore carto::CartoMapsService::setLayerIndices;
%ignore carto::CartoMapsService::getCDNURLs;
%ignore carto::CartoMapsService::setCDNURLs;
!standard_equals(carto::CartoMapsService);

%include "services/CartoMapsService.h"

#endif
