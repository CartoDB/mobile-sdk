/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOJSONGEOMETRYREADER_H_
#define _CARTO_GEOJSONGEOMETRYREADER_H_

#include "core/MapPos.h"

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
    class Geometry;
    class Projection;

    /**
     * A GeoJSON parser.
     * Only Geometry objects are supported, Features and FeatureCollections are not supported.
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
         * @return The geometry read from the string. Null if reading failed.
         */
        std::shared_ptr<Geometry> readGeometry(const std::string& geoJSON) const;

    private:
        std::shared_ptr<Geometry> readGeometry(const rapidjson::Value& value) const;
        MapPos readPoint(const rapidjson::Value& value) const;
        std::vector<MapPos> readRing(const rapidjson::Value& value) const;
        std::vector<std::vector<MapPos> > readRings(const rapidjson::Value& value) const;

        std::shared_ptr<Projection> _targetProjection;
        mutable std::mutex _mutex;
    };

}

#endif
