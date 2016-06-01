#ifndef _CARTOSWIG_I
#define _CARTOSWIG_I

#ifndef SWIGCSHARP
%define SWIGEXCODE
0
%enddef
#endif

%apply unsigned int { size_t }; 
%apply unsigned int { std::size_t }; 

%include <attribute.i>

%define %staticattribute_custom(Class, AttributeType, AttributeName, GetMethod, SetMethod, GetMethodCall, SetMethodCall)
  %ignore Class::GetMethod();
  %ignore Class::GetMethod() const;
  #if #SetMethod != #AttributeName
    %ignore Class::SetMethod;
  #endif
  %extend Class {
    /**
     * @copydoc GetMethod
     */
    static AttributeType AttributeName;
  }
  %{
    #define %mangle(Class) ##_## AttributeName ## _get() Class :: GetMethodCall
    #define %mangle(Class) ##_## AttributeName ## _set(val_) Class :: SetMethodCall
  %}
%enddef

%define %attribute_readonlystatic(Class, AttributeType, AttributeName, GetMethod, GetMethodCall)
  %ignore Class::GetMethod();
  %ignore Class::GetMethod() const;
  %immutable Class::AttributeName;
  %extend Class {
    /**
     * @copydoc GetMethod
     */
    static AttributeType AttributeName;
  }
  %{
    #define %mangle(Class) ##_## AttributeName ## _get() Class :: GetMethodCall
  %}
%enddef

%define %staticattribute(Class, AttributeType, AttributeName, GetMethod, SetMethod...)
  #if #SetMethod != ""
    %staticattribute_custom(%arg(Class), %arg(AttributeType), AttributeName, GetMethod, SetMethod, GetMethod(), SetMethod(val_))
  #else
    %staticattribute_readonly(%arg(Class), %arg(AttributeType), AttributeName, GetMethod, GetMethod())
  #endif
%enddef

%define %staticattributestring(Class, AttributeType, AttributeName, GetMethod, SetMethod...)
  %{
    #define %mangle(Class) ##_## AttributeName ## _get() *new AttributeType(Class :: GetMethod())
  %}
  #if #SetMethod != ""
    %{
      #define %mangle(Class) ##_## AttributeName ## _set(val_) Class :: SetMethod(val_)
    %}
    #if #SetMethod != #AttributeName
      %ignore Class::SetMethod;
    #endif
  #else
    %immutable Class::AttributeName;
  #endif
  %ignore Class::GetMethod();
  %ignore Class::GetMethod() const;
  %newobject Class::AttributeName;
  %typemap(newfree) const AttributeType &AttributeName "delete $1;"
  %extend Class {
    /**
     * @copydoc GetMethod
     */
    static AttributeType AttributeName;
  }
%enddef

// Change director ownership, native side
// Dotnet/PInvoke does not support this, so ownership handling
// must be done on managed side.
%define %director_change_ownership(obj, owner)

if (auto director = std::dynamic_pointer_cast<Swig::Director>(obj)) {
#if SWIGJAVA
  (director)->swig_java_change_ownership(jenv, (director)->swig_get_self(jenv), (owner));
#endif
#if SWIGCSHARP
  (director)->swig_csharp_change_ownership((director)->swig_get_self(), (owner));
#endif
#if SWIGOBJECTIVEC
  (director)->swig_objc_change_ownership((director)->swig_get_self(), (owner));
#endif
}

%enddef
#endif
