/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NMLMODELDRAWDATA_H_
#define _CARTO_NMLMODELDRAWDATA_H_

#include "renderers/drawdatas/VectorElementDrawData.h"

#include <memory>

#include <cglib/mat.h>

namespace carto {
    namespace nml {
        class Model;
    }

    class NMLModelDrawData : public VectorElementDrawData {
    public:
        NMLModelDrawData(const std::shared_ptr<nml::Model>& sourceModel, const cglib::mat4x4<double>& localMat);
    
        std::shared_ptr<nml::Model> getSourceModel() const;
        const cglib::mat4x4<double>& getLocalMat() const;
        
        virtual void offsetHorizontally(double offset);
    
    private:
        std::shared_ptr<nml::Model> _sourceModel;
        cglib::mat4x4<double> _localMat;
    };
    
}

#endif
