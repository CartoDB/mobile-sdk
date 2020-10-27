#ifndef _DOUBLEVECTOR_I
#define _DOUBLEVECTOR_I

#pragma SWIG nowarn=302

%module DoubleVector

%include <std_vector.i>

!value_type(std::vector<double>, core.DoubleVector)

#ifdef SWIGOBJECTIVEC
%template(NTDoubleVector) std::vector<double>;
#else
%template(DoubleVector) std::vector<double>;
#endif

#endif