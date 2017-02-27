#include "AnimationStyle.h"

namespace carto {

    AnimationStyle::~AnimationStyle() {
    }
    
    float AnimationStyle::getRelativeSpeed() const {
        return _relativeSpeed;
    }

    AnimationType::AnimationType AnimationStyle::getFadeAnimationType() const {
        return _fadeAnimationType;
    }
    
    AnimationType::AnimationType AnimationStyle::getSizeAnimationType() const {
        return _sizeAnimationType;
    }

    AnimationStyle::AnimationStyle(float relativeSpeed, AnimationType::AnimationType fadeAnimationType, AnimationType::AnimationType sizeAnimationType) :
        _relativeSpeed(relativeSpeed), _fadeAnimationType(fadeAnimationType), _sizeAnimationType(sizeAnimationType)
    {
    }
    
}
