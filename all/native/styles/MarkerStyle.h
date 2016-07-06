/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MARKERSTYLE_H_
#define _CARTO_MARKERSTYLE_H_

#include "styles/BillboardStyle.h"

#include <memory>

namespace carto {
    class Bitmap;
    
    /**
     * A style for markers. Contains attributes for configuring how the marker is drawn on the screen.
     */
    class MarkerStyle : public BillboardStyle {
    public:
        /**
         * Constructs a MarkerStyle object from various parameters. Instantiating the object directly is
         * not recommended, MarkerStyleBuilder should be used instead.
         * @param color The color for the marker.
         * @param attachAnchorPointX The horizontal attaching anchor point for the marker.
         * @param attachAnchorPointY The vertical attaching anchor point for the marker.
         * @param causesOverlap The causes overlap flag for the marker.
         * @param hideIfOverlapped The hide if overlapped flag for the marker.
         * @param horizontalOffset The horizontal offset for the marker.
         * @param verticalOffset The vertical offset for the marker.
         * @param placementPriority The placement priority for the marker.
         * @param scaleWithDPI The scale with DPI flag for the label.
         * @param anchorPointX The horizontal anchor point for the marker.
         * @param anchorPointY The vertical anchor point for the marker.
         * @param bitmap The bitmap for the marker.
         * @param orientationMode The orientation mode for the marker.
         * @param scalingMode The scaling mode for the marker.
         * @param size The size for the marker.
         */
        MarkerStyle(const Color& color,
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
                    const std::shared_ptr<Bitmap>& bitmap,
                    BillboardOrientation::BillboardOrientation orientationMode,
                    BillboardScaling::BillboardScaling scalingMode,
                    float size);
        virtual ~MarkerStyle();
    
        /**
         * Returns the horizontal anchor point of the marker.
         * @return The horizontal anchor point of the marker.
         */
        float getAnchorPointX() const;
        /**
         * Returns the vertical anchor point of the marker.
         * @return The vertical anchor point of the marker.
         */
        float getAnchorPointY() const;
        
        /**
         * Returns the bitmap of the marker.
         * @return The bitmap of the marker.
         */
        std::shared_ptr<Bitmap> getBitmap() const;
        
        /**
         * Returns the orientation mode of the marker.
         * @return The orientation mode of the marker.
         */
        BillboardOrientation::BillboardOrientation getOrientationMode() const;
        
        /**
         * Returns the scaling mode of the marker.
         * @return The scaling mode of the marker.
         */
        BillboardScaling::BillboardScaling getScalingMode() const;
        
        /**
         * Returns the size of the marker.
         * @return The size of the marker, units depend on the scaling mode.
         */
        float getSize() const;
    
    protected:
        float _anchorPointX;
        float _anchorPointY;
        
        std::shared_ptr<Bitmap> _bitmap;
        
        BillboardOrientation::BillboardOrientation _orientationMode;
        
        BillboardScaling::BillboardScaling _scalingMode;
        
        float _size;
    };
    
}

#endif
