#include "GeometryCollection.h"
#include "datasources/VectorDataSource.h"
#include "geometry/MultiGeometry.h"
#include "renderers/drawdatas/GeometryCollectionDrawData.h"

namespace carto {

    GeometryCollection::GeometryCollection(const std::shared_ptr<MultiGeometry>& geometry, const std::shared_ptr<GeometryCollectionStyle>& style) :
        VectorElement(geometry),
        _style(style)
    {
        if (!style) {
            throw std::invalid_argument("Null style");
        }
    }

    GeometryCollection::~GeometryCollection() {
    }

    std::shared_ptr<MultiGeometry> GeometryCollection::getGeometry() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return std::static_pointer_cast<MultiGeometry>(_geometry);
    }

    void GeometryCollection::setGeometry(const std::shared_ptr<MultiGeometry>& geometry) {
        if (!geometry) {
            throw std::invalid_argument("Null geometry");
        }

        {
            std::lock_guard<std::mutex> lock(_mutex);
            _geometry = geometry;
        }
        notifyElementChanged();
    }

    std::shared_ptr<GeometryCollectionStyle> GeometryCollection::getStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _style;
    }

    void GeometryCollection::setStyle(const std::shared_ptr<GeometryCollectionStyle>& style) {
        if (!style) {
            throw std::invalid_argument("Null style");
        }

        {
            std::lock_guard<std::mutex> lock(_mutex);
            _style = style;
        }
        notifyElementChanged();
    }

    std::shared_ptr<GeometryCollectionDrawData> GeometryCollection::getDrawData() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _drawData;
    }

    void GeometryCollection::setDrawData(const std::shared_ptr<GeometryCollectionDrawData>& drawData) {
        std::lock_guard<std::mutex> lock(_mutex);
        _drawData = drawData;
    }

}
