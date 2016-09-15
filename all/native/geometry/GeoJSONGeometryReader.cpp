#include "GeoJSONGeometryReader.h"
#include "components/Exceptions.h"
#include "geometry/Feature.h"
#include "geometry/FeatureCollection.h"
#include "geometry/Geometry.h"
#include "geometry/PointGeometry.h"
#include "geometry/LineGeometry.h"
#include "geometry/PolygonGeometry.h"
#include "geometry/MultiGeometry.h"
#include "geometry/MultiPointGeometry.h"
#include "geometry/MultiLineGeometry.h"
#include "geometry/MultiPolygonGeometry.h"
#include "projections/Projection.h"
#include "utils/Log.h"

#include <stdexcept>

#include <rapidjson/rapidjson.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

namespace carto {

    GeoJSONGeometryReader::GeoJSONGeometryReader() :
        _targetProjection(), _mutex()
    {
    }
    
    std::shared_ptr<Projection> GeoJSONGeometryReader::getTargetProjection() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _targetProjection;
    }
    
    void GeoJSONGeometryReader::setTargetProjection(const std::shared_ptr<Projection>& proj) {
        std::lock_guard<std::mutex> lock(_mutex);
        _targetProjection = proj;
    }

    std::shared_ptr<Geometry> GeoJSONGeometryReader::readGeometry(const std::string& geoJSON) const {
        std::lock_guard<std::mutex> lock(_mutex);

        rapidjson::Document geometryDoc;
        if (geometryDoc.Parse<rapidjson::kParseDefaultFlags>(geoJSON.c_str()).HasParseError()) {
            std::string err = rapidjson::GetParseError_En(geometryDoc.GetParseError());
            throw ParseException(err, geoJSON, static_cast<int>(geometryDoc.GetErrorOffset()));
        }

        return readGeometry(geometryDoc);
    }

    std::shared_ptr<Feature> GeoJSONGeometryReader::readFeature(const std::string& geoJSON) const {
        std::lock_guard<std::mutex> lock(_mutex);

        rapidjson::Document featureDoc;
        if (featureDoc.Parse<rapidjson::kParseDefaultFlags>(geoJSON.c_str()).HasParseError()) {
            std::string err = rapidjson::GetParseError_En(featureDoc.GetParseError());
            throw ParseException(err, geoJSON, static_cast<int>(featureDoc.GetErrorOffset()));
        }

        return readFeature(featureDoc);
    }

    std::shared_ptr<FeatureCollection> GeoJSONGeometryReader::readFeatureCollection(const std::string& geoJSON) const {
        std::lock_guard<std::mutex> lock(_mutex);

        rapidjson::Document featureCollectionDoc;
        if (featureCollectionDoc.Parse<rapidjson::kParseDefaultFlags>(geoJSON.c_str()).HasParseError()) {
            std::string err = rapidjson::GetParseError_En(featureCollectionDoc.GetParseError());
            throw ParseException(err, geoJSON, static_cast<int>(featureCollectionDoc.GetErrorOffset()));
        }

        return readFeatureCollection(featureCollectionDoc);
    }

    std::shared_ptr<FeatureCollection> GeoJSONGeometryReader::readFeatureCollection(const rapidjson::Value& value) const {
        if (!value.IsObject()) {
            throw ParseException("Wrong JSON type for feature collection");
        }

        if (!value.HasMember("type")) {
            throw ParseException("Missing type information from feature collection");
        }
        std::string type = value["type"].GetString();
        if (type != "FeatureCollection") {
             throw ParseException("Illegal type for the feature collection");
        }

        const rapidjson::Value& featuresValue = value["features"];
        std::vector<std::shared_ptr<Feature> > features;
        features.reserve(featuresValue.Size());
        for (rapidjson::SizeType i = 0; i < featuresValue.Size(); i++) {
            std::shared_ptr<Feature> feature = readFeature(featuresValue[i]);
            features.push_back(feature);
        }
        return std::make_shared<FeatureCollection>(features);
    }

    std::shared_ptr<Feature> GeoJSONGeometryReader::readFeature(const rapidjson::Value& value) const {
        if (!value.IsObject()) {
            throw ParseException("Wrong JSON type for feature");
        }

        if (!value.HasMember("type")) {
            throw ParseException("Missing type information from feature");
        }
        std::string type = value["type"].GetString();
        if (type != "Feature") {
             throw ParseException("Illegal type for the feature");
        }

        std::shared_ptr<Geometry> geometry = readGeometry(value["geometry"]);
        Variant properties;
        if (value.HasMember("properties")) {
            properties = readProperties(value["properties"]);
        }
        return std::make_shared<Feature>(geometry, properties);
    }

    std::shared_ptr<Geometry> GeoJSONGeometryReader::readGeometry(const rapidjson::Value& value) const {
        if (!value.IsObject()) {
            throw ParseException("Wrong JSON type for geometry");
        }

        if (!value.HasMember("type")) {
            throw ParseException("Missing type information from geometry");
        }
        std::string type = value["type"].GetString();
        if (type == "Point") {
            return std::make_shared<PointGeometry>(readPoint(value["coordinates"]));
        } else if (type == "LineString") {
            return std::make_shared<LineGeometry>(readRing(value["coordinates"]));
        } else if (type == "Polygon") {
            return std::make_shared<PolygonGeometry>(readRings(value["coordinates"]));
        } else if (type == "MultiPoint") {
            const rapidjson::Value& coordinates = value["coordinates"];
            if (!coordinates.IsArray()) {
                throw ParseException("Wrong JSON type for coordinates");
            }
            std::vector<std::shared_ptr<PointGeometry> > points;
            points.reserve(coordinates.Size());
            for (rapidjson::SizeType i = 0; i < coordinates.Size(); i++) {
                points.push_back(std::make_shared<PointGeometry>(readPoint(coordinates[i])));
            }
            return std::make_shared<MultiPointGeometry>(points);
        } else if (type == "MultiLineString") {
            const rapidjson::Value& coordinates = value["coordinates"];
            if (!coordinates.IsArray()) {
                throw ParseException("Wrong JSON type for coordinates");
            }
            std::vector<std::shared_ptr<LineGeometry> > lines;
            lines.reserve(coordinates.Size());
            for (rapidjson::SizeType i = 0; i < coordinates.Size(); i++) {
                lines.push_back(std::make_shared<LineGeometry>(readRing(coordinates[i])));
            }
            return std::make_shared<MultiLineGeometry>(lines);
        } else if (type == "MultiPolygon") {
            const rapidjson::Value& coordinates = value["coordinates"];
            if (!coordinates.IsArray()) {
                throw ParseException("Wrong JSON type for coordinates");
            }
            std::vector<std::shared_ptr<PolygonGeometry> > polygons;
            polygons.reserve(coordinates.Size());
            for (rapidjson::SizeType i = 0; i < coordinates.Size(); i++) {
                polygons.push_back(std::make_shared<PolygonGeometry>(readRings(coordinates[i])));
            }
            return std::make_shared<MultiPolygonGeometry>(polygons);
        } else if (type == "GeometryCollection") {
            const rapidjson::Value& geometries = value["geometries"];
            if (!geometries.IsArray()) {
                throw ParseException("Wrong JSON type for geometries");
            }
            std::vector<std::shared_ptr<Geometry> > geometryList;
            geometryList.reserve(geometries.Size());
            for (rapidjson::SizeType i = 0; i < geometries.Size(); i++) {
                geometryList.push_back(readGeometry(geometries[i]));
            }
            return std::make_shared<MultiGeometry>(geometryList);
        } else {
            throw ParseException("Unsupported geometry type: " + type);
        }
    }

    Variant GeoJSONGeometryReader::readProperties(const rapidjson::Value& value) const {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        value.Accept(writer);
        return Variant::FromString(buffer.GetString());
    }

    MapPos GeoJSONGeometryReader::readPoint(const rapidjson::Value& value) const {
        if (!value.IsArray()) {
            throw ParseException("Wrong JSON type for coordinates");
        }
        if (value.Size() < 2) {
            throw ParseException("Too few components in coordinates");
        }
        MapPos mapPos(value[0].GetDouble(), value[1].GetDouble(), value.Size() > 2 ? value[2].GetDouble() : 0);
        if (_targetProjection) {
            mapPos = _targetProjection->fromWgs84(mapPos);
        }
        return mapPos;
    }

    std::vector<MapPos> GeoJSONGeometryReader::readRing(const rapidjson::Value& value) const {
        if (!value.IsArray()) {
            throw ParseException("Wrong JSON type for coordinates");
        }
        std::vector<MapPos> ring;
        ring.reserve(value.Size());
        for (rapidjson::SizeType i = 0; i < value.Size(); i++) {
            ring.push_back(readPoint(value[i]));
        }
        return ring;
    }

    std::vector<std::vector<MapPos> > GeoJSONGeometryReader::readRings(const rapidjson::Value& value) const {
        if (!value.IsArray()) {
            throw ParseException("Wrong JSON type for coordinates");
        }
        std::vector<std::vector<MapPos> > rings;
        rings.reserve(value.Size());
        for (rapidjson::SizeType i = 0; i < value.Size(); i++) {
            rings.push_back(readRing(value[i]));
        }
        return rings;
    }

}
