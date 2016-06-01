#ifndef _ASSETPACKAGE_I
#define _ASSETPACKAGE_I

%module AssetPackage

!proxy_imports(carto::AssetPackage, core.BinaryData, core.StringVector)

%{
#include "vectortiles/AssetPackage.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_vector.i>
%include <cartoswig.i>

%import "core/BinaryData.i"
%import "core/StringVector.i"

!polymorphic_shared_ptr(carto::AssetPackage, vectortiles.AssetPackage)

%attributeval(carto::AssetPackage, %arg(std::vector<std::string>), AssetNames, getAssetNames)
!standard_equals(carto::AssetPackage);

%include "vectortiles/AssetPackage.h"

#endif
