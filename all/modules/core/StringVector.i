#ifndef _STRINGVECTOR_I
#define _STRINGVECTOR_I

#pragma SWIG nowarn=302

%module StringVector

%include <std_vector.i>
%include <std_string.i>

!value_type(std::vector<std::string>, core.StringVector)

#ifdef SWIGOBJECTIVEC
%template(NTStringVector) std::vector<std::string>;
#else
%template(StringVector) std::vector<std::string>;
#endif

#endif
