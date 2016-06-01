#ifdef _CARTO_WKBT_SUPPORT

#include "WKTGeometryReader.h"
#include "Geometry.h"
#include "PointGeometry.h"
#include "LineGeometry.h"
#include "PolygonGeometry.h"
#include "MultiGeometry.h"
#include "MultiPointGeometry.h"
#include "MultiLineGeometry.h"
#include "MultiPolygonGeometry.h"
#include "WKTGeometryParser.h"
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
            Log::Error("WKTGeometryReader: Failed to parse WKT geometry.");
        } else if (it != wkt.end()) {
            Log::Error("WKTGeometryReader: Could not parse to the end of WKT geometry.");
        }
        return geometry;
    }

}

#endif
