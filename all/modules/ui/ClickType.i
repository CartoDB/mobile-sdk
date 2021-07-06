#ifndef _CLICKTYPE_I
#define _CLICKTYPE_I

%module ClickType

%{
#include "ui/ClickType.h"
%}

%include <cartoswig.i>

!enum(carto::ClickType::ClickType)

%include "ui/ClickType.h"

#endif
