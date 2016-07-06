/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_LABELSTYLE_H_
#define _CARTO_LABELSTYLE_H_

#include "styles/BillboardStyle.h"

#include <memory>

namespace carto {
    class Bitmap;
    
    /**
     * A style for labels. Contains attributes for configuring how the label is drawn on the screen.
     */
    class LabelStyle : public BillboardStyle {
    public:
        /**
         * Constructs a LabelStyle object from various parameters. Instantiating the object directly is
         * not recommended, LabelStyleBuilder should be used instead.
         * @param color The color for the label.
         * @param attachAnchorPointX The horizontal attaching anchor point for the label.
         * @param attachAnchorPointY The vertical attaching anchor point for the label.
         * @param causesOverlap The causes overlap flag for the label.
         * @param hideIfOverlapped The hide if overlapped flag for the label.
         * @param horizontalOffset The horizontal offset for the label.
         * @param verticalOffset The vertical offset for the label.
         * @param placementPriority The placement priority for the label.
         * @param scaleWithDPI The scale with DPI flag for the label.
         * @param anchorPointX The horizontal anchor point for the label.
         * @param anchorPointY The vertical anchor point for the label.
         * @param flippable The fliappble flag for the label.
         * @param orientationMode The orientation mode for the label.
         * @param scalingMode The scaling mode for the label.
         */
        LabelStyle(const Color& color,
                   float attachAnchorPointX,
                   float attachAnchorPointY,
                   bool causesOverlap,
                   bool hideIfOverlapped,
                   float horizontalOffset,
                   float verticalOffset,
                   int placementPriority,
                   bool scaleWithDPI,
                   float anchorPointX,
                   float anchorPointY,
                   bool flippable,
                   BillboardOrientation::BillboardOrientation orientationMode,
                   BillboardScaling::BillboardScaling scalingMode);
        virtual ~LabelStyle();
        
        /**
         * Returns the horizontal anchor point of the label.
         * @return The horizontal anchor point of the label.
         */
        float getAnchorPointX() const;
        /**
         * Returns the vertical anchor point of the label.
         * @return The vertical anchor point of the label.
         */
        float getAnchorPointY() const;
        
        /**
         * Returns the state of the flippable flag.
         * @return True if the label is flippable.
         */
        bool isFlippable() const;
        
        /**
         * Returns the orientation mode of the label.
         * @return The orientation mode of the label.
         */
        BillboardOrientation::BillboardOrientation getOrientationMode() const;
        
        /**
         * Returns the scaling mode of the label.
         * @return The scaling mode of the label.
         */
        BillboardScaling::BillboardScaling getScalingMode() const;

    protected:
        float _anchorPointX;
        float _anchorPointY;
        
        bool _flippable;
        
        BillboardOrientation::BillboardOrientation _orientationMode;
        
        BillboardScaling::BillboardScaling _scalingMode;
    };
    
}

#endif
