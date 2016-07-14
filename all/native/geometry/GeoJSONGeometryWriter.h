/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOJSONGEOMETRYWRITER_H_
#define _CARTO_GEOJSONGEOMETRYWRITER_H_

#include "core/MapPos.h"
#include "core/Variant.h"

#include <memory>
#include <mutex>
#include <cstddef>
#include <vector>
#include <stack>

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
     * A GeoJSON writer. Generates human-readable GeoJSON representation of the geometry, feature or feature collection.
     * Supports both 2D and 3D coordinate serialization.
     */
    class GeoJSONGeometryWriter {
    public:
        /**
         * Constructs a new GeoJSONGeometryWriter object with default settings.
         */
        GeoJSONGeometryWriter();

        /**
         * Returns the current source projection. If source projection is set, all geometry
         * coordinates will be converted from given coordinate system to WGS84.
         * @return The current source projection or null.
         */
        std::shared_ptr<Projection> getSourceProjection() const;
        
        /**
         * Sets the current source projection. If source projection is set, all geometry
         * coordinates will be converted from given coordinate system to WGS84.
         * @param proj The new source projection or null.
         */
        void setSourceProjection(const std::shared_ptr<Projection>& proj);

        /**
         * Returns the state of Z coordinate serialization.
         * @return True if Z coordinate is stored in GeoJSON output, false otherwise. The default is false.
         */
        bool getZ() const;
        /**
         * Sets the state of Z coordinate serialization.
         * @param z True when Z coordinate should be serialized, false otherwise.
         */
        void setZ(bool z);

        /**
         * Creates a GeoJSON string corresponding to the specified geometry.
         * @param geometry The geometry to write.
         * @return The corresponding GeoJSON string.
         * @throws std::runtime_error If string could not be generated.
         */
        std::string writeGeometry(const std::shared_ptr<Geometry>& geometry) const;

        /**
         * Creates a GeoJSON string corresponding to the specified feature.
         * @param feature The feature to write.
         * @return The corresponding GeoJSON string.
         * @throws std::runtime_error If string could not be generated.
         */
        std::string writeFeature(const std::shared_ptr<Feature>& feature) const;

        /**
         * Creates a GeoJSON string corresponding to the specified feature collection.
         * @param featureCollection The feature collection to write.
         * @return The corresponding GeoJSON string.
         * @throws std::runtime_error If string could not be generated.
         */
        std::string writeFeatureCollection(const std::shared_ptr<FeatureCollection>& featureCollection) const;

    private:
        void writeFeatureCollection(const std::shared_ptr<FeatureCollection>& featureCollection, rapidjson::Value& value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator) const;
        void writeFeature(const std::shared_ptr<Feature>& feature, rapidjson::Value& value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator) const;
        void writeGeometry(const std::shared_ptr<Geometry>& geometry, rapidjson::Value& value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator) const;
        void writeProperties(const Variant& properties, rapidjson::Value& value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator) const;
        void writePoint(const MapPos& pos, rapidjson::Value& value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator) const;
        void writeRing(const std::vector<MapPos>& ring, rapidjson::Value& value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator) const;
        void writeRings(const std::vector<std::vector<MapPos> >& rings, rapidjson::Value& value, rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator>& allocator) const;

        std::shared_ptr<Projection> _sourceProjection;
        bool _z;

        mutable std::mutex _mutex;
    };

}

#endif
