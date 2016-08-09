/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_POPUP_H_
#define _CARTO_POPUP_H_

#include "core/ScreenPos.h"
#include "vectorelements/Billboard.h"

namespace carto {
    class Bitmap;
    class PopupDrawData;
    class PopupStyle;
    
    /**
     * An abstract billboard element that can be displayed on the map. It
     * always faces the camera and has a dynamically drawn bitmap.
     */
    class Popup : public Billboard {
    public:
        /**
         * Constructs an abstract Popup object with the specified style and attaches it to a billboard element.
         * @param baseBillboard The billboard this popup will be attached to.
         * @param style The style that defines what this popup looks like.
         */
        Popup(const std::shared_ptr<Billboard>& baseBillboard, const std::shared_ptr<PopupStyle>& style);
        /**
         * Constructs an abstract Popup object from a geometry object and a style.
         * @param geometry The geometry object that defines the location of this popup.
         * @param style The style that defines what this popup looks like.
         */
        Popup(const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<PopupStyle>& style);
        /**
         * Constructs an abstract Popup object from a map position and a style.
         * @param pos The map position that defines the location of this popup.
         * @param style The style that defines what this popup looks like.
         */
        Popup(const MapPos& pos, const std::shared_ptr<PopupStyle>& style);
        virtual ~Popup();
    
        /**
         * Draws a custom bitmap for this Popup that will be used for drawing the Popup on the map.
         * The method is called each time the Popup gets reloaded internally.
         * @param anchorScreenPos The screen position of the anchor point of this popup in pixels.
         * @param screenWidth The current screen width in pixels.
         * @param screenHeight The current screen height in pixels.
         * @param dpToPX The value used for converting display independent pixels (dp) to pixels (px).
         * @return The custom Popup bitmap.
         */
        virtual std::shared_ptr<Bitmap> drawBitmap(const ScreenPos& anchorScreenPos,
                                                    float screenWidth, float screenHeight, float dpToPX) = 0;

        /**
         * Returns the horizontal anchor point of this popup.
         * @return The horizontal anchor point of this popup.
         */
        float getAnchorPointX() const;
        /**
         * Sets the horizontal anchor point for the popup. It should only be called from
         * Popup::drawBitmap method, to match the anchor point to the drawn bitmap. 
         * @param anchorPointX The new horizontal anchor point for the popup. -1 means the left side,
         * 0 the center and 1 the right side of the popup. The default is 0.
         */
        void setAnchorPointX(float anchorPointX);
        /**
         * Returns the vertical anchor point of this popup.
         * @return The vertical anchor point of this popup.
         */
        float getAnchorPointY() const;
        /**
         * Sets the vertical anchor point for the popup. It should only be called from
         * Popup::drawBitmap method, to match the anchor point to the drawn bitmap. 
         * @param anchorPointY The vertical anchor point for the popup. -1 means the bottom,
         * 0 the center and 1 the top of the popup. The default is -1.
         */
        void setAnchorPointY(float anchorPointY);
        /**
         * Sets the anchor point for the popup. It should only be called from
         * Popup::drawBitmap method, to match the anchor point to the drawn bitmap. 
         * Values will be clamped to [-1, 1] range.
         * @param anchorPointX The new horizontal anchor point for the popup. -1 means the left side,
         * 0 the center and 1 the right side of the popup. The default is 0.
         * @param anchorPointY The vertical anchor point for the popup. -1 means the bottom,
         * 0 the center and 1 the top of the popup. The default is -1.
         */
        void setAnchorPoint(float anchorPointX, float anchorPointY);
        
        /**
         * Returns the style of this Popup.
         * @return The style that defines what this Popup looks like.
         */
        std::shared_ptr<PopupStyle> getStyle() const;
        /**
         * Sets the style for this Popup.
         * @param style The new style that defines what this popup looks like.
         */
        void setStyle(const std::shared_ptr<PopupStyle>& style);
        
    private:
        float _anchorPointX;
        float _anchorPointY;
        
        std::shared_ptr<PopupStyle> _style;
    };
    
}

#endif
