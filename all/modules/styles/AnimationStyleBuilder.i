#ifndef _ANIMATIONSTYLEBUILDER_I
#define _ANIMATIONSTYLEBUILDER_I

%module AnimationStyleBuilder

!proxy_imports(carto::AnimationStyleBuilder, styles.AnimationStyle)

%{
#include "styles/AnimationStyleBuilder.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

%import "styles/AnimationStyle.i"

!polymorphic_shared_ptr(carto::AnimationStyleBuilder, styles.AnimationStyleBuilder)

%attribute(carto::AnimationStyleBuilder, float, RelativeSpeed, getRelativeSpeed, setRelativeSpeed)
%attribute(carto::AnimationStyleBuilder, carto::AnimationType::AnimationType, FadeAnimationType, getFadeAnimationType, setFadeAnimationType)
%attribute(carto::AnimationStyleBuilder, carto::AnimationType::AnimationType, SizeAnimationType, getSizeAnimationType, setSizeAnimationType)

%include "styles/AnimationStyleBuilder.h"

#endif
