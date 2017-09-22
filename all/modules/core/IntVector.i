#ifndef _INTVECTOR_I
#define _INTVECTOR_I

#pragma SWIG nowarn=302

%module IntVector

%include <std_vector.i>

!value_type(std::vector<int>, core.IntVector)

#ifdef SWIGOBJECTIVEC
%template(NTIntVector) std::vector<int>;
#else
%template(IntVector) std::vector<int>;
#endif

#endif
