#ifndef _COMPILEDSTYLESET_I
#define _COMPILEDSTYLESET_I

%module CompiledStyleSet

!proxy_imports(carto::CompiledStyleSet, utils.AssetPackage)

%{
#include "styles/CompiledStyleSet.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_string.i>
%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "utils/AssetPackage.i"

!shared_ptr(carto::CompiledStyleSet, styles.CompiledStyleSet)

%attributestring(carto::CompiledStyleSet, std::string, StyleName, getStyleName)
%attributestring(carto::CompiledStyleSet, std::string, StyleAssetName, getStyleAssetName)
%attributestring(carto::CompiledStyleSet, std::shared_ptr<carto::AssetPackage>, AssetPackage, getAssetPackage)
%std_exceptions(carto::CompiledStyleSet::CompiledStyleSet)
!standard_equals(carto::CompiledStyleSet);

%include "styles/CompiledStyleSet.h"

#endif
