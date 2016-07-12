#ifndef _ZIPPEDASSETPACKAGE_I
#define _ZIPPEDASSETPACKAGE_I

%module ZippedAssetPackage

!proxy_imports(carto::ZippedAssetPackage, core.BinaryData, core.StringVector, utils.AssetPackage)

%{
#include "utils/ZippedAssetPackage.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_vector.i>
%include <cartoswig.i>

%import "core/BinaryData.i"
%import "core/StringVector.i"
%import "utils/AssetPackage.i"

!polymorphic_shared_ptr(carto::ZippedAssetPackage, utils.ZippedAssetPackage)

%include "utils/ZippedAssetPackage.h"

#endif
