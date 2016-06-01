#ifndef _CARTOMAPSSERVICE_I
#define _CARTOMAPSSERVICE_I

%module CartoMapsService

!proxy_imports(carto::CartoMapsService, core.Variant, core.StringVariantMap, core.StringMap, layers.Layers, vectortiles.VectorTileDecoder, vectortiles.TorqueTileDecoder)

%{
#include "services/CartoMapsService.h"
%}

%include <std_string.i>
%include <std_shared_ptr.i>
%include <std_map.i>
%include <cartoswig.i>

%import "core/Variant.i"
%import "core/StringMap.i"
%import "layers/Layers.i"
%import "vectortiles/VectorTileDecoder.i"
%import "vectortiles/TorqueTileDecoder.i"

!shared_ptr(carto::CartoMapsService, services.CartoMapsService)

%attributestring(carto::CartoMapsService, std::string, Username, getUsername, setUsername)
%attributestring(carto::CartoMapsService, std::string, APIKey, getAPIKey, setAPIKey)
%attributestring(carto::CartoMapsService, std::string, APITemplate, getAPITemplate, setAPITemplate)
%attributestring(carto::CartoMapsService, std::string, AuthToken, getAuthToken, setAuthToken)
%attribute(carto::CartoMapsService, bool, DefaultVectorLayerMode, isDefaultVectorLayerMode, setDefaultVectorLayerMode)
%attributestring(carto::CartoMapsService, std::shared_ptr<carto::VectorTileDecoder>, VectorTileDecoder, getVectorTileDecoder, setVectorTileDecoder)
%attributestring(carto::CartoMapsService, std::shared_ptr<carto::TorqueTileDecoder>, TorqueTileDecoder, getTorqueTileDecoder, setTorqueTileDecoder)
!standard_equals(carto::CartoMapsService);

%include "services/CartoMapsService.h"

#endif
