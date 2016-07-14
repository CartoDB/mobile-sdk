#ifndef _PACKAGEINFO_I
#define _PACKAGEINFO_I

%module PackageInfo

#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

!proxy_imports(carto::PackageInfo, packagemanager.PackageMetaInfo, packagemanager.PackageTileMask, core.StringVector)

%{
#include "packagemanager/PackageInfo.h"
#include <memory>
%}

%include <stdint.i>
%include <std_shared_ptr.i>
%include <std_vector.i>
%include <cartoswig.i>

%import "core/StringVector.i"
%import "packagemanager/PackageMetaInfo.i"
%import "packagemanager/PackageTileMask.i"

using std::uint64_t;

!shared_ptr(carto::PackageInfo, packagemanager.PackageInfo)
!value_type(std::vector<std::shared_ptr<carto::PackageInfo> >, packagemanager.PackageInfoVector)

%attributestring(carto::PackageInfo, std::string, PackageId, getPackageId)
%attribute(carto::PackageInfo, carto::PackageType::PackageType, PackageType, getPackageType)
%attributestring(carto::PackageInfo, std::string, Name, getName)
%attribute(carto::PackageInfo, int, Version, getVersion)
%attribute(carto::PackageInfo, std::uint64_t, Size, getSize)
%attributestring(carto::PackageInfo, std::shared_ptr<carto::PackageMetaInfo>, MetaInfo, getMetaInfo)
%attributestring(carto::PackageInfo, std::shared_ptr<carto::PackageTileMask>, TileMask, getTileMask)
%ignore carto::PackageInfo::getServerURL;
!standard_equals(carto::PackageInfo);

%include "packagemanager/PackageInfo.h"

!value_template(std::vector<std::shared_ptr<carto::PackageInfo> >, packagemanager.PackageInfoVector)

#endif

#endif
