#ifndef _ASSETUTILS_I
#define _ASSETUTILS_I

%module AssetUtils

!proxy_imports(carto::AssetUtils, core.BinaryData)

%{
#include "utils/AssetUtils.h"
%}

%include <std_string.i>

%import "core/BinaryData.i"

%include "utils/AssetUtils.h"

#endif
