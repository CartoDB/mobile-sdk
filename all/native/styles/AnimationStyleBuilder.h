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
        // TODO: doc
        AnimationStyleBuilder();
        virtual ~AnimationStyleBuilder();
    
        // TODO: doc
        float getRelativeSpeed() const;
        void setRelativeSpeed(float relativeSpeed);

        // TODO: doc
        AnimationType::AnimationType getFadeAnimationType() const;
        void setFadeAnimationType(AnimationType::AnimationType animType);

        // TODO: doc
        AnimationType::AnimationType getSizeAnimationType() const;
        void setSizeAnimationType(AnimationType::AnimationType animType);

        // TODO: doc
        std::shared_ptr<AnimationStyle> buildStyle() const;
    
    protected:
        float _relativeSpeed;

        AnimationType::AnimationType _fadeAnimationType;
        AnimationType::AnimationType _sizeAnimationType;

        mutable std::mutex _mutex;
    };
    
}

#endif
