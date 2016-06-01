#ifdef _CARTO_WKBT_SUPPORT

#include "WKTGeometryWriter.h"
#include "WKTGeometryGenerator.h"
#include "Geometry.h"
#include "PointGeometry.h"
#include "LineGeometry.h"
#include "PolygonGeometry.h"
#include "MultiGeometry.h"
#include "MultiPointGeometry.h"
#include "MultiLineGeometry.h"
#include "MultiPolygonGeometry.h"
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
        std::lock_guard<std::mutex> lock(_mutex);

        std::string wkt;
        std::back_insert_iterator<std::string> it(wkt);
        WKTGeometryGenerator<std::back_insert_iterator<std::string> > grammar;
        bool result = boost::spirit::karma::generate(it, grammar(_z), geometry);
        if (!result) {
            Log::Error("WKTGeometryWriter: Failed to generate WKT");
            return std::string();
        }
        return wkt;
    }

}

#endif
