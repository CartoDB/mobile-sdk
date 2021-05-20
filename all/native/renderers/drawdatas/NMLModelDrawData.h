/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NMLMODELDRAWDATA_H_
#define _CARTO_NMLMODELDRAWDATA_H_

#include "renderers/drawdatas/BillboardDrawData.h"

#include <memory>

#include <cglib/mat.h>
#include <cglib/bbox.h>

namespace carto {
    class NMLModel;
    class NMLModelStyle;
    class Projection;
    
    namespace nml {
        class Model;
    }

    class NMLModelDrawData : public BillboardDrawData {
    public:
        NMLModelDrawData(const NMLModel& model, const NMLModelStyle& style, const Projection& projection, const std::shared_ptr<ProjectionSurface>& projectionSurface);
        virtual ~NMLModelDrawData();
    
        std::shared_ptr<nml::Model> getSourceModel() const;
        const cglib::bbox3<float>& getSourceModelBounds() const;
        const cglib::mat4x4<double>& getLocalFrameMat() const;
        const cglib::mat4x4<double>& getLocalTransformMat() const;
        
        virtual void offsetHorizontally(double offset);
    
    private:
        std::shared_ptr<nml::Model> _sourceModel;
        cglib::bbox3<float> _sourceModelBounds;
        cglib::mat4x4<double> _localFrameMat;
        cglib::mat4x4<double> _localTransformMat;
    };
    
}

#endif
