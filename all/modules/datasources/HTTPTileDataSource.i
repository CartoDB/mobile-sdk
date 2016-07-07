#ifndef _HTTPTILEDATASOURCE_I
#define _HTTPTILEDATASOURCE_I

%module(directors="1") HTTPTileDataSource

!proxy_imports(carto::HTTPTileDataSource, core.MapTile, core.StringVector, core.StringMap, datasources.TileDataSource, datasources.components.TileData)

%{
#include "datasources/HTTPTileDataSource.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <std_map.i>
%include <std_vector.i>
%include <cartoswig.i>

%import "datasources/TileDataSource.i"
%import "core/StringVector.i"
%import "core/StringMap.i"

!polymorphic_shared_ptr(carto::HTTPTileDataSource, datasources.HTTPTileDataSource)

%attributestring(carto::HTTPTileDataSource, std::string, BaseURL, getBaseURL, setBaseURL)
%attributeval(carto::HTTPTileDataSource, %arg(std::vector<std::string>), Subdomains, getSubdomains, setSubdomains)
%attribute(carto::HTTPTileDataSource, bool, TMSScheme, isTMSScheme, setTMSScheme)
%attributeval(carto::HTTPTileDataSource, %arg(std::map<std::string, std::string>), HTTPHeaders, getHTTPHeaders, setHTTPHeaders)

%feature("director") carto::HTTPTileDataSource;

%include "datasources/HTTPTileDataSource.h"

#endif
