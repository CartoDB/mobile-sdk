/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_OGRVECTORDATASOURCE_H_
#define _CARTO_OGRVECTORDATASOURCE_H_

#ifdef _CARTO_GDAL_SUPPORT

#include "core/MapBounds.h"
#include "core/Variant.h"
#include "datasources/VectorDataSource.h"
#include "datasources/OGRVectorDataBase.h"

#include <map>
#include <vector>

class OGRGeometry;
class OGRFeature;
class OGRLayer;
class OGRSpatialReference;
class OGRCoordinateTransformation;

namespace carto {
    class Geometry;
    class GeometrySimplifier;
    class StyleSelector;
    class ViewState;
    class VectorElement;
    
    namespace OGRFieldType {
        /**
         * Supported types of OGR data source fields.
         */
        enum OGRFieldType {
            /**
             * Unknown/unsupported field type.
             */
            OGR_FIELD_TYPE_UNKNOWN,
            /**
             * Integer field.
             */
            OGR_FIELD_TYPE_INTEGER,
            /**
             * Real-valued or floating point field.
             */
            OGR_FIELD_TYPE_REAL,
            /**
             * String field.
             */
            OGR_FIELD_TYPE_STRING,
            /**
             * Date field.
             */
            OGR_FIELD_TYPE_DATE,
            /**
             * Time field.
             */
            OGR_FIELD_TYPE_TIME,
            /**
             * Date/Time field.
             */
            OGR_FIELD_TYPE_DATETIME
        };
    }

    /**
     * High-level vector element data source that supports various OGR data formats.
     * Shapefiles, GeoJSON, KML files can be used using this data source.
     */
    class OGRVectorDataSource : public VectorDataSource {
    public:
        /**
         * Constructs a new OGR data source given style selector and OGR supported data file.
         * If the file contains multiple layers, only the first layer is used. Note: data source is opened in read-only mode.
         * @param projection The projection to use for data coordinates. Note that if actual data is in different coordinates, coordinates will be re-projected.
         * @param styleSelector The style selector to use when loading geometry from OGR file.
         * @param fileName The full path of the data file
         * @throws std::exception If the file could not be opened.
         */
        OGRVectorDataSource(const std::shared_ptr<Projection>& projection, const std::shared_ptr<StyleSelector>& styleSelector, const std::string& fileName);

        /**
         * Constructs a new OGR data source given style selector, OGR database and layer index in database.
         * Note: if the database is opened in writable mode, the layer can be modified.
         * @param projection The projection to use for data coordinates. Note that if actual data is in different coordinates, coordinates will be re-projected.
         * @param styleSelector The style selector to use when loading geometry from OGR file.
         * @param dataBase The database for this data source
         * @param layerIndex The layer to use (layer names are available from database)
         */
        OGRVectorDataSource(const std::shared_ptr<Projection>& projection, const std::shared_ptr<StyleSelector>& styleSelector, const std::shared_ptr<OGRVectorDataBase>& dataBase, int layerIndex);
        virtual ~OGRVectorDataSource();

        /**
         * Returns the current code page used for decoding DBF text data. The default is ISO-8859-1.
         * @return The current code page for text data.
         */
        std::string getCodePage() const;
        /**
         * Specifies the code page to use for decoding DBF text data.
         * @param codePage The codepage to use (for example, "CP1254", or any other iconv encoding)
         */
        void setCodePage(const std::string& codePage);

        /**
         * Returns the active geometry simplifier of the data source.
         * @return The current geometry simplifier (can be null)
         */
        std::shared_ptr<GeometrySimplifier> getGeometrySimplifier() const;
        /**
         * Attaches geometry simplifier to the data source. If the specified simplifier is null, detaches any active simplifier.
         * @param simplifier The new geometry simplifier to use (can be null).
         */
        void setGeometrySimplifier(const std::shared_ptr<GeometrySimplifier>& simplifier);
        
        /**
         * Returns the extent of this data source. Extent is the minimal bounding box encompassing all the elements.
         * @return The minimal bounding box for the elements.
         */
        MapBounds getDataExtent() const;
        
        /**
         * Returns the total feature count for this data source.
         * @return The feature count for the data source.
         */
        int getFeatureCount() const;
        
        /**
         * Returns the geometry type of the data source.
         * @return The geometry type of the data source (layer).
         */
        OGRGeometryType::OGRGeometryType getGeometryType() const;
        
        /**
         * Adds a vector element to the data source. The vector element's coordinates are expected to be
         * in the data source's projection's coordinate system. The vector element is not stored in actual
         * data source persistently until commited.
         * @param element The vector element to be added.
         */
        void add(const std::shared_ptr<VectorElement>& element);
        /**
         * Removes a vector element from the data source. The vector element is not removed persistently from actual
         * data source until commited.
         * @param element The vector element to be removed.
         * @return True if the vector element existed in the data source.
         */
        bool remove(const std::shared_ptr<VectorElement>& element);

        /**
         * Returns whether all changes to data source elements have been committed.
         * @return true if all changes have been committed, false otherwise.
         */
        bool isCommitted() const;
        /**
         * Commits all changes to actual persistent data source.
         * @return The committed vector elements.
         */
        std::vector<std::shared_ptr<VectorElement> > commit();
        /**
         * Rolls back all local changes to the data source.
         * @return The rolled back vector elements.
         */
        std::vector<std::shared_ptr<VectorElement> > rollback();
        
        /**
         * Returns the list of existing fields of the data source.
         * @return The names of the fields.
         */
        std::vector<std::string> getFieldNames() const;
        /**
         * Creates a new field with specified name and type.
         * @param name The name of the field (must be unique)
         * @param type The type of the field.
         * @param width The maximum formatting width (in characters) for string fields. Can be 0 for other field types.
         * @return True if the field was successfully created, false otherwise.
         */
        bool createField(const std::string& name, OGRFieldType::OGRFieldType type, int width);
        /**
         * Deletes an existing field at specified index.
         * @param index The index of the field to delete (starting from 0).
         * @return True if the field was successfully created, false otherwise.
         */
        bool deleteField(int index);
        
        virtual std::shared_ptr<VectorData> loadElements(const std::shared_ptr<CullState>& cullState);

        /**
         * Tests the data source capability.
         * @param capability The name of the capability (for example, "CreateFeature", "DeleteFeature", etc)
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
        virtual void notifyElementChanged(const std::shared_ptr<VectorElement>& element);        
        
    private:
        struct LayerSpatialReference;
        
        std::shared_ptr<Geometry> createGeometry(const OGRGeometry* poGeometry) const;
        
        std::shared_ptr<VectorElement> createVectorElement(const ViewState& viewState, const std::shared_ptr<Geometry>& geometry, const std::map<std::string, Variant>& metaData) const;
        
        std::shared_ptr<OGRGeometry> createOGRGeometry(const std::shared_ptr<Geometry>& geometry) const;

        std::shared_ptr<OGRFeature> createOGRFeature(const std::shared_ptr<VectorElement>& element) const;

        std::string _codePage;
        std::shared_ptr<StyleSelector> _styleSelector;
        std::shared_ptr<GeometrySimplifier> _geometrySimplifier;

        long long _localElementId;
        std::map<long long, std::shared_ptr<VectorElement> > _localElements;

        std::shared_ptr<OGRVectorDataBase> _dataBase;
        OGRLayer* _poLayer;
        std::shared_ptr<LayerSpatialReference> _poLayerSpatialRef;
    };
}

#endif

#endif
