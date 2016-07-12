#ifdef _CARTO_WKBT_SUPPORT

#include "WKTGeometryWriter.h"
#include "components/Exceptions.h"
#include "geometry/Geometry.h"
#include "geometry/PointGeometry.h"
#include "geometry/LineGeometry.h"
#include "geometry/PolygonGeometry.h"
#include "geometry/MultiGeometry.h"
#include "geometry/MultiPointGeometry.h"
#include "geometry/MultiLineGeometry.h"
#include "geometry/MultiPolygonGeometry.h"
#include "geometry/WKTGeometryGenerator.h"
#include "utils/Log.h"

namespace carto {

    WKTGeometryWriter::WKTGeometryWriter() :
        _z(false),
        _mutex()
    {
    }

    bool WKTGeometryWriter::getZ() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _z;
    }

    void WKTGeometryWriter::setZ(bool z) {
        std::lock_guard<std::mutex> lock(_mutex);
        _z = z;
    }

    std::string WKTGeometryWriter::writeGeometry(const std::shared_ptr<Geometry>& geometry) const {
        if (!geometry) {
            throw std::invalid_argument("Null geometry");
        }

        std::lock_guard<std::mutex> lock(_mutex);

        std::string wkt;
        std::back_insert_iterator<std::string> it(wkt);
        WKTGeometryGenerator<std::back_insert_iterator<std::string> > grammar;
        bool result = boost::spirit::karma::generate(it, grammar(_z), geometry);
        if (!result) {
            throw GenerateException("Failed to generate WKT");
        }
        return wkt;
    }

}

#endif
