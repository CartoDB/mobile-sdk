#ifndef _PACKAGEMETAINFO_I
#define _PACKAGEMETAINFO_I

%module PackageMetaInfo

#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

!proxy_imports(carto::PackageMetaInfo, core.Variant)

%{
#include "packagemanager/PackageMetaInfo.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "core/Variant.i"

!shared_ptr(carto::PackageMetaInfo, packagemanager.PackageMetaInfo)

%attributeval(carto::PackageMetaInfo, carto::Variant, Variant, getVariant)
!standard_equals(carto::PackageMetaInfo);

%include "packagemanager/PackageMetaInfo.h"

#endif

#endif
