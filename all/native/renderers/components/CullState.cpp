#include "CullState.h"
#include "projections/Projection.h"
#include "utils/GeomUtils.h"

namespace carto {
    
    CullState::CullState(const MapEnvelope& envelope, const ViewState& viewState) :
        _envelope(envelope),
        _viewState(viewState)
    {
    }
        
    CullState::~CullState() {
    }

    MapEnvelope CullState::getProjectionEnvelope(const std::shared_ptr<Projection>& proj) const {
        std::vector<MapPos> mapPoses;
        mapPoses.reserve(8);
        for (const MapPos& mapPosInternal : _envelope.getConvexHull()) {
            mapPoses.push_back(proj->fromInternal(mapPosInternal));
        }
        return MapEnvelope(GeomUtils::CalculateConvexHull(mapPoses));
    }
    
    const MapEnvelope& CullState::getEnvelope() const {
        return _envelope;
    }
         
    const ViewState& CullState::getViewState() const {
        return _viewState;
    }
    
}
