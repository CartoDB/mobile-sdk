#include "NMLModelDrawData.h"

namespace carto {

    NMLModelDrawData::NMLModelDrawData(const std::shared_ptr<nml::Model>& sourceModel, const cglib::mat4x4<double>& localMat) :
        VectorElementDrawData(Color()),
        _sourceModel(sourceModel),
        _localMat(localMat)
    {
    }
    
    std::shared_ptr<nml::Model> NMLModelDrawData::getSourceModel() const {
        return _sourceModel;
    }
    
    const cglib::mat4x4<double>& NMLModelDrawData::getLocalMat() const {
        return _localMat;
    }
    
    void NMLModelDrawData::offsetHorizontally(double offset) {
        _localMat = cglib::translate4_matrix(cglib::vec3<double>(offset, 0, 0)) * _localMat;
        setIsOffset(true);
    }

}
