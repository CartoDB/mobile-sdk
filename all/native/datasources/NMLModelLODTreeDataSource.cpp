#ifdef _CARTO_NMLMODELLODTREE_SUPPORT

#include "NMLModelLODTreeDataSource.h"
#include "projections/Projection.h"

namespace carto {

    NMLModelLODTreeDataSource::~NMLModelLODTreeDataSource() {
    }
    
    NMLModelLODTreeDataSource::NMLModelLODTreeDataSource(const std::shared_ptr<Projection>& projection) :
        _projection(projection),
        _mutex()
    {
        if (!projection) {
            throw std::invalid_argument("Null projection");
        }
    }
    
    std::shared_ptr<Projection> NMLModelLODTreeDataSource::getProjection() const {
        return _projection;
    }
    
}

#endif
