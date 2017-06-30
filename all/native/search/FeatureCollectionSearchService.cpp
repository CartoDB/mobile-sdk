#ifdef _CARTO_SEARCH_SUPPORT

#include "FeatureCollectionSearchService.h"
#include "core/MapPos.h"
#include "components/Exceptions.h"
#include "geometry/Geometry.h"
#include "geometry/PointGeometry.h"
#include "geometry/LineGeometry.h"
#include "geometry/PolygonGeometry.h"
#include "geometry/MultiPointGeometry.h"
#include "geometry/MultiLineGeometry.h"
#include "geometry/MultiPolygonGeometry.h"
#include "geometry/MultiGeometry.h"
#include "geometry/Feature.h"
#include "geometry/FeatureCollection.h"
#include "search/query/QueryContext.h"
#include "search/query/QueryExpressionParser.h"
#include "projections/Projection.h"
#include "projections/EPSG3857.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include <limits>
#include <regex>
#include <algorithm>
#include <numeric>

namespace {

    typedef boost::geometry::model::d2::point_xy<double> BoostPointType;
    typedef boost::geometry::model::linestring<BoostPointType> BoostLinestringType;
    typedef boost::geometry::model::polygon<BoostPointType> BoostPolygonType;
    typedef boost::variant<BoostPointType, BoostLinestringType, BoostPolygonType> BoostGeometryType;

    std::shared_ptr<carto::Geometry> convertToEPSG3857(const std::shared_ptr<carto::Geometry>& geometry, const std::shared_ptr<carto::Projection>& proj) {
        if (std::dynamic_pointer_cast<carto::EPSG3857>(proj)) {
            return geometry;
        }

        carto::EPSG3857 epsg3857;
        if (auto pointGeometry = std::dynamic_pointer_cast<carto::PointGeometry>(geometry)) {
            carto::MapPos mapPos = epsg3857.fromWgs84(proj->toWgs84(pointGeometry->getPos()));
            return std::make_shared<carto::PointGeometry>(mapPos);
        } else if (auto lineGeometry = std::dynamic_pointer_cast<carto::LineGeometry>(geometry)) {
            std::vector<carto::MapPos> mapPoses = lineGeometry->getPoses();
            std::for_each(mapPoses.begin(), mapPoses.end(), [&epsg3857, &proj](carto::MapPos& mapPos) { mapPos = epsg3857.fromWgs84(proj->toWgs84(mapPos)); });
            return std::make_shared<carto::LineGeometry>(mapPoses);
        } else if (auto polygonGeometry = std::dynamic_pointer_cast<carto::PolygonGeometry>(geometry)) {
            std::vector<std::vector<carto::MapPos> > rings = polygonGeometry->getRings();
            for (std::vector<carto::MapPos>& ring : rings) {
                std::for_each(ring.begin(), ring.end(), [&epsg3857, &proj](carto::MapPos& mapPos) { mapPos = epsg3857.fromWgs84(proj->toWgs84(mapPos)); });
            }
            return std::make_shared<carto::PolygonGeometry>(rings);
        } else if (auto multiGeometry = std::dynamic_pointer_cast<carto::MultiGeometry>(geometry)) {
            std::vector<std::shared_ptr<carto::Geometry> > geometries;
            for (int i = 0; i < multiGeometry->getGeometryCount(); i++) {
                geometries.push_back(convertToEPSG3857(multiGeometry->getGeometry(i), proj));
            }
            return std::make_shared<carto::MultiGeometry>(geometries);
        } else {
            throw carto::GenericException("Unsupported geometry type");
        }
    }

    BoostGeometryType convertToBoostGeometry(const std::shared_ptr<carto::Geometry>& geometry) {
        if (auto pointGeometry = std::dynamic_pointer_cast<carto::PointGeometry>(geometry)) {
            carto::MapPos mapPos = pointGeometry->getPos();
            BoostPointType boostPoint(mapPos.getX(), mapPos.getY());
            return boostPoint;
        } else if (auto lineGeometry = std::dynamic_pointer_cast<carto::LineGeometry>(geometry)) {
            const std::vector<carto::MapPos>& mapPoses = lineGeometry->getPoses();
            BoostLinestringType boostLinestring;
            std::for_each(mapPoses.begin(), mapPoses.end(), [&boostLinestring](const carto::MapPos& mapPos) { boostLinestring.push_back(BoostPointType(mapPos.getX(), mapPos.getY())); });
            return boostLinestring;
        } else if (auto polygonGeometry = std::dynamic_pointer_cast<carto::PolygonGeometry>(geometry)) {
            const std::vector<std::vector<carto::MapPos> >& rings = polygonGeometry->getRings();
            BoostPolygonType boostPolygon;
            for (std::size_t i = 0; i < rings.size(); i++) {
                BoostPolygonType::ring_type boostRing;
                std::for_each(rings[i].begin(), rings[i].end(), [&boostRing](const carto::MapPos& mapPos) { boostRing.push_back(BoostPointType(mapPos.getX(), mapPos.getY())); });
                if (i == 0) {
                    boostPolygon.outer() = std::move(boostRing);
                } else {
                    boostPolygon.inners().push_back(std::move(boostRing));
                }
            }
            return boostPolygon;
        } else {
            throw carto::GenericException("Unsupported geometry type");
        }
    }

    double calculateDistance(const std::shared_ptr<carto::Geometry>& geometry1, const std::shared_ptr<carto::Geometry>& geometry2) {
        if (auto multiGeometry1 = std::dynamic_pointer_cast<carto::MultiGeometry>(geometry1)) {
            double dist = std::numeric_limits<double>::infinity();
            for (int i = 0; i < multiGeometry1->getGeometryCount(); i++) {
                dist = std::min(dist, calculateDistance(multiGeometry1->getGeometry(i), geometry2));
            }
            return dist;
        }

        if (auto multiGeometry2 = std::dynamic_pointer_cast<carto::MultiGeometry>(geometry2)) {
            double dist = std::numeric_limits<double>::infinity();
            for (int i = 0; i < multiGeometry2->getGeometryCount(); i++) {
                dist = std::min(dist, calculateDistance(geometry1, multiGeometry2->getGeometry(i)));
            }
            return dist;
        }

        return boost::geometry::distance(convertToBoostGeometry(geometry1), convertToBoostGeometry(geometry2));
    }

    bool matchRegexFilter(const carto::Variant& variant, const std::regex& re) {
        std::string str;
        switch (variant.getType()) {
        case carto::VariantType::VARIANT_TYPE_NULL:
            return false;
        case carto::VariantType::VARIANT_TYPE_STRING:
        case carto::VariantType::VARIANT_TYPE_BOOL:
        case carto::VariantType::VARIANT_TYPE_INTEGER:
        case carto::VariantType::VARIANT_TYPE_DOUBLE:
            return std::regex_match(variant.getString(), re);
        case carto::VariantType::VARIANT_TYPE_ARRAY:
            for (int i = 0; i < variant.getArraySize(); i++) {
                if (matchRegexFilter(variant.getArrayElement(i), re)) {
                    return true;
                }
            }
            return false;
        case carto::VariantType::VARIANT_TYPE_OBJECT:
            for (const std::string& key : variant.getObjectKeys()) {
                if (matchRegexFilter(variant.getObjectElement(key), re)) {
                    return true;
                }
            }
            return false;
        }
        return false;
    }

    class SearchQueryContext : public carto::QueryContext {
    public:
        explicit SearchQueryContext(const std::shared_ptr<carto::Geometry>& geometry, const carto::Variant& variant) : _geometry(geometry), _variant(variant) { }
        virtual ~SearchQueryContext() { }

        virtual bool getVariable(const std::string& name, carto::Variant& value) const {
            if (name == "geometry::type") {
                value = carto::Variant(GetGeometryType(_geometry));
                return true;
            }

            if (name == "geometry::vertices") {
                value = carto::Variant(static_cast<long long>(GetGeometryVerticesCount(_geometry)));
                return true;
            }

            switch (_variant.getType()) {
            case carto::VariantType::VARIANT_TYPE_OBJECT:
                if (_variant.containsObjectKey(name)) {
                    value = _variant.getObjectElement(name);
                    return true;
                }
                return false;
            case carto::VariantType::VARIANT_TYPE_ARRAY:
                return false;
            default:
                if (name == "value") {
                    value = _variant;
                    return true;
                }
                return false;
            }
        }

    private:
        static std::string GetGeometryType(const std::shared_ptr<carto::Geometry> &geometry) {
            if (std::dynamic_pointer_cast<carto::PointGeometry>(geometry)) {
                return "point";
            } else if (std::dynamic_pointer_cast<carto::LineGeometry>(geometry)) {
                return "linestring";
            } else if (std::dynamic_pointer_cast<carto::PolygonGeometry>(geometry)) {
                return "polygon";
            } else if (std::dynamic_pointer_cast<carto::MultiPointGeometry>(geometry)) {
                return "multipoint";
            } else if (std::dynamic_pointer_cast<carto::MultiLineGeometry>(geometry)) {
                return "multilinestring";
            } else if (std::dynamic_pointer_cast<carto::MultiPolygonGeometry>(geometry)) {
                return "multipolygon";
            } else if (std::dynamic_pointer_cast<carto::MultiGeometry>(geometry)) {
                return "multigeometry";
            } else if (std::dynamic_pointer_cast<carto::Geometry>(geometry)) {
                return "geometry";
            }
            return "unknown";
        }
        
        static std::size_t GetGeometryVerticesCount(const std::shared_ptr<carto::Geometry>& geometry) {
            if (std::dynamic_pointer_cast<carto::PointGeometry>(geometry)) {
                return 1;
            } else if (auto lineGeometry = std::dynamic_pointer_cast<carto::LineGeometry>(geometry)) {
                return lineGeometry->getPoses().size();
            } else if (auto polygonGeometry = std::dynamic_pointer_cast<carto::PolygonGeometry>(geometry)) {
                return std::accumulate(polygonGeometry->getHoles().begin(), polygonGeometry->getHoles().end(), polygonGeometry->getPoses().size(), [](std::size_t size, const std::vector<carto::MapPos>& ring) { return size + ring.size(); });
            } else if (auto multiGeometry = std::dynamic_pointer_cast<carto::MultiGeometry>(geometry)) {
                std::size_t count = 0;
                for (int i = 0; i < multiGeometry->getGeometryCount(); i++) {
                    count += GetGeometryVerticesCount(multiGeometry->getGeometry(i));
                }
                return count;
            }
            return 0;
        }

        const std::shared_ptr<carto::Geometry>& _geometry;
        const carto::Variant& _variant;
    };

}

namespace carto {

    FeatureCollectionSearchService::FeatureCollectionSearchService(const std::shared_ptr<Projection>& projection, const std::shared_ptr<FeatureCollection>& featureCollection) :
        _projection(projection),
        _featureCollection(featureCollection)
    {
        if (!projection) {
            throw NullArgumentException("Null projection");
        }
        if (!featureCollection) {
            throw NullArgumentException("Null featureCollection");
        }
    }

    FeatureCollectionSearchService::~FeatureCollectionSearchService() {
    }

    const std::shared_ptr<Projection>& FeatureCollectionSearchService::getProjection() const {
        return _projection;
    }

    const std::shared_ptr<FeatureCollection>& FeatureCollectionSearchService::getFeatureCollection() const {
        return _featureCollection;
    }

    std::shared_ptr<FeatureCollection> FeatureCollectionSearchService::findFeatures(const std::shared_ptr<SearchRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        boost::optional<std::regex> re;
        if (!request->getRegexFilter().empty()) {
            try {
                re = std::regex(request->getRegexFilter());
            }
            catch (const std::exception& ex) {
                Log::Errorf("FeatureCollectionSearchService::findFeatures: Exception when parsing regex: %s", ex.what());
                throw GenericException("Failed to parse regex", ex.what());
            }
        }

        std::shared_ptr<Geometry> geometry;
        double searchRadius = 0;
        if (request->getGeometry()) {
            MapPos centerPos = request->getProjection()->toWgs84(request->getGeometry()->getCenterPos());
            geometry = convertToEPSG3857(request->getGeometry(), request->getProjection());
            searchRadius = request->getSearchRadius() / std::cos(centerPos.getY() * Const::DEG_TO_RAD);
        }

        std::shared_ptr<QueryExpression> expr;
        if (!request->getFilterExpression().empty()) {
            expr = QueryExpressionParser::parse(request->getFilterExpression());
        }

        std::vector<std::shared_ptr<Feature> > features;
        for (int i = 0; i < _featureCollection->getFeatureCount(); i++) {
            const std::shared_ptr<Feature>& feature = _featureCollection->getFeature(i);

            if (re) {
                if (!matchRegexFilter(feature->getProperties(), *re)) {
                    continue;
                }
            }

            if (expr) {
                SearchQueryContext context(feature->getGeometry(), feature->getProperties());
                if (!expr->evaluate(context)) {
                    continue;
                }
            }

            if (geometry) {
                if (calculateDistance(convertToEPSG3857(feature->getGeometry(), _projection), geometry) > searchRadius) {
                    continue;
                }
            }

            features.push_back(feature);
        }
        return std::make_shared<FeatureCollection>(features);
    }

}

#endif
