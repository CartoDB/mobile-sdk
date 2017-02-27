/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ANIMATIONSTYLE_H_
#define _CARTO_ANIMATIONSTYLE_H_

namespace carto {

    namespace AnimationType {
        /**
         * Possible animation types.
         */
        enum AnimationType {
            /**
             * No animation is applied.
             */
            ANIMATION_TYPE_NONE,
            /**
             * Step transition is applied at the middle of the animation.
             */
            ANIMATION_TYPE_STEP,
            /**
             * Linear transition is used for animation.
             */
            ANIMATION_TYPE_LINEAR,
            /**
             * Smooth transition with 2nd order continuity is used for animation.
             */
            ANIMATION_TYPE_SMOOTHSTEP,
            /**
             * Spring-like transition is used for animation.
             */
            ANIMATION_TYPE_SPRING
        };
    };
    
    /**
     * A class describing animation effect.
     */
    class AnimationStyle {
    public:
        AnimationStyle(float relativeSpeed, AnimationType::AnimationType fadeAnimationType, AnimationType::AnimationType sizeAnimationType);
        virtual ~AnimationStyle();
    
        /**
         * Returns the relative speed of the animation.
         * @return The relative speed of the animation (1.0.
         */
        float getRelativeSpeed() const;

        /**
         * Returns the fade animation type.
         * @return The type of the fade animation.
         */
        AnimationType::AnimationType getFadeAnimationType() const;

        /**
         * Returns the size-related animation type.
         * @return The type of the size-related animation.
         */
        AnimationType::AnimationType getSizeAnimationType() const;
        
    protected:
        float _relativeSpeed;
        
        AnimationType::AnimationType _fadeAnimationType;
        AnimationType::AnimationType _sizeAnimationType;
    };
    
}

#endif
