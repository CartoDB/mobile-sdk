#include "GeoJSONGeometryWriter.h"
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

namespace carto {

    GeoJSONGeometryWriter::GeoJSONGeometryWriter() :
        _sourceProjection(),
        _z(false),
        _mutex()
    {
    }

    std::shared_ptr<Projection> GeoJSONGeometryWriter::getSourceProjection() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _sourceProjection;
    }
    
    void GeoJSONGeometryWriter::setSourceProjection(const std::shared_ptr<Projection>& proj) {
        std::lock_guard<std::mutex> lock(_mutex);
        _sourceProjection = proj;
    }

    bool GeoJSONGeometryWriter::getZ() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _z;
    }

    void GeoJSONGeometryWriter::setZ(bool z) {
        std::lock_guard<std::mutex> lock(_mutex);
        _z = z;
    }

    std::string GeoJSONGeometryWriter::writeGeometry(const std::shared_ptr<Geometry>& geometry) const {
        if (!geometry) {
            throw NullArgumentException("Null geometry");
        }

        std::lock_guard<std::mutex> lock(_mutex);

        rapidjson::StringBuffer geoJSON;
        rapidjson::Writer<rapidjson::StringBuffer> writer(geoJSON);
        rapidjson::Document doc;
        writeGeometry(geometry, doc, doc.GetAllocator());
        doc.Accept(writer);
        return geoJSON.GetString();
    }

    std::string GeoJSONGeometryWriter::writeFeature(const std::shared_ptr<Feature>& feature) const {
        if (!feature) {
            throw NullArgumentException("Null feature");
        }

        std::lock_guard<std::mutex> lock(_mutex);

        rapidjson::StringBuffer geoJSON;
        rapidjson::Writer<rapidjson::StringBuffer> writer(geoJSON);
        rapidjson::Document doc;
        writeFeature(feature, doc, doc.GetAllocator());
        doc.Accept(writer);
        return geoJSON.GetString();
    }

    std::string GeoJSONGeometryWriter::writeFeatureCollection(const std::shared_ptr<FeatureCollection>& featureCollection) const {
        if (!featureCollection) {
            throw NullArgumentException("Null feature collection");
        }

        std::lock_guard<std::mutex> lock(_mutex);

        rapidjson::StringBuffer geoJSON;
        rapidjson::Writer<rapidjson::StringBuffer> writer(geoJSON);
        rapidjson::Document doc;
        writeFeatureCollection(featureCollection, doc, doc.GetAllocator());
        doc.Accept(writer);
        return geoJSON.GetString();
    }

    void GeoJSONGeometryWriter::writeFeatureCollection(const std::shared_ptr<FeatureCollection>& featureCollection, rapidjson::Value& value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator) const {
        value.SetObject();
        value.AddMember("type", "FeatureCollection", allocator);
        rapidjson::Value featuresValue;
        featuresValue.SetArray();
        for (int i = 0; i < featureCollection->getFeatureCount(); i++) {
            featuresValue.PushBack(rapidjson::Value(), allocator);
            writeFeature(featureCollection->getFeature(i), featuresValue[i], allocator);
        }
        value.AddMember("features", featuresValue.Move(), allocator);
    }

    void GeoJSONGeometryWriter::writeFeature(const std::shared_ptr<Feature>& feature, rapidjson::Value& value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator) const {
        value.SetObject();
        value.AddMember("type", "Feature", allocator);
        value.AddMember("geometry", rapidjson::Value(), allocator);
        writeGeometry(feature->getGeometry(), value["geometry"], allocator);
        value.AddMember("properties", rapidjson::Value(), allocator);
        writeProperties(feature->getProperties(), value["properties"], allocator);
    }

    void GeoJSONGeometryWriter::writeGeometry(const std::shared_ptr<Geometry>& geometry, rapidjson::Value& value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator) const {
        value.SetObject();
        if (auto point = std::dynamic_pointer_cast<PointGeometry>(geometry)) {
            value.AddMember("type", "Point", allocator);
            value.AddMember("coordinates", rapidjson::Value(), allocator);
            writePoint(point->getPos(), value["coordinates"], allocator);
        } else if (auto line = std::dynamic_pointer_cast<LineGeometry>(geometry)) {
            value.AddMember("type", "LineString", allocator);
            value.AddMember("coordinates", rapidjson::Value(), allocator);
            writeRing(line->getPoses(), value["coordinates"], allocator);
        } else if (auto polygon = std::dynamic_pointer_cast<PolygonGeometry>(geometry)) {
            value.AddMember("type", "Polygon", allocator);
            value.AddMember("coordinates", rapidjson::Value(), allocator);
            writeRings(polygon->getRings(), value["coordinates"], allocator);
        } else if (auto multiPoint = std::dynamic_pointer_cast<MultiPointGeometry>(geometry)) {
            value.AddMember("type", "MultiPoint", allocator);
            value.AddMember("coordinates", rapidjson::Value(), allocator);
            rapidjson::Value& coordinates = value["coordinates"];
            coordinates.SetArray();
            for (int i = 0; i < multiPoint->getGeometryCount(); i++) {
                coordinates.PushBack(rapidjson::Value(), allocator);
                writePoint(multiPoint->getGeometry(i)->getPos(), coordinates[i], allocator);
            }
        } else if (auto multiLine = std::dynamic_pointer_cast<MultiLineGeometry>(geometry)) {
            value.AddMember("type", "MultiLineString", allocator);
            value.AddMember("coordinates", rapidjson::Value(), allocator);
            rapidjson::Value& coordinates = value["coordinates"];
            coordinates.SetArray();
            for (int i = 0; i < multiLine->getGeometryCount(); i++) {
                coordinates.PushBack(rapidjson::Value(), allocator);
                writeRing(multiLine->getGeometry(i)->getPoses(), coordinates[i], allocator);
            }
        } else if (auto multiPolygon = std::dynamic_pointer_cast<MultiPolygonGeometry>(geometry)) {
            value.AddMember("type", "MultiPolygon", allocator);
            value.AddMember("coordinates", rapidjson::Value(), allocator);
            rapidjson::Value& coordinates = value["coordinates"];
            coordinates.SetArray();
            for (int i = 0; i < multiPolygon->getGeometryCount(); i++) {
                coordinates.PushBack(rapidjson::Value(), allocator);
                writeRings(multiPolygon->getGeometry(i)->getRings(), coordinates[i], allocator);
            }
        } else if (auto multiGeometry = std::dynamic_pointer_cast<MultiGeometry>(geometry)) {
            value.AddMember("type", "GeometryCollection", allocator);
            value.AddMember("geometries", rapidjson::Value(), allocator);
            rapidjson::Value& geometries = value["geometries"];
            geometries.SetArray();
            for (int i = 0; i < multiGeometry->getGeometryCount(); i++) {
                geometries.PushBack(rapidjson::Value(), allocator);
                writeGeometry(multiGeometry->getGeometry(i), geometries[i], allocator);
            }
        } else {
            throw GenerateException("Unsupported geometry type");
        }
    }

    void GeoJSONGeometryWriter::writeProperties(const Variant& properties, rapidjson::Value& value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator) const {
        rapidjson::Document propertiesDoc;
        if (propertiesDoc.Parse<rapidjson::kParseDefaultFlags>(properties.toString().c_str()).HasParseError()) {
            throw GenerateException("Failed to read properties");
        }

        value.CopyFrom(propertiesDoc, allocator);
    }

    void GeoJSONGeometryWriter::writePoint(const MapPos& pos, rapidjson::Value& value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator) const {
        MapPos mapPos(pos);
        if (_sourceProjection) {
            mapPos = _sourceProjection->toWgs84(mapPos);
        }
        value.SetArray();
        value.PushBack(rapidjson::Value(mapPos.getX()), allocator);
        value.PushBack(rapidjson::Value(mapPos.getY()), allocator);
        if (_z) {
            value.PushBack(rapidjson::Value(mapPos.getZ()), allocator);
        }
    }

    void GeoJSONGeometryWriter::writeRing(const std::vector<MapPos>& ring, rapidjson::Value& value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator) const {
        value.SetArray();
        for (rapidjson::SizeType i = 0; i < ring.size(); i++) {
            value.PushBack(rapidjson::Value(), allocator);
            writePoint(ring[i], value[i], allocator);
        }
    }

    void GeoJSONGeometryWriter::writeRings(const std::vector<std::vector<MapPos> >& rings, rapidjson::Value& value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator) const {
        value.SetArray();
        for (rapidjson::SizeType i = 0; i < rings.size(); i++) {
            value.PushBack(rapidjson::Value(), allocator);
            writeRing(rings[i], value[i], allocator);
        }
    }

}
