/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOJSONGEOMETRYREADER_H_
#define _CARTO_GEOJSONGEOMETRYREADER_H_

#include "core/MapPos.h"
#include "core/Variant.h"

#include <memory>
#include <string>
#include <vector>
#include <mutex>

namespace rapidjson {
    class CrtAllocator;
    template <typename BaseAllocator> class MemoryPoolAllocator;
    template<typename CharType> struct UTF8;
    template <typename Encoding, typename Allocator> class GenericValue;
    typedef GenericValue<UTF8<char>, MemoryPoolAllocator<CrtAllocator> > Value;
}

namespace carto {
    class Feature;
    class FeatureCollection;
    class Geometry;
    class Projection;

    /**
     * A GeoJSON parser.
     * Parser supports Geometry, Feature and FeatureCollection inputs.
     */
    class GeoJSONGeometryReader {
    public:
        /**
         * Constructs a new GeoJSONGeometryReader object.
         */
        GeoJSONGeometryReader();
        
        /**
         * Returns the current target projection. If target projection is set, all geometry
         * coordinates will be converted from WGS84 to target projection coordinate system.
         * @return The current target projection or null.
         */
        std::shared_ptr<Projection> getTargetProjection() const;
        
        /**
         * Sets the current target projection. If target projection is set, all geometry
         * coordinates will be converted from WGS84 to target projection coordinate system.
         * @param proj The new target projection or null.
         */
        void setTargetProjection(const std::shared_ptr<Projection>& proj);

        /**
         * Reads geometry from the specified GeoJSON string.
         * @param geoJSON The GeoJSON string to read.
         * @return The geometry read from the string.
         * @throws std::runtime_error If string could not be parsed.
         */
        std::shared_ptr<Geometry> readGeometry(const std::string& geoJSON) const;

        /**
         * Reads feature from the specified GeoJSON string.
         * @param geoJSON The GeoJSON string to read.
         * @return The feature read from the string.
         * @throws std::runtime_error If string could not be parsed.
         */
        std::shared_ptr<Feature> readFeature(const std::string& geoJSON) const;

        /**
         * Reads feature collection from the specified GeoJSON string.
         * @param geoJSON The GeoJSON string to read.
         * @return The feature collection read from the string.
         * @throws std::runtime_error If string could not be parsed.
         */
        std::shared_ptr<FeatureCollection> readFeatureCollection(const std::string& geoJSON) const;

    private:
        std::shared_ptr<FeatureCollection> readFeatureCollection(const rapidjson::Value& value) const;
        std::shared_ptr<Feature> readFeature(const rapidjson::Value& value) const;
        std::shared_ptr<Geometry> readGeometry(const rapidjson::Value& value) const;
        Variant readProperties(const rapidjson::Value& value) const;
        MapPos readPoint(const rapidjson::Value& value) const;
        std::vector<MapPos> readRing(const rapidjson::Value& value) const;
        std::vector<std::vector<MapPos> > readRings(const rapidjson::Value& value) const;

        std::shared_ptr<Projection> _targetProjection;
        mutable std::mutex _mutex;
    };

}

#endif
