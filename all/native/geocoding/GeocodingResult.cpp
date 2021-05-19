#ifdef _CARTO_GEOCODING_SUPPORT

#include "GeocodingResult.h"
#include "components/Exceptions.h"
#include "geometry/FeatureCollection.h"
#include "projections/Projection.h"

#include <iomanip>
#include <sstream>

namespace carto {

    GeocodingResult::GeocodingResult(const std::shared_ptr<Projection>& projection, const GeocodingAddress& address, float rank, const std::shared_ptr<FeatureCollection>& featureCollection) :
        _address(address),
        _rank(rank),
        _featureCollection(featureCollection),
        _projection(projection)
    {
        if (!featureCollection) {
            throw NullArgumentException("Null geometry");
        }
        if (!projection) {
            throw NullArgumentException("Null projection");
        }
    }

    GeocodingResult::~GeocodingResult() {
    }

    const GeocodingAddress& GeocodingResult::getAddress() const {
        return _address;
    }

    float GeocodingResult::getRank() const {
        return _rank;
    }

    const std::shared_ptr<FeatureCollection>& GeocodingResult::getFeatureCollection() const {
        return _featureCollection;
    }

    const std::shared_ptr<Projection>& GeocodingResult::getProjection() const {
        return _projection;
    }

    std::string GeocodingResult::toString() const {
        std::stringstream ss;
        ss << std::setiosflags(std::ios::fixed);
        ss << "GeocodingResult [";
        ss << "rank=" << _rank << ", ";
        ss << "address=" << _address.toString();
        ss << "]";
        return ss.str();
    }
    
}

#endif
