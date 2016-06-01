#ifndef _OGRVECTORDATABASE_I
#define _OGRVECTORDATABASE_I

%module OGRVectorDataBase

#ifdef _CARTO_GDAL_SUPPORT

!proxy_imports(carto::OGRVectorDataBase, datasources.OGRGeometryType, core.StringVector)

%{
#include "datasources/OGRVectorDataBase.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "core/StringVector.i"

!polymorphic_shared_ptr(carto::OGRVectorDataBase, datasources.OGRVectorDataBase)

%attribute(carto::OGRVectorDataBase, int, LayerCount, getLayerCount)
%attributeval(carto::OGRVectorDataBase, std::vector<std::string>, LayerNames, getLayerNames)

%include "datasources/OGRVectorDataBase.h"

#endif

#endif
