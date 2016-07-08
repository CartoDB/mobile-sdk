#ifdef _CARTO_GDAL_SUPPORT

#include "OGRVectorDataBase.h"
#include "utils/Log.h"

#include <ogrsf_frmts.h>
#include <cpl_port.h>
#include <cpl_config.h>

namespace {
    
    struct OGRLibrary {
        OGRLibrary() {
            OGRRegisterAll();
        }
    } _OGRLibrary;
    
}

namespace carto {
    
    OGRVectorDataBase::OGRVectorDataBase(const std::string& fileName, bool writable) :
        _poDS(nullptr),
        _poLayers(),
        _mutex()
    {
        OGRSFDriver* poDriver = nullptr;
        _poDS = OGRSFDriverRegistrar::Open(fileName.c_str(), writable, &poDriver);
        if (!_poDS) {
            Log::Errorf("OGRVectorDataBase: Failed to open file %s.", fileName.c_str());
            return;
        }
        
        for (int i = 0; i < _poDS->GetLayerCount(); i++) {
            OGRLayer* poLayer = _poDS->GetLayer(i);
            _poLayers.push_back(poLayer);
        }
    }
    
    OGRVectorDataBase::~OGRVectorDataBase() {
        if (_poDS) {
            _poDS->Release();
        }
    }
    
    int OGRVectorDataBase::getLayerCount() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return static_cast<int>(_poLayers.size());
    }
    
    std::vector<std::string> OGRVectorDataBase::getLayerNames() const {
        std::lock_guard<std::mutex> lock(_mutex);
        std::vector<std::string> layerNames;
        for (const OGRLayer* poLayer : _poLayers) {
            layerNames.push_back(const_cast<OGRLayer*>(poLayer)->GetName());
        }
        return layerNames;
    }

    bool OGRVectorDataBase::createLayer(const std::string& name, int epsgCode, OGRGeometryType::OGRGeometryType geometryType) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        if (!_poDS) {
            return false;
        }
        
        if (!_poDS->TestCapability(ODsCCreateLayer)) {
            Log::Warnf("OGRVectorDataBase::createLayer: Database does not support creating layers");
        }
        
        std::shared_ptr<OGRSpatialReference> poSpatialRef;
        if (epsgCode) {
            poSpatialRef = std::make_shared<OGRSpatialReference>();
            poSpatialRef->importFromEPSG(epsgCode);
        }
        OGRwkbGeometryType wkbGeometryType = wkbUnknown;
        switch (geometryType) {
            case OGRGeometryType::OGR_GEOMETRY_TYPE_POINT:
                wkbGeometryType = wkbPoint;
                break;
            case OGRGeometryType::OGR_GEOMETRY_TYPE_LINE:
                wkbGeometryType = wkbLineString;
                break;
            case OGRGeometryType::OGR_GEOMETRY_TYPE_POLYGON:
                wkbGeometryType = wkbPolygon;
                break;
            case OGRGeometryType::OGR_GEOMETRY_TYPE_MULTIPOINT:
                wkbGeometryType = wkbMultiPoint;
                break;
            case OGRGeometryType::OGR_GEOMETRY_TYPE_MULTILINE:
                wkbGeometryType = wkbMultiLineString;
                break;
            case OGRGeometryType::OGR_GEOMETRY_TYPE_MULTIPOLYGON:
                wkbGeometryType = wkbMultiPolygon;
                break;
            case OGRGeometryType::OGR_GEOMETRY_TYPE_GEOMETRYCOLLECTION:
                wkbGeometryType = wkbGeometryCollection;
                break;
            default:
                wkbGeometryType = wkbUnknown;
                break;
        }
        OGRLayer* poLayer = _poDS->CreateLayer(name.c_str(), poSpatialRef.get(), wkbGeometryType, NULL);
        if (!poLayer) {
            Log::Errorf("OGRVectorDataBase::createLayer: Failed to create layer %s", name.c_str());
            return false;
        }
        _poLayers.push_back(poLayer);
        return true;
    }
    
    bool OGRVectorDataBase::deleteLayer(int index) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        if (!_poDS) {
            return false;
        }

        if (!_poDS->TestCapability(ODsCDeleteLayer)) {
            Log::Warnf("OGRVectorDataBase::deleteLayer: Database does not support deleting layers");
        }

        OGRErr err = _poDS->DeleteLayer(index);
        if (err != OGRERR_NONE) {
            Log::Errorf("OGRVectorDataBase::deleteLayer: Failed to delete layer %d, error code: %d", index, (int)err);
            return false;
        }
        _poLayers.erase(_poLayers.begin() + index);
        return true;
    }
    
    bool OGRVectorDataBase::testCapability(const std::string& capability) const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _poDS->TestCapability(capability.c_str()) != 0;
    }

    void OGRVectorDataBase::SetConfigOption(const std::string& name, const std::string& value) {
        CPLSetConfigOption(name.c_str(), value.c_str());
    }
    
    std::string OGRVectorDataBase::GetConfigOption(const std::string& name) {
        const char* value = CPLGetConfigOption(name.c_str(), nullptr);
        if (!value) {
            return std::string();
        }
        return value;
    }
    
}

#endif
