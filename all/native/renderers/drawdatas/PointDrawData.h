/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_POINTDRAWDATA_H_
#define _CARTO_POINTDRAWDATA_H_

#include "renderers/drawdatas/VectorElementDrawData.h"

#include <memory>

#include <cglib/vec.h>

namespace carto {
    class Bitmap;
    class PointGeometry;
    class PointStyle;
    class Projection;
    
    class PointDrawData : public VectorElementDrawData {
    public:
        PointDrawData(const PointGeometry& geometry, const PointStyle& style, const Projection& projection);
        virtual ~PointDrawData();
    
        const std::shared_ptr<Bitmap> getBitmap() const;
    
        float getClickScale() const;
    
        const cglib::vec3<double>& getPos() const;
    
        float getSize() const;
    
        virtual void offsetHorizontally(double offset);
    
    private:
        static const int IDEAL_CLICK_SIZE = 64;
    
        static const float CLICK_SIZE_COEF;
    
        std::shared_ptr<Bitmap> _bitmap;
    
        float _clickScale;
    
        cglib::vec3<double> _pos;
    
        float _size;
    };
    
}

#endif
