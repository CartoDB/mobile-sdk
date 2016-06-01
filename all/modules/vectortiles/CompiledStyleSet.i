#ifndef _COMPILEDSTYLESET_I
#define _COMPILEDSTYLESET_I

%module CompiledStyleSet

!proxy_imports(carto::CompiledStyleSet, vectortiles.AssetPackage)

%{
#include "vectortiles/AssetPackage.h"
#include "vectortiles/CompiledStyleSet.h"
#include <memory>
%}

%include <std_string.i>
%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "vectortiles/AssetPackage.i"

!shared_ptr(carto::CompiledStyleSet, vectortiles.CompiledStyleSet)

%attributestring(carto::CompiledStyleSet, std::string, StyleName, getStyleName)
%attributestring(carto::CompiledStyleSet, std::string, StyleAssetName, getStyleAssetName)
%attributestring(carto::CompiledStyleSet, std::shared_ptr<carto::AssetPackage>, AssetPackage, getAssetPackage)
!standard_equals(carto::CompiledStyleSet);

%include "vectortiles/CompiledStyleSet.h"

#endif
