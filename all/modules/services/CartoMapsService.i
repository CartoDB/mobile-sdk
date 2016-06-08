#ifndef _CARTOMAPSSERVICE_I
#define _CARTOMAPSSERVICE_I

%module CartoMapsService

!proxy_imports(carto::CartoMapsService, core.Variant, core.StringVariantMap, core.StringMap, core.StringVector, layers.LayerVector)

%{
#include "services/CartoMapsService.h"
%}

%include <std_string.i>
%include <std_shared_ptr.i>
%include <std_map.i>
%include <cartoswig.i>

%import "core/Variant.i"
%import "core/StringVector.i"
%import "core/StringMap.i"
%import "layers/Layer.i"

!shared_ptr(carto::CartoMapsService, services.CartoMapsService)

%attributestring(carto::CartoMapsService, std::string, Username, getUsername, setUsername)
%attributestring(carto::CartoMapsService, std::string, APIKey, getAPIKey, setAPIKey)
%attributestring(carto::CartoMapsService, std::string, APITemplate, getAPITemplate, setAPITemplate)
%attributeval(carto::CartoMapsService, %arg(std::vector<std::string>), AuthTokens, getAuthTokens, setAuthTokens)
%attribute(carto::CartoMapsService, bool, DefaultVectorLayerMode, isDefaultVectorLayerMode, setDefaultVectorLayerMode)
%ignore carto::CartoMapsService::getTilerURL;
%ignore carto::CartoMapsService::setTilerURL;
%ignore carto::CartoMapsService::getStatTag;
%ignore carto::CartoMapsService::setStatTag;
%ignore carto::CartoMapsService::getLayerFilter;
%ignore carto::CartoMapsService::setLayerFilter;
%ignore carto::CartoMapsService::getLayerIndices;
%ignore carto::CartoMapsService::setLayerIndices;
!standard_equals(carto::CartoMapsService);

%include "services/CartoMapsService.h"

#endif
