/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MARKERSTYLEBUILDER_H_
#define _CARTO_MARKERSTYLEBUILDER_H_

#include "styles/BillboardStyleBuilder.h"

#include <memory>

namespace carto {
    class Bitmap;
    class MarkerStyle;
    
    /**
     * A builder class for MarkerStyle.
     */
    class MarkerStyleBuilder : public BillboardStyleBuilder {
    public:
        /**
         * Constructs a MarkerStyleBuilder object with all parameters set to defaults.
         */
        MarkerStyleBuilder();
        virtual ~MarkerStyleBuilder();
        
        /**
         * Returns the horizontal anchor point of the marker.
         * @return The horizontal anchor point of the marker.
         */
        float getAnchorPointX() const;
        /**
         * Sets the horizontal anchor point of the marker.
         * @param anchorPointX The new horizontal anchor point for the marker. -1 means the left side,
         * 0 the center and 1 the right side of the marker. The default is 0.
         */
        void setAnchorPointX(float anchorPointX);
        /**
         * Returns the vertical anchor point of the marker.
         * @return The vertical anchor point of the marker.
         */
        float getAnchorPointY() const;
        /**
         * Sets the vertical anchor point of the marker.
         * @param anchorPointY The vertical anchor point for the marker. -1 means the bottom,
         * 0 the center and 1 the top of the marker. The default is -1.
         */
        void setAnchorPointY(float anchorPointY);
        /**
         * Sets the anchor point for the marker. Values will be clamped to [-1, 1] range.
         * @param anchorPointX The new horizontal anchor point for the marker. -1 means the left side,
         * 0 the center and 1 the right side of the marker. The default is 0.
         * @param anchorPointY The vertical anchor point for the marker. -1 means the bottom,
         * 0 the center and 1 the top of the marker. The default is -1.
         */
        void setAnchorPoint(float anchorPointX, float anchorPointY);
    
        /**
         * Returns the bitmap of the marker.
         * @return The bitmap of the marker.
         */
        std::shared_ptr<Bitmap> getBitmap() const;
        /**
         * Sets the bitmap that will be used for drawing the marker. The default is "default_marker.png".
         * @param bitmap The new bitmap for the marker.
         */
        void setBitmap(const std::shared_ptr<Bitmap>& bitmap);
        
        /**
         * Returns the orientation mode of the marker.
         * @return The orientation mode of the marker.
         */
        BillboardOrientation::BillboardOrientation getOrientationMode() const;
        /**
         * Sets the orientation mode for the marker. The default is BillboardOrientation::FACE_CAMERA_BILLBOARD.
         * @param orientationMode The new orientation mode for the marker.
         */
        void setOrientationMode(BillboardOrientation::BillboardOrientation orientationMode);
        
        /**
         * Returns the scaling mode of the marker.
         * @return The scaling mode of the marker.
         */
        BillboardScaling::BillboardScaling getScalingMode() const;
        /**
         * Sets the scaling mode for the marker. The default is BillboardScaling::CONST_SCREEN_SIZE.
         * @param scalingMode The new scaling mode for the marker.
         */
        void setScalingMode(BillboardScaling::BillboardScaling scalingMode);
        
        /**
         * Returns the size of the marker.
         * @return The size of the marker, units depend on the scaling mode.
         */
        float getSize() const;
        /**
         * Sets the size for the marker. The units depend on the scaling mode, if it's set to BillboardScaling::WORLD_SIZE,
         * the size is in meters near the equator, the size will not be stretched near the poles. If it's set to
         * BillboardScaling::SCREEN_SIZE or BillboardScaling::CONST_SCREEN_SIZE then the size is in screen density
         * independent pixels (dp or dip) or normal pixels, depending whether setScaleWithDPI was set to true or false
         * respectively. If set to -1, unpadded bitmap width will be used instead. The default is -1.
         * @param size The new marker size.
         */
        void setSize(float size);
    
        /**
         * Builds a new instance of the MarkerStyle object using previously set parameters.
         * @return A new MarkerStyle object.
         */
        std::shared_ptr<MarkerStyle> buildStyle() const;
    
    protected:
        static std::shared_ptr<Bitmap> GetDefaultBitmap();
    
        static std::shared_ptr<Bitmap> _DefaultBitmap;
        static std::mutex _DefaultBitmapMutex;
        
        float _anchorPointX;
        float _anchorPointY;
    
        std::shared_ptr<Bitmap> _bitmap;
        
        BillboardOrientation::BillboardOrientation _orientationMode;
        
        BillboardScaling::BillboardScaling _scalingMode;
        
        float _size;
    };
    
}

#endif
