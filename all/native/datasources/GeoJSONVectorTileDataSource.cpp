#include "GeoJSONVectorTileDataSource.h"
#include "core/BinaryData.h"
#include "core/MapTile.h"
#include "components/Exceptions.h"
#include "geometry/FeatureCollection.h"
#include "geometry/GeoJSONGeometryWriter.h"
#include "projections/Projection.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <mbvtbuilder/MBVTTileBuilder.h>

#include <mapnikvt/mbvtpackage/MBVTPackage.pb.h>

namespace carto {

    GeoJSONVectorTileDataSource::GeoJSONVectorTileDataSource(int minZoom, int maxZoom) :
        TileDataSource(minZoom, maxZoom),
        _tileBuilder(std::make_unique<mbvtbuilder::MBVTTileBuilder>(minZoom, maxZoom)),
        _mutex()
    {
    }
    
    GeoJSONVectorTileDataSource::~GeoJSONVectorTileDataSource() {
    }

    int GeoJSONVectorTileDataSource::createLayer(const std::string& name) {
        int layerIndex = -1;
        try {
            std::lock_guard<std::mutex> lock(_mutex);
            layerIndex = _tileBuilder->createLayer(name);
        }
        catch (const std::exception& ex) {
            Log::Errorf("GeoJSONVectorTileDataSource::createLayer: Failed to create layer: %s", ex.what());
            throw GenericException("Failed to create layer", ex.what());
        }
        notifyTilesChanged(false);
        return layerIndex;
    }

    void GeoJSONVectorTileDataSource::setLayerGeoJSON(int layerIndex, const Variant& geoJSON) {
        try {
            std::lock_guard<std::mutex> lock(_mutex);
            _tileBuilder->clearLayer(layerIndex);
            _tileBuilder->importGeoJSONFeatureCollection(layerIndex, geoJSON.toPicoJSON());
        }
        catch (const std::exception& ex) {
            Log::Errorf("GeoJSONVectorTileDataSource::setLayerGeoJSON: Failed to update layer: %s", ex.what());
            throw GenericException("Failed to set layer contents", ex.what());
        }
        notifyTilesChanged(false);
    }
    
    void GeoJSONVectorTileDataSource::setLayerFeatureCollection(int layerIndex, const std::shared_ptr<Projection>& projection, const std::shared_ptr<FeatureCollection>& featureCollection) {
        if (!featureCollection) {
            throw NullArgumentException("Null featureCollection");
        }

        try {
            GeoJSONGeometryWriter geometryWriter;
            geometryWriter.setSourceProjection(projection);
            geometryWriter.setZ(false);
            picojson::value geoJSON;
            std::string err = picojson::parse(geoJSON, geometryWriter.writeFeatureCollection(featureCollection));
            if (!err.empty()) {
                throw GenericException("Error while serializing feature data", err);
            }

            std::lock_guard<std::mutex> lock(_mutex);
            _tileBuilder->clearLayer(layerIndex);
            _tileBuilder->importGeoJSONFeatureCollection(layerIndex, geoJSON);
        }
        catch (const std::exception& ex) {
            Log::Errorf("GeoJSONVectorTileDataSource::setLayerGeoJSON: Failed to update layer: %s", ex.what());
            throw GenericException("Failed to set layer contents", ex.what());
        }
        notifyTilesChanged(false);
    }
    
    void GeoJSONVectorTileDataSource::deleteLayer(int layerIndex) {
        try {
            std::lock_guard<std::mutex> lock(_mutex);
            _tileBuilder->deleteLayer(layerIndex);
        }
        catch (const std::exception& ex) {
            Log::Errorf("GeoJSONVectorTileDataSource::deleteLayer: Failed to delete layer: %s", ex.what());
        }
        notifyTilesChanged(false);
    }

    MapBounds GeoJSONVectorTileDataSource::getDataExtent() const {
        std::lock_guard<std::mutex> lock(_mutex);
        MapBounds mapBounds;
        for (int layerIndex : _tileBuilder->getLayerIndices()) {
            // NOTE: layerBounds are flipped
            auto layerBounds = _tileBuilder->getLayerBounds(layerIndex);
            mapBounds.expandToContain(MapPos(layerBounds.min(0), -layerBounds.max(1)));
            mapBounds.expandToContain(MapPos(layerBounds.max(0), -layerBounds.min(1)));
        }
        return mapBounds;
    }
    
    std::shared_ptr<TileData> GeoJSONVectorTileDataSource::loadTile(const MapTile& mapTile) {
        std::lock_guard<std::mutex> lock(_mutex);
        Log::Infof("GeoJSONVectorTileDataSource::loadTile: Loading %s", mapTile.toString().c_str());
        try {
            protobuf::encoded_message encodedTile;
            _tileBuilder->buildTile(mapTile.getZoom(), mapTile.getX(), mapTile.getY(), encodedTile);
            auto data = std::make_shared<BinaryData>(reinterpret_cast<const unsigned char*>(encodedTile.data().data()), encodedTile.data().size());
            return std::make_shared<TileData>(data);
        }
        catch (const std::exception& ex) {
            Log::Errorf("GeoJSONVectorTileDataSource::loadTile: Failed to build tile: %s", ex.what());
            return std::shared_ptr<TileData>();
        }
    }
    
}
