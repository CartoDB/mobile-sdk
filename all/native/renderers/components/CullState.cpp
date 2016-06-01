#include "CullState.h"
#include "projections/Projection.h"

namespace carto {
    
    CullState::CullState(const MapEnvelope& envelope, const ViewState& viewState) :
        _envelope(envelope),
        _viewState(viewState)
    {
    }
        
    CullState::~CullState() {
    }

    MapEnvelope CullState::getProjectionEnvelope(const std::shared_ptr<Projection>& proj) const {
        std::vector<MapPos> convexHull;
        for (const MapPos& mapPosInternal : _envelope.getConvexHull()) {
            convexHull.push_back(proj->fromInternal(mapPosInternal));
        }
        return MapEnvelope(convexHull);
    }
    
    const MapEnvelope& CullState::getEnvelope() const {
        return _envelope;
    }
         
    const ViewState& CullState::getViewState() const {
        return _viewState;
    }
    
}
