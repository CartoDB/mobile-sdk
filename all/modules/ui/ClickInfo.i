#ifndef _CLICKINFO_I
#define _CLICKINFO_I

%module ClickInfo

%{
#include "ui/ClickInfo.h"
%}

%include <std_string.i>
%include <cartoswig.i>

%import "ui/ClickInfo.i"

!enum(carto::ClickType::ClickType)
!value_type(carto::ClickInfo, ui.ClickInfo)

%attribute(carto::ClickInfo, carto::ClickType::ClickType, ClickType, getClickType)
%attribute(carto::ClickInfo, float, Duration, getDuration)
!custom_equals(carto::ClickInfo);
!custom_tostring(carto::ClickInfo);

%include "ui/ClickInfo.h"

#endif
