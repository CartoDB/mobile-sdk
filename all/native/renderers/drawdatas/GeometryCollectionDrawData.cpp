#include "GeometryCollectionDrawData.h"
#include "geometry/PointGeometry.h"
#include "geometry/LineGeometry.h"
#include "geometry/PolygonGeometry.h"
#include "geometry/MultiGeometry.h"
#include "projections/Projection.h"
#include "renderers/drawdatas/PointDrawData.h"
#include "renderers/drawdatas/LineDrawData.h"
#include "renderers/drawdatas/PolygonDrawData.h"
#include "styles/GeometryCollectionStyle.h"
#include "vectorelements/GeometryCollection.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <utility>
#include <algorithm>

namespace carto {
    
    GeometryCollectionDrawData::GeometryCollectionDrawData(const MultiGeometry& geometry, const GeometryCollectionStyle& style, const Projection& projection) :
        VectorElementDrawData(Color()),
        _drawDatas()
    {
        addDrawData(geometry, style, projection);
    }
    
    GeometryCollectionDrawData::~GeometryCollectionDrawData() {
    }
    
    const std::vector<std::shared_ptr<VectorElementDrawData> >& GeometryCollectionDrawData::getDrawDatas() const {
        return _drawDatas;
    }
    
    void GeometryCollectionDrawData::offsetHorizontally(double offset) {
        for (const std::shared_ptr<VectorElementDrawData>& drawData : _drawDatas) {
            drawData->offsetHorizontally(offset);
        }
        setIsOffset(true);
    }

    void GeometryCollectionDrawData::addDrawData(const Geometry &geometry, const GeometryCollectionStyle &style, const Projection& projection) {
        if (auto pointGeometry = dynamic_cast<const PointGeometry*>(&geometry)) {
            if (style.getPointStyle()) {
                _drawDatas.push_back(std::make_shared<PointDrawData>(*pointGeometry, *style.getPointStyle(), projection));
            }
        } else if (auto lineGeometry = dynamic_cast<const LineGeometry*>(&geometry)) {
            if (style.getLineStyle()) {
                _drawDatas.push_back(std::make_shared<LineDrawData>(*lineGeometry, *style.getLineStyle(), projection));
            }
        } else if (auto polygonGeometry = dynamic_cast<const PolygonGeometry*>(&geometry)) {
            if (style.getPolygonStyle()) {
                _drawDatas.push_back(std::make_shared<PolygonDrawData>(*polygonGeometry, *style.getPolygonStyle(), projection));
            }
        } else if (auto geomCollection = dynamic_cast<const MultiGeometry*>(&geometry)) {
            for (int i = 0; i < geomCollection->getGeometryCount(); i++) {
                addDrawData(*geomCollection->getGeometry(i), style, projection);
            }
        }
    }
    
}
