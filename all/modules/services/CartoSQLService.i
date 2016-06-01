#ifndef _CARTOSQLSERVICE_I
#define _CARTOSQLSERVICE_I

%module CartoSQLService

!proxy_imports(carto::CartoSQLService, core.Variant)

%{
#include "services/CartoSQLService.h"
%}

%include <std_string.i>
%include <std_shared_ptr.i>
%include <std_map.i>
%include <cartoswig.i>

%import "core/Variant.i"

!shared_ptr(carto::CartoSQLService, services.CartoSQLService)

%attributestring(carto::CartoSQLService, std::string, Username, getUsername, setUsername)
%attributestring(carto::CartoSQLService, std::string, APIKey, getAPIKey, setAPIKey)
%attributestring(carto::CartoSQLService, std::string, APITemplate, getAPITemplate, setAPITemplate)
!standard_equals(carto::CartoSQLService);

%include "services/CartoSQLService.h"

#endif
