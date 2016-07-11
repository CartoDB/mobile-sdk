#ifndef _PACKAGEMANAGER_I
#define _PACKAGEMANAGER_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module PackageManager

#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

!proxy_imports(carto::PackageManager, packagemanager.PackageInfo, packagemanager.PackageMetaInfo, packagemanager.PackageStatus, packagemanager.PackageManagerListener, packagemanager.PackageInfoVector)

%{
#include "packagemanager/PackageManager.h"
#include "packagemanager/PackageInfo.h"
#include "packagemanager/PackageMetaInfo.h"
#include "packagemanager/PackageStatus.h"
#include "packagemanager/PackageManagerListener.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <stdint.i>
%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "packagemanager/PackageInfo.i"
%import "packagemanager/PackageMetaInfo.i"
%import "packagemanager/PackageStatus.i"
%import "packagemanager/PackageManagerListener.i"

using std::uint64_t;

!polymorphic_shared_ptr(carto::PackageManager, packagemanager.PackageManager)

%attributeval(carto::PackageManager, std::vector<std::shared_ptr<carto::PackageInfo> >, ServerPackages, getServerPackages)
%attributeval(carto::PackageManager, std::vector<std::shared_ptr<carto::PackageInfo> >, LocalPackages, getLocalPackages)
%attribute(carto::PackageManager, int, ServerPackageListAge, getServerPackageListAge)
%attributestring(carto::PackageManager, std::shared_ptr<carto::PackageMetaInfo>, ServerPackageListMetaInfo, getServerPackageListMetaInfo)
!attributestring_polymorphic(carto::PackageManager, packagemanager.PackageManagerListener, PackageManagerListener, getPackageManagerListener, setPackageManagerListener)
%std_io_exceptions(carto::PackageManager::PackageManager)
%ignore carto::PackageManager::PackageManager(const std::string&, const std::string&, const std::string&, const std::string&, const std::shared_ptr<Logger>&);
%ignore carto::PackageManager::Logger;
%ignore carto::PackageManager::OnChangeListener;
%ignore carto::PackageManager::registerOnChangeListener;
%ignore carto::PackageManager::unregisterOnChangeListener;
%ignore carto::PackageManager::loadTile;
%ignore carto::PackageManager::accessPackageFiles;
!standard_equals(carto::PackageManager);

%include "packagemanager/PackageManager.h"

#endif

#endif
