#ifdef _CARTO_GEOCODING_SUPPORT

#include "GeocodingResult.h"
#include "components/Exceptions.h"
#include "geometry/Geometry.h"
#include "projections/Projection.h"

#include <iomanip>
#include <sstream>

namespace carto {

    GeocodingResult::GeocodingResult(const std::shared_ptr<Projection>& projection, long long id, const GeocodingAddress& address, float rank, const std::shared_ptr<Geometry>& geometry) :
        _id(id),
        _address(address),
        _rank(rank),
        _geometry(geometry),
        _projection(projection)
    {
        if (!geometry) {
            throw NullArgumentException("Null geometry");
        }
        if (!projection) {
            throw NullArgumentException("Null projection");
        }
    }

    GeocodingResult::~GeocodingResult() {
    }

    long long GeocodingResult::getId() const {
        return _id;
    }

    const GeocodingAddress& GeocodingResult::getAddress() const {
        return _address;
    }

    float GeocodingResult::getRank() const {
        return _rank;
    }

    const std::shared_ptr<Geometry>& GeocodingResult::getGeometry() const {
        return _geometry;
    }

    const std::shared_ptr<Projection>& GeocodingResult::getProjection() const {
        return _projection;
    }

    std::string GeocodingResult::toString() const {
        std::stringstream ss;
        ss << std::setiosflags(std::ios::fixed);
        ss << "GeocodingResult [";
        ss << "id=" << _id << ", ";
        ss << "rank=" << _rank << ", ";
        ss << "address=" << _address.toString();
        ss << "]";
        return ss.str();
    }
    
}

#endif
