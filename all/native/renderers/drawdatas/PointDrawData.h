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
        PointDrawData(const PointGeometry& geometry, const PointStyle& style, const Projection& projection, const std::shared_ptr<ProjectionSurface>& projectionSurface);
        virtual ~PointDrawData();
    
        const std::shared_ptr<Bitmap> getBitmap() const;
    
        float getClickScale() const;
    
        const cglib::vec3<double>& getPos() const;
        const cglib::vec3<float>& getXAxis() const;
        const cglib::vec3<float>& getYAxis() const;

        float getSize() const;
    
        virtual void offsetHorizontally(double offset);
    
    private:
        static const int IDEAL_CLICK_SIZE;
    
        static const float CLICK_SIZE_COEF;
    
        std::shared_ptr<Bitmap> _bitmap;
    
        float _clickScale;
    
        cglib::vec3<double> _pos;
        cglib::vec3<float> _xAxis;
        cglib::vec3<float> _yAxis;

        float _size;
    };
    
}

#endif
