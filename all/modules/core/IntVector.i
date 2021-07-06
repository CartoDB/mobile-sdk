#ifndef _INTVECTOR_I
#define _INTVECTOR_I

#pragma SWIG nowarn=302

%module IntVector

%include <std_vector.i>

!value_type(std::vector<int>, core.IntVector)

!value_template(std::vector<int>, core.IntVector)

#endif
