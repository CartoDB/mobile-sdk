#ifndef _PACKAGESTATUS_I
#define _PACKAGESTATUS_I

%module PackageStatus

#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

%{
#include "packagemanager/PackageStatus.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

!shared_ptr(carto::PackageStatus, packagemanager.PackageStatus)

%attribute(carto::PackageStatus, bool, Paused, isPaused)
%attribute(carto::PackageStatus, carto::PackageAction::PackageAction, CurrentAction, getCurrentAction)
%attribute(carto::PackageStatus, float, Progress, getProgress)
!standard_equals(carto::PackageStatus);

%include "packagemanager/PackageStatus.h"

#endif

#endif
