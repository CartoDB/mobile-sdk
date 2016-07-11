/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_LABELSTYLEBUILDER_H_
#define _CARTO_LABELSTYLEBUILDER_H_

#include "styles/BillboardStyleBuilder.h"

#include <memory>

namespace carto {
    class LabelStyle;
    
    /**
     * A builder class for LabelStyle.
     */
    class LabelStyleBuilder : public BillboardStyleBuilder {
    public:
        /**
         * Constructs a LabelStyleBuilder object with all parameters set to defaults.
         */
        LabelStyleBuilder();
        virtual ~LabelStyleBuilder();
        
        /**
         * Returns the horizontal anchor point of the label.
         * @return The horizontal anchor point of the label.
         */
        float getAnchorPointX() const;
        /**
         * Sets the horizontal anchor point of the label.
         * @param anchorPointX The new horizontal anchor point for the label. -1 means the left side,
         * 0 the center and 1 the right side of the label. The default is 0.
         */
        void setAnchorPointX(float anchorPointX);
        /**
         * Returns the vertical anchor point of the label.
         * @return The vertical anchor point of the label.
         */
        float getAnchorPointY() const;
        /**
         * Sets the vertical anchor point of the label.
         * @param anchorPointY The vertical anchor point for the label. -1 means the bottom,
         * 0 the center and 1 the top of the label. The default is -1.
         */
        void setAnchorPointY(float anchorPointY);
        /**
         * Sets the anchor point for the label. Values will be clamped to [-1, 1] range.
         * @param anchorPointX The new horizontal anchor point for the label. -1 means the left side,
         * 0 the center and 1 the right side of the label. The default is 0.
         * @param anchorPointY The vertical anchor point for the label. -1 means the bottom,
         * 0 the center and 1 the top of the label. The default is -1.
         */
        void setAnchorPoint(float anchorPointX, float anchorPointY);
        
        /**
         * Returns the state of the flippable flag.
         * @return True if the label is flippable.
         */
        bool isFlippable() const;
        /**
         * Sets the state of the flippable flag. If set to true and the orientation mode is set to
         * GROUND the label is allowed to flip 180 degrees to try and face the camera better. This is
         * useful for street names and some other texts. The default is true.
         * @param flippable The new state of the flippable flag.
         */
        void setFlippable(bool flippable);
        
        /**
         * Returns the orientation mode of the label.
         * @return The orientation mode of the label.
         */
        BillboardOrientation::BillboardOrientation getOrientationMode() const;
        /**
         * Sets the orientation mode for the label. The default is BillboardOrientation::FACE_CAMERA_BILLBOARD.
         * @param orientationMode The new orientation mode for the label.
         */
        void setOrientationMode(BillboardOrientation::BillboardOrientation orientationMode);
        
        /**
         * Returns the scaling mode of the label.
         * @return The scaling mode of the label.
         */
        BillboardScaling::BillboardScaling getScalingMode() const;
        /**
         * Sets the scaling mode for the label. The default is BillboardScaling::CONST_SCREEN_SIZE.
         * @param scalingMode The new scaling mode for the label.
         */
        void setScalingMode(BillboardScaling::BillboardScaling scalingMode);
        
        /**
         * Builds a new instance of the LabelStyle object using previously set parameters.
         * @return A new LabelStyle object.
         */
        std::shared_ptr<LabelStyle> buildStyle() const;
    
    protected:
        float _anchorPointX;
        float _anchorPointY;
        
        bool _flippable;
        
        BillboardOrientation::BillboardOrientation _orientationMode;
        
        BillboardScaling::BillboardScaling _scalingMode;
    };
    
}

#endif
