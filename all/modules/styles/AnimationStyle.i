#ifndef _ANIMATIONSTYLE_I
#define _ANIMATIONSTYLE_I

%module AnimationStyle

!proxy_imports(carto::AnimationStyle)

%{
#include "styles/AnimationStyle.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <std_string.i>
%include <cartoswig.i>

!enum(carto::AnimationType::AnimationType)
!polymorphic_shared_ptr(carto::AnimationStyle, styles.AnimationStyle)

%attribute(carto::AnimationStyle, float, RelativeSpeed, getRelativeSpeed)
%attribute(carto::AnimationStyle, float, PhaseInDuration, getPhaseInDuration)
%attribute(carto::AnimationStyle, float, PhaseOutDuration, getPhaseOutDuration)
%attribute(carto::AnimationStyle, carto::AnimationType::AnimationType, FadeAnimationType, getFadeAnimationType)
%attribute(carto::AnimationStyle, carto::AnimationType::AnimationType, SizeAnimationType, getSizeAnimationType)
%ignore carto::AnimationStyle::AnimationStyle;
%ignore carto::AnimationStyle::CalculateTransition;
!standard_equals(carto::AnimationStyle);

%include "styles/AnimationStyle.h"

#endif
