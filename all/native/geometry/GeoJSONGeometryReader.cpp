#include "GeoJSONGeometryReader.h"
#include "Feature.h"
#include "FeatureCollection.h"
#include "Geometry.h"
#include "PointGeometry.h"
#include "LineGeometry.h"
#include "PolygonGeometry.h"
#include "MultiGeometry.h"
#include "MultiPointGeometry.h"
#include "MultiLineGeometry.h"
#include "MultiPolygonGeometry.h"
#include "projections/Projection.h"
#include "utils/Log.h"

#include <stdexcept>

#include <rapidjson/rapidjson.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>

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
            Log::Error("GeoJSONGeometryReader::readGeometry: Error while parsing package list");
            return std::shared_ptr<Geometry>();
        }
        try {
            return readGeometry(geometryDoc);
        } catch (const std::exception& ex) {
            Log::Errorf("GeoJSONGeometryReader::readGeometry: Failed to read geometry: %s", ex.what());
        }
        return std::shared_ptr<Geometry>();
    }

    std::shared_ptr<Feature> GeoJSONGeometryReader::readFeature(const std::string& geoJSON) const {
        std::lock_guard<std::mutex> lock(_mutex);

        rapidjson::Document featureDoc;
        if (featureDoc.Parse<rapidjson::kParseDefaultFlags>(geoJSON.c_str()).HasParseError()) {
            Log::Error("GeoJSONGeometryReader::readFeature: Error while parsing package list");
            return std::shared_ptr<Feature>();
        }
        try {
            return readFeature(featureDoc);
        } catch (const std::exception& ex) {
            Log::Errorf("GeoJSONGeometryReader::readFeature: Failed to read geometry: %s", ex.what());
        }
        return std::shared_ptr<Feature>();
    }

    std::shared_ptr<FeatureCollection> GeoJSONGeometryReader::readFeatureCollection(const std::string& geoJSON) const {
        std::lock_guard<std::mutex> lock(_mutex);

        rapidjson::Document featureCollectionDoc;
        if (featureCollectionDoc.Parse<rapidjson::kParseDefaultFlags>(geoJSON.c_str()).HasParseError()) {
            Log::Error("GeoJSONGeometryReader::readFeatureCollection: Error while parsing package list");
            return std::shared_ptr<FeatureCollection>();
        }
        try {
            return readFeatureCollection(featureCollectionDoc);
        } catch (const std::exception& ex) {
            Log::Errorf("GeoJSONGeometryReader::readFeatureCollection: Failed to read geometry: %s", ex.what());
        }
        return std::shared_ptr<FeatureCollection>();
    }

    std::shared_ptr<FeatureCollection> GeoJSONGeometryReader::readFeatureCollection(const rapidjson::Value& value) const {
        if (!value.IsObject()) {
            throw std::runtime_error("Wrong JSON type for feature collection");
        }

        std::string type = value["type"].GetString();
        if (type != "FeatureCollection") {
             throw std::runtime_error("Illegal type for the feature collection");
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
            throw std::runtime_error("Wrong JSON type for feature");
        }

        std::string type = value["type"].GetString();
        if (type != "Feature") {
             throw std::runtime_error("Illegal type for the feature");
        }

        std::shared_ptr<Geometry> geometry = readGeometry(value["geometry"]);
        Variant properties = readProperties(value["properties"]);
        return std::make_shared<Feature>(geometry, properties);
    }

    std::shared_ptr<Geometry> GeoJSONGeometryReader::readGeometry(const rapidjson::Value& value) const {
        if (!value.IsObject()) {
            throw std::runtime_error("Wrong JSON type for geometry");
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
                throw std::runtime_error("Wrong JSON type for coordinates");
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
                throw std::runtime_error("Wrong JSON type for coordinates");
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
                throw std::runtime_error("Wrong JSON type for coordinates");
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
                throw std::runtime_error("Wrong JSON type for geometries");
            }
            std::vector<std::shared_ptr<Geometry> > geometryList;
            geometryList.reserve(geometries.Size());
            for (rapidjson::SizeType i = 0; i < geometries.Size(); i++) {
                geometryList.push_back(readGeometry(geometries[i]));
            }
            return std::make_shared<MultiGeometry>(geometryList);
        } else {
            throw std::runtime_error("Unsupported geometry type: " + type);
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
            throw std::runtime_error("Wrong JSON type for coordinates");
        }
        if (value.Size() < 2) {
            throw std::runtime_error("Too few components in coordinates");
        }
        MapPos mapPos(value[0].GetDouble(), value[1].GetDouble(), value.Size() > 2 ? value[2].GetDouble() : 0);
        if (_targetProjection) {
            mapPos = _targetProjection->fromWgs84(mapPos);
        }
        return mapPos;
    }

    std::vector<MapPos> GeoJSONGeometryReader::readRing(const rapidjson::Value& value) const {
        if (!value.IsArray()) {
            throw std::runtime_error("Wrong JSON type for coordinates");
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
            throw std::runtime_error("Wrong JSON type for coordinates");
        }
        std::vector<std::vector<MapPos> > rings;
        rings.reserve(value.Size());
        for (rapidjson::SizeType i = 0; i < value.Size(); i++) {
            rings.push_back(readRing(value[i]));
        }
        return rings;
    }

}
