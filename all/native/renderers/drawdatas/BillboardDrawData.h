/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BILLBOARDDRAWDATA_H_
#define _CARTO_BILLBOARDDRAWDATA_H_

#include "styles/BillboardStyle.h"
#include "renderers/drawdatas/VectorElementDrawData.h"

#include <atomic>
#include <memory>

#include <cglib/vec.h>

namespace carto {
    class Bitmap;
    class Billboard;
    class BillboardRenderer;
    class BillboardStyle;
    class Projection;
    
    class BillboardDrawData : public VectorElementDrawData {
    public:
        virtual ~BillboardDrawData();
    
        float getAnchorPointX() const;
        float getAnchorPointY() const;
        
        float getAspect() const;
    
        float getAttachAnchorPointX() const;
        float getAttachAnchorPointY() const;

        const std::weak_ptr<Billboard>& getBillboard() const;    
        const std::weak_ptr<Billboard>& getBaseBillboard() const;
    
        std::shared_ptr<Bitmap> getBitmap() const;
    
        const cglib::vec2<float>* getCoords() const;
    
        bool isFlippable() const;
    
        float getHorizontalOffset() const;
        float getVerticalOffset() const;
        
        bool isGenMipmaps() const;
    
        BillboardOrientation::BillboardOrientation getOrientationMode() const;
    
        bool isCausesOverlap() const;
        bool isHideIfOverlapped() const;
        
        // These two methods may be called from multiple threads at the same time
        bool isOverlapping() const;
        void setOverlapping(bool overlapping);
    
        float getPlacementPriority() const;
    
        const cglib::vec3<double>& getPos() const;
        void setPos(const cglib::vec3<double>& pos); // Used for setting label positon, when it's attached to another billboards
    
        float getRotation() const;
        
        bool isScaleWithDPI() const;
        
        BillboardScaling::BillboardScaling getScalingMode() const;
    
        float getSize() const;
    
        double getCameraPlaneZoomDistance() const;
        void setCameraPlaneZoomDistance(double cameraPlaneDistance);
    
        double getScreenBottomDistance() const;
        void setScreenBottomDistance(double screenBottomDistance);
    
        BillboardRenderer* getRenderer() const;
        void setRenderer(BillboardRenderer& renderer);
    
        virtual void offsetHorizontally(double offset);
    
    protected:
        BillboardDrawData(const Billboard& billboard,
                          const BillboardStyle& style,
                          const Projection& projection,
                          const std::shared_ptr<Bitmap>& bitmap,
                          float anchorPointX,
                          float anchorPointY,
                          bool flippable,
                          BillboardOrientation::BillboardOrientation _orientationMode,
                          BillboardScaling::BillboardScaling _scalingMode,
                          float size);
    
        float _anchorPointX;
        float _anchorPointY;
    
        float _aspect;
        
        float _attachAnchorPointX;
        float _attachAnchorPointY;
  
        std::weak_ptr<Billboard> _billboard;  
        std::weak_ptr<Billboard> _baseBillboard;
    
        std::shared_ptr<Bitmap> _bitmap;
    
        cglib::vec2<float> _coords[4];
    
        bool _flippable;
    
        float _horizontalOffset;
        float _verticalOffset;
        
        bool _genMipmaps;
    
        BillboardOrientation::BillboardOrientation _orientationMode;
    
        bool _causesOverlap;
        bool _hideIfOverlapped;
        std::atomic<bool> _overlapping;
        
        int _placementPriority;
    
        cglib::vec3<double> _pos;
    
        float _rotation;
        
        bool _scaleWithDPI;
        
        BillboardScaling::BillboardScaling _scalingMode;
    
        float _size;
    
        double _cameraPlaneZoomDistance;
        double _screenBottomDistance;
    
        BillboardRenderer* _renderer;
    };
    
}

#endif
