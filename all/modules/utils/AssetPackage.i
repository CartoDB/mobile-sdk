#ifndef _ASSETPACKAGE_I
#define _ASSETPACKAGE_I

%module AssetPackage

!proxy_imports(carto::AssetPackage, core.BinaryData, core.StringVector)

%{
#include "utils/AssetPackage.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_vector.i>
%include <cartoswig.i>

%import "core/BinaryData.i"
%import "core/StringVector.i"

!polymorphic_shared_ptr(carto::AssetPackage, utils.AssetPackage)

%attributeval(carto::AssetPackage, %arg(std::vector<std::string>), AssetNames, getAssetNames)
!standard_equals(carto::AssetPackage);

%include "utils/AssetPackage.h"

#endif
