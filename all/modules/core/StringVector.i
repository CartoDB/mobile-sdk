#ifndef _STRINGVECTOR_I
#define _STRINGVECTOR_I

#pragma SWIG nowarn=302

%module StringVector

%include <std_vector.i>
%include <std_string.i>

!value_type(std::vector<std::string>, core.StringVector)

!value_template(std::vector<std::string>, core.StringVector)

#endif
