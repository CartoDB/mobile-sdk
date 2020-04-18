/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VECTORELEMENTDRAWDATA_H_
#define _CARTO_VECTORELEMENTDRAWDATA_H_

#include "graphics/Color.h"

#include <memory>

namespace carto {
    class ProjectionSurface;
    class VectorElement;
    
    class VectorElementDrawData {
    public:
        virtual ~VectorElementDrawData();
    
        const Color& getColor() const;
        
        virtual bool isOffset() const;
        virtual void offsetHorizontally(double offset) = 0;

        const std::shared_ptr<ProjectionSurface>& getProjectionSurface() const;
    
    protected:
        static Color GetPremultipliedColor(const Color& color);

        VectorElementDrawData(const Color& color, const std::shared_ptr<ProjectionSurface>& projectionSurface);

        void setIsOffset(bool isOffset);

        std::shared_ptr<ProjectionSurface> _projectionSurface;

    private:
        Color _color;
        bool _isOffset;
    };
    
}

#endif
