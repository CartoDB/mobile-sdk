#ifndef _LOG_I
#define _LOG_I

%module Log

!proxy_imports(carto::Log, utils.LogEventListener)

%{
#include "utils/Log.h"
%}

%include <std_string.i>
%include <cartoswig.i>

%import "utils/LogEventListener.i"

%staticattribute(carto::Log, bool, ShowError, IsShowError, SetShowError)
%staticattribute(carto::Log, bool, ShowWarn, IsShowWarn, SetShowWarn)
%staticattribute(carto::Log, bool, ShowInfo, IsShowInfo, SetShowInfo)
%staticattribute(carto::Log, bool, ShowDebug, IsShowDebug, SetShowDebug)
%staticattributestring(carto::Log, std::string, Tag, GetTag, SetTag)
%ignore carto::Log::Fatalf;
%ignore carto::Log::Errorf;
%ignore carto::Log::Warnf;
%ignore carto::Log::Infof;
%ignore carto::Log::Debugf;

%include "utils/Log.h"

#endif
