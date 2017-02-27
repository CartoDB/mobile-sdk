#include "AnimationStyleBuilder.h"
#include "styles/AnimationStyle.h"

namespace carto {

    AnimationStyleBuilder::AnimationStyleBuilder() :
        _relativeSpeed(1.0f),
        _fadeAnimationType(AnimationType::ANIMATION_TYPE_NONE),
        _sizeAnimationType(AnimationType::ANIMATION_TYPE_NONE),
        _mutex()
    {
    }
    
    AnimationStyleBuilder::~AnimationStyleBuilder() {
    }
    
    float AnimationStyleBuilder::getRelativeSpeed() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _relativeSpeed;
    }
    
    void AnimationStyleBuilder::setRelativeSpeed(float relativeSpeed) {
        std::lock_guard<std::mutex> lock(_mutex);
        _relativeSpeed = relativeSpeed;
    }

    AnimationType::AnimationType AnimationStyleBuilder::getFadeAnimationType() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _fadeAnimationType;
    }
    
    void AnimationStyleBuilder::setFadeAnimationType(AnimationType::AnimationType animType) {
        std::lock_guard<std::mutex> lock(_mutex);
        _fadeAnimationType = animType;
    }

    AnimationType::AnimationType AnimationStyleBuilder::getSizeAnimationType() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _sizeAnimationType;
    }
    
    void AnimationStyleBuilder::setSizeAnimationType(AnimationType::AnimationType animType) {
        std::lock_guard<std::mutex> lock(_mutex);
        _sizeAnimationType = animType;
    }

    std::shared_ptr<AnimationStyle> AnimationStyleBuilder::buildStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return std::shared_ptr<AnimationStyle>(new AnimationStyle(_relativeSpeed, _fadeAnimationType, _sizeAnimationType));
    }
        
}
