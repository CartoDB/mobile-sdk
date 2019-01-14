#ifndef _ZIPPEDASSETPACKAGE_I
#define _ZIPPEDASSETPACKAGE_I

%module(directors="1") ZippedAssetPackage

!proxy_imports(carto::ZippedAssetPackage, core.BinaryData, core.StringVector, utils.AssetPackage)

%{
#include "utils/ZippedAssetPackage.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_vector.i>
%include <cartoswig.i>

%import "core/BinaryData.i"
%import "core/StringVector.i"
%import "utils/AssetPackage.i"

!polymorphic_shared_ptr(carto::ZippedAssetPackage, utils.ZippedAssetPackage)

%attributeval(carto::ZippedAssetPackage, %arg(std::vector<std::string>), LocalAssetNames, getLocalAssetNames)
%std_exceptions(carto::ZippedAssetPackage::ZippedAssetPackage)

%include "utils/ZippedAssetPackage.h"

#endif
