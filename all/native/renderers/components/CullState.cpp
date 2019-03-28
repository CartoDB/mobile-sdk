#include "CullState.h"
#include "components/Exceptions.h"
#include "projections/Projection.h"
#include "projections/EPSG3857.h"
#include "utils/GeomUtils.h"

namespace carto {
    
    CullState::CullState(const MapEnvelope& envelope, const ViewState& viewState) :
        _envelope(envelope),
        _viewState(viewState)
    {
    }
        
    CullState::~CullState() {
    }

    MapEnvelope CullState::getProjectionEnvelope(const std::shared_ptr<Projection>& projection) const {
        if (!projection) {
            throw NullArgumentException("Null projection");
        }

        std::vector<MapPos> mapPoses;
        if (std::dynamic_pointer_cast<EPSG3857>(projection)) {
            mapPoses.reserve(_envelope.getConvexHull().size());
            for (const MapPos& mapPosInternal : _envelope.getConvexHull()) {
                mapPoses.push_back(projection->fromInternal(mapPosInternal));
            }
        } else {
            MapPos minPos = projection->fromInternal(_envelope.getBounds().getMin());
            MapPos maxPos = projection->fromInternal(_envelope.getBounds().getMax());
            mapPoses.reserve(4);
            mapPoses.emplace_back(minPos.getX(), minPos.getY());
            mapPoses.emplace_back(minPos.getX(), maxPos.getY());
            mapPoses.emplace_back(maxPos.getX(), maxPos.getY());
            mapPoses.emplace_back(maxPos.getX(), minPos.getY());
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
