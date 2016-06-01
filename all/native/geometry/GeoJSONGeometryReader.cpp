#include "GeoJSONGeometryReader.h"
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
