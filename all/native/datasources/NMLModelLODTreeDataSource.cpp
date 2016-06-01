#include "NMLModelLODTreeDataSource.h"
#include "projections/Projection.h"

namespace carto {

    NMLModelLODTreeDataSource::~NMLModelLODTreeDataSource() {
    }
    
    NMLModelLODTreeDataSource::NMLModelLODTreeDataSource(const std::shared_ptr<Projection>& projection) :
        _projection(projection),
        _mutex()
    {
    }
    
    std::shared_ptr<Projection> NMLModelLODTreeDataSource::getProjection() const {
        return _projection;
    }
    
}
