#ifdef _CARTO_WKBT_SUPPORT

#include "WKTGeometryReader.h"
#include "components/Exceptions.h"
#include "geometry/Geometry.h"
#include "geometry/PointGeometry.h"
#include "geometry/LineGeometry.h"
#include "geometry/PolygonGeometry.h"
#include "geometry/MultiGeometry.h"
#include "geometry/MultiPointGeometry.h"
#include "geometry/MultiLineGeometry.h"
#include "geometry/MultiPolygonGeometry.h"
#include "geometry/WKTGeometryParser.h"
#include "utils/Log.h"

#include <memory>
#include <cstddef>
#include <vector>
#include <stack>
#include <stdexcept>

namespace carto {
    
    WKTGeometryReader::WKTGeometryReader() {
    }

    std::shared_ptr<Geometry> WKTGeometryReader::readGeometry(const std::string& wkt) const {
        std::string::const_iterator it = wkt.begin();
        std::string::const_iterator end = wkt.end();
        WKTGeometryParserImpl::encoding::space_type space;
        std::shared_ptr<Geometry> geometry;
        bool result = boost::spirit::qi::phrase_parse(it, end, WKTGeometryParser<std::string::const_iterator>(), space, geometry);
        if (!result) {
            throw ParseException("Failed to parse WKT geometry", wkt);
        } else if (it != wkt.end()) {
            throw ParseException("Could not parse to the end of WKT geometry", wkt, static_cast<int>(it - wkt.begin()));
        }
        return geometry;
    }

}

#endif
