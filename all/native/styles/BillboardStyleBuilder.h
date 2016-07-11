/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BILLBOARDSTYLEBUILDER_H_
#define _CARTO_BILLBOARDSTYLEBUILDER_H_

#include "styles/BillboardStyle.h"
#include "styles/StyleBuilder.h"

#include <memory>

namespace carto {

    /**
     * A base class for BillboardStyleBuilder subclasses.
     */
    class BillboardStyleBuilder : public StyleBuilder {
    public:
        virtual ~BillboardStyleBuilder();
    
        /**
         * Returns the horizontal attaching anchor point of the billboard.
         * @return The horizontal attaching anchor point of the billboard.
         */
        float getAttachAnchorPointX() const;
        /**
         * Sets the horizontal attaching anchor point of the billboard.
         * @param attachAnchorPointX The new horizontal attaching anchor point for the billboard. -1 means the left side,
         * 0 the center and 1 the right side. The default is 0.
         */
        void setAttachAnchorPointX(float attachAnchorPointX);
        /**
         * Returns the vertical attaching anchor point of the billboard.
         * @return The vertical attaching anchor point of the billboard.
         */
        float getAttachAnchorPointY() const;
        /**
         * Sets the vertical attaching anchor point of the billboard.
         * @param attachAnchorPointY The new vertical attaching anchor point for the billboard. -1 means the bottom,
         * 0 the center and 1 the top. The default is 1.
         */
        void setAttachAnchorPointY(float attachAnchorPointY);
        /**
         * Sets the attaching anchor point for the billboard. The attaching anchor point is
         * only used if the billboard is attached to another (base) billboard. It defines the point
         * on the base billboard where the attachable billboard will be placed.
         * @param attachAnchorPointX The new horizontal attaching anchor point for the billboard. -1 means the left side,
         * 0 the center and 1 the right side. The default is 0.
         * @param attachAnchorPointY The new vertical attaching anchor point for the billboard. -1 means the bottom,
         * 0 the center and 1 the top. The default is 1.
         */
        void setAttachAnchorPoint(float attachAnchorPointX, float attachAnchorPointY);
        
        /**
         * Returns the state of the causes overlap flag.
         * @return True if this billboard causes overlapping with other billboards behind it.
         */
        bool isCausesOverlap() const;
        /**
         * Sets the state of the causes overlap flag. If set to false the billboard will
         * never hide other billboards, even if they are overlapping and have the hide if overlapped flag
         * set to true. If set to true the billboard may hide other overlapping billboards that have the
         * hide if overlapped flag set to true and are located behind this billboard. This billboard may still
         * be hidden by other billboards in front of it, regardless of this parameter.
         * The default depends on the subclass.
         * @param causesOverlap The new state of the allow overlap flag.
         */
        void setCausesOverlap(bool causesOverlap);
        /**
         * Returns the state of the allow overlap flag.
         * @return True if this billboard can be hidden by overlapping billboards in front of it.
         */
        bool isHideIfOverlapped() const;
        /**
         * Sets the state of the hide if overlapped flag. If set to false the billboard may be
         * hidden by other billboards that are overlapping with it and are located
         * in front of this billboard. If set to true the billboard will never
         * be hidden by other overlapping billboards. The default depends on the subclass.
         * @param hideIfOverlapped The new state of the allow overlap flag.
         */
        void setHideIfOverlapped(bool hideIfOverlapped);
    
        /**
         * Returns the horizontal offset of the billboard.
         * @return The horizontal offset of the billboard, units depend on the scaling mode.
         */
        float getHorizontalOffset() const;
        /**
         * Sets the horizontal offset for the billboard. Horizontal offset is used to offset the bitmap 
         * of the billboard horizontally relative to the billboard rotation. The units are the same as the
         * units used in the setSize method. Negative values offset the bitmap to the left and
         * positive values to the right. The default is 0.
         * @param horizontalOffset The new horizontal offset for the billboard.
         */
        void setHorizontalOffset(float horizontalOffset);
        /**
         * Returns the vertical offset of the billboard.
         * @return The vertical offset of the billboard, units depend on the scaling mode.
         */
        float getVerticalOffset() const;
        /**
         * Sets the vertical offset for the billboard. Vertical offset is used to offset the bitmap
         * of the billboard vertically relative to the billboard rotation. The units are the same as the
         * units used in the setSize method. Negative values offset the bitmap to the bottom and positive values to the top.
         * The default is 0.
         * @param verticalOffset The new vertical offset for the billboard.
         */
        void setVerticalOffset(float verticalOffset);
    
        /**
         * Returns the placement priority of the billboard.
         * @return The placement priority of the billboard.
         */
        int getPlacementPriority() const;
        /**
         * Sets the placement priority for the billboard. Higher priority billboard get drawn in front of lower
         * priority billboards regardless of their distance to the camera. If billboards are not allowed to overlap then
         * higher priority billboards hide overlapping lower priority billboards. The default is 0.
         * @param placementPriority The new placement priority for the billboard.
         */
        void setPlacementPriority(int placementPriority);
        
        /**
         * Returns the state of the scale with DPI flag.
         * @return True if this billboard's size will be scaled using the screen dot's per inch.
         */
        bool isScaleWithDPI() const;
        /**
         * Sets the state of the scale with DPI flag. If set to true the billboard's size will scale using the screen
         * dots per inch parameter. This means that billboards will look the same size regardless of the device screen size
         * and density. If set to false the billboards will not be scaled with the screen density parameter, which means 
         * that billboard will look smaller on on higher density screens. Custom Label and Popup implementations may,
         * but are not guaranteed to compensate for this by generating higher resolution images. The default depends on the subclass.
         * @param scaleWithDPI The new state of the scale with DPI flag.
         */
        void setScaleWithDPI(bool scaleWithDPI);
    
    protected:
        BillboardStyleBuilder();

        float _attachAnchorPointX;
        float _attachAnchorPointY;
        
        bool _causesOverlap;
        bool _hideIfOverlapped;
    
        float _horizontalOffset;
        float _verticalOffset;
    
        int _placementPriority;
        
        bool _scaleWithDPI;
    };
    
}

#endif
