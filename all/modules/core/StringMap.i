#ifndef _STRINGMAP_I
#define _STRINGMAP_I

#pragma SWIG nowarn=302

%module StringMap

%include <std_map.i>
%include <std_string.i>

!value_template(std::map<std::string, std::string>, core.StringMap)

#endif
