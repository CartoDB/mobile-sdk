/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_OGRVECTORDATABASE_H_
#define _CARTO_OGRVECTORDATABASE_H_

#ifdef _CARTO_GDAL_SUPPORT

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <mutex>

class OGRLayer;
class OGRDataSource;

namespace carto {
    
    namespace OGRGeometryType {
        /**
         * Supported OGR geometry types.
         */
        enum OGRGeometryType {
            /**
             * Unspecified/unsupported geometry type.
             */
            OGR_GEOMETRY_TYPE_UNKNOWN,
            /**
             * Point geometry.
             */
            OGR_GEOMETRY_TYPE_POINT,
            /**
             * Line geometry.
             */
            OGR_GEOMETRY_TYPE_LINE,
            /**
             * Polygon geometry.
             */
            OGR_GEOMETRY_TYPE_POLYGON,
            /**
             * Multipoint geometry.
             */
            OGR_GEOMETRY_TYPE_MULTIPOINT,
            /**
             * Multiline geometry.
             */
            OGR_GEOMETRY_TYPE_MULTILINE,
            /**
             * Multipolygon geometry.
             */
            OGR_GEOMETRY_TYPE_MULTIPOLYGON,
            /**
             * Geometry collection.
             */
            OGR_GEOMETRY_TYPE_GEOMETRYCOLLECTION
        };
    }
    
    /**
     * High-level vector element data source that supports various OGR data formats.
     * Shapefiles, GeoJSON, KML files can be used using this data source.
     */
    class OGRVectorDataBase {
    public:
        /**
         * Constructs a new OGR database instance given OGR supported data file.
         * @param fileName The full path of the data file
         * @param writable True when the database supports write operations. False when only read-only access is needed.
         * @throws std::exception If the the file could not be opened or created.
         */
        OGRVectorDataBase(const std::string& fileName, bool writable);
        virtual ~OGRVectorDataBase();
        
        /**
         * Returns the total layer count for this database.
         * @return The layer count for the database.
         */
        int getLayerCount() const;
        
        /**
         * Returns the names of all layers in the database.
         * @return The names of the layers contained in the database.
         */
        std::vector<std::string> getLayerNames() const;
        
        /**
         * Creates a new layer on top of existing layers.
         * @param name The name for the new layer.
         * @param epsgCode EPSG code for geometry data (3857, for example)
         * @param geometryType The geometry type for the layer.
         * @return True when the layer was successfully created, false otherwise.
         */
        bool createLayer(const std::string& name, int epsgCode, OGRGeometryType::OGRGeometryType geometryType);
        
        /**
         * Deletes an existing layer.
         * @param index The layer index to delete.
         * @return True when the layer was successfully deleted, false otherwise.
         */
        bool deleteLayer(int index);
        
        /**
         * Tests the database capability.
         * @param capability The name of the capability (for example, "CreateLayer", "DeleteLayer")
         * @return True when the capability exists, false otherwise.
         */
        bool testCapability(const std::string& capability) const;
        
        /**
         * Sets global OGR configuration option. This method can be used to redefine default locale, for example.
         * @param name The name of the option parameter to set ("SHAPE_ENCODING", for example)
         * @param value The value of the parameter.
         */
        static void SetConfigOption(const std::string& name, const std::string& value);
        /**
         * Returns the value of global OGR configuration option.
         * @param name The name of the the option parameter to read.
         * @return The value of the specified option parameter.
         */
        static std::string GetConfigOption(const std::string& name);
        
    protected:
        friend class OGRVectorDataSource;
        
    private:
        OGRDataSource* _poDS;
        std::vector<OGRLayer*> _poLayers;
        
        mutable std::mutex _mutex;
    };
}

#endif

#endif
