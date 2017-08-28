/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ANIMATIONSTYLEBUILDER_H_
#define _CARTO_ANIMATIONSTYLEBUILDER_H_

#include "styles/AnimationStyle.h"

#include <memory>
#include <mutex>

namespace carto {

    /**
     * A builder for AnimationStyle instances.
     */
    class AnimationStyleBuilder {
    public:
        /**
         * Constructs an AnimationStyleBuilder object with all parameters set to defaults.
         */
        AnimationStyleBuilder();
        virtual ~AnimationStyleBuilder();
    
        /**
         * Returns the relative speed of the animation.
         * @return The relative speed of the animation (1.0 corresponds to the default speed).
         */
        float getRelativeSpeed() const;
        /**
         * Sets the relative speed of the animation. The relative speed affects all phases of the animation.
         * @param relativeSpeed The new relative speed of the animation.
         */
        void setRelativeSpeed(float relativeSpeed);

        /**
         * Returns the phase-in duration of the animation.
         * @return The phase-in duration of the animation in seconds. By default this is 0.5.
         */
        float getPhaseInDuration() const;
        /**
         * Sets the phase-in duration of the animation.
         * @param duration The new phase-in duration of the animation in seconds.
         */
        void setPhaseInDuration(float duration);

        /**
         * Returns the phase-out duration of the animation.
         * @return The phase-out duration of the animation in seconds. By default this is 0.0, which means instant effect.
         */
        float getPhaseOutDuration() const;
        /**
         * Sets the phase-out duration of the animation.
         * @param duration The new phase-out duration of the animation in seconds.
         */
        void setPhaseOutDuration(float duration);

        /**
         * Returns the fade animation type.
         * @return The type of the fade animation.
         */
        AnimationType::AnimationType getFadeAnimationType() const;
        /**
         * Sets the fade animation type.
         * @param animType The new type of the fade animation.
         */
        void setFadeAnimationType(AnimationType::AnimationType animType);

        /**
         * Returns the size-related animation type.
         * @return The type of the size-related animation.
         */
        AnimationType::AnimationType getSizeAnimationType() const;
        /**
         * Sets the size-related animation type.
         * @param animType The new type of the size-related animation.
         */
        void setSizeAnimationType(AnimationType::AnimationType animType);

        /**
         * Builds a new instance of the AnimationStyle object using previously set parameters.
         * @return A new AnimationStyle object.
         */
        std::shared_ptr<AnimationStyle> buildStyle() const;
    
    protected:
        float _relativeSpeed;
        float _phaseInDuration;
        float _phaseOutDuration;

        AnimationType::AnimationType _fadeAnimationType;
        AnimationType::AnimationType _sizeAnimationType;

        mutable std::mutex _mutex;
    };
    
}

#endif
