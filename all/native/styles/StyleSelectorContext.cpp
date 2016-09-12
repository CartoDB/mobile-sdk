#ifdef _CARTO_GDAL_SUPPORT

#include "StyleSelectorContext.h"
#include "graphics/ViewState.h"
#include "geometry/PointGeometry.h"
#include "geometry/LineGeometry.h"
#include "geometry/PolygonGeometry.h"
#include "geometry/MultiGeometry.h"
#include "geometry/MultiPointGeometry.h"
#include "geometry/MultiLineGeometry.h"
#include "geometry/MultiPolygonGeometry.h"

#include <vector>
#include <algorithm>
#include <numeric>

namespace carto {

    StyleSelectorContext::StyleSelectorContext(const ViewState& viewState, const std::shared_ptr<Geometry>& geometry, const std::map<std::string, std::string>& metaData) :
        _viewState(viewState), _geometry(geometry), _metaData(metaData)
    {
    }

    const ViewState& StyleSelectorContext::getViewState() const {
        return _viewState;
    }
    
    const std::shared_ptr<Geometry>& StyleSelectorContext::getGeometry() const {
        return _geometry;
    }
        
    const std::map<std::string, std::string>& StyleSelectorContext::getMetaData() const {
        return _metaData;
    }

    bool StyleSelectorContext::getVariable(const std::string& name, boost::variant<double, std::string>& value) const {
        auto it = _metaData.find(name);
        if (it != _metaData.end()) {
            value = it->second;
            return true;
        }

        if (name == "view::zoom") {
            value = static_cast<double>(_viewState.getZoom());
            return true;
        }

        if (name == "geometry::type") {
            value = GetGeometryType(_geometry);
            return true;
        }

        if (name == "geometry::vertices") {
            value = static_cast<double>(GetGeometryVerticesCount(_geometry));
            return true;
        }

        return false;
    }
    
    std::string StyleSelectorContext::GetGeometryType(const std::shared_ptr<Geometry> &geometry) {
        if (std::dynamic_pointer_cast<PointGeometry>(geometry)) {
            return "point";
        } else if (std::dynamic_pointer_cast<LineGeometry>(geometry)) {
            return "linestring";
        } else if (std::dynamic_pointer_cast<PolygonGeometry>(geometry)) {
            return "polygon";
        } else if (std::dynamic_pointer_cast<MultiPointGeometry>(geometry)) {
            return "multipoint";
        } else if (std::dynamic_pointer_cast<MultiLineGeometry>(geometry)) {
            return "multilinestring";
        } else if (std::dynamic_pointer_cast<MultiPolygonGeometry>(geometry)) {
            return "multipolygon";
        } else if (std::dynamic_pointer_cast<MultiGeometry>(geometry)) {
            return "multigeometry";
        } else if (std::dynamic_pointer_cast<Geometry>(geometry)) {
            return "geometry";
        }
        return "unknown";
    }
    
    std::size_t StyleSelectorContext::GetGeometryVerticesCount(const std::shared_ptr<Geometry>& geometry) {
        if (std::dynamic_pointer_cast<PointGeometry>(geometry)) {
            return 1;
        } else if (auto lineGeometry = std::dynamic_pointer_cast<LineGeometry>(geometry)) {
            return lineGeometry->getPoses().size();
        } else if (auto polygonGeometry = std::dynamic_pointer_cast<PolygonGeometry>(geometry)) {
            return std::accumulate(polygonGeometry->getHoles().begin(), polygonGeometry->getHoles().end(), polygonGeometry->getPoses().size(), [](std::size_t size, const std::vector<MapPos>& ring) { return size + ring.size(); });
        } else if (auto multiGeometry = std::dynamic_pointer_cast<MultiGeometry>(geometry)) {
            std::size_t count = 0;
            for (int i = 0; i < multiGeometry->getGeometryCount(); i++) {
                count += GetGeometryVerticesCount(multiGeometry->getGeometry(i));
            }
            return count;
        }
        return 0;
    }

}

#endif
