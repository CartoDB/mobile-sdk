#ifdef _CARTO_GDAL_SUPPORT

#include "OGRVectorDataSource.h"
#include "OGRVectorDataBase.h"
#include "core/MapEnvelope.h"
#include "graphics/ViewState.h"
#include "projections/Projection.h"
#include "geometry/PointGeometry.h"
#include "geometry/LineGeometry.h"
#include "geometry/PolygonGeometry.h"
#include "geometry/MultiGeometry.h"
#include "geometry/MultiPointGeometry.h"
#include "geometry/MultiLineGeometry.h"
#include "geometry/MultiPolygonGeometry.h"
#include "geometry/GeometrySimplifier.h"
#include "vectorelements/Point.h"
#include "vectorelements/Line.h"
#include "vectorelements/Polygon.h"
#include "vectorelements/Marker.h"
#include "vectorelements/Text.h"
#include "vectorelements/GeometryCollection.h"
#include "renderers/components/CullState.h"
#include "styles/StyleSelector.h"
#include "styles/StyleSelectorContext.h"
#include "utils/Log.h"
#include "styles/PointStyle.h"
#include "styles/LineStyle.h"
#include "styles/PolygonStyle.h"
#include "styles/MarkerStyle.h"
#include "styles/TextStyle.h"
#include "styles/GeometryCollectionStyle.h"
#include "styles/GeometryCollectionStyleBuilder.h"
#include "projections/EPSG3857.h"

#include <ogrsf_frmts.h>
#include <cpl_port.h>
#include <cpl_config.h>

namespace carto {

    struct OGRVectorDataSource::LayerSpatialReference {
        LayerSpatialReference(OGRLayer* poLayer, const std::shared_ptr<Projection>& proj) : _poSpatialRef(nullptr), _poCoordinateTransform(nullptr), _poInverseCoordinateTransform(nullptr)
        {
            _poSpatialRef = new OGRSpatialReference();
            if (std::dynamic_pointer_cast<EPSG3857>(proj)) {
                _poSpatialRef->importFromEPSG(3857);
            } else {
                _poSpatialRef->SetWellKnownGeogCS(proj->getName().c_str());
            }
            if (!_poSpatialRef->IsSame(poLayer->GetSpatialRef())) {
                Log::Info("OGRVectorDataSource::LayerSpatialReference: Data source and file spatial reference systems do no match, using coordinate transformations.");
                _poCoordinateTransform = OGRCreateCoordinateTransformation(poLayer->GetSpatialRef(), _poSpatialRef);
                _poInverseCoordinateTransform = OGRCreateCoordinateTransformation(_poSpatialRef, poLayer->GetSpatialRef());
            }
        }

        ~LayerSpatialReference() {
            if (_poCoordinateTransform) {
                OGRCoordinateTransformation::DestroyCT(_poCoordinateTransform);
            }
            if (_poInverseCoordinateTransform) {
                OGRCoordinateTransformation::DestroyCT(_poInverseCoordinateTransform);
            }
            delete _poSpatialRef;
        }
     
        MapPos transform(double x, double y, double z) const {
            if (_poCoordinateTransform) {
                _poCoordinateTransform->Transform(1, &x, &y, &z);
            }
            return MapPos(x, y, z);
        }

        MapPos transform(const MapPos& mapPos) const {
            return transform(mapPos.getX(), mapPos.getY(), mapPos.getZ());
        }

        MapPos inverseTransform(double x, double y, double z) const {
            if (_poInverseCoordinateTransform) {
                _poInverseCoordinateTransform->Transform(1, &x, &y, &z);
            }
            return MapPos(x, y, z);
        }
     
        MapPos inverseTransform(const MapPos& mapPos) const {
            return inverseTransform(mapPos.getX(), mapPos.getY(), mapPos.getZ());
        }

    private:
        OGRSpatialReference* _poSpatialRef;
        OGRCoordinateTransformation* _poCoordinateTransform;
        OGRCoordinateTransformation* _poInverseCoordinateTransform;
    };

    OGRVectorDataSource::OGRVectorDataSource(const std::shared_ptr<Projection>& projection, const std::shared_ptr<StyleSelector>& styleSelector, const std::string& fileName) :
        VectorDataSource(projection),
        _codePage("ISO-8859-1"),
        _styleSelector(styleSelector),
        _geometrySimplifier(),
        _localElementId(-1),
        _localElements(),
        _dataBase(std::make_shared<OGRVectorDataBase>(fileName, false)),
        _poLayer(),
        _poLayerSpatialRef()
    {
        std::lock_guard<std::mutex> lock(_dataBase->_mutex);
        if (!_dataBase->_poLayers.empty()) {
            _poLayer = _dataBase->_poLayers.front();
            _poLayerSpatialRef = std::make_shared<LayerSpatialReference>(_poLayer, projection);
        } else {
            Log::Errorf("OGRVectorDataSource: No layers in file %s", fileName.c_str());
        }
    }

    OGRVectorDataSource::OGRVectorDataSource(const std::shared_ptr<Projection>& projection, const std::shared_ptr<StyleSelector>& styleSelector, const std::shared_ptr<OGRVectorDataBase>& dataBase, int layerIndex) :
        VectorDataSource(projection),
        _codePage("ISO-8859-1"),
        _styleSelector(styleSelector),
        _geometrySimplifier(),
        _localElementId(-1),
        _localElements(),
        _dataBase(dataBase),
        _poLayer(),
        _poLayerSpatialRef()
    {
        std::lock_guard<std::mutex> lock(_dataBase->_mutex);
        if (layerIndex >= 0 && layerIndex < static_cast<int>(_dataBase->_poLayers.size())) {
            _poLayer = _dataBase->_poLayers[layerIndex];
            _poLayerSpatialRef = std::make_shared<LayerSpatialReference>(_poLayer, projection);
        } else {
            Log::Errorf("OGRVectorDataSource: No layer %d in database", layerIndex);
        }
    }
    
    OGRVectorDataSource::~OGRVectorDataSource() {
    }
    
    std::string OGRVectorDataSource::getCodePage() const {
        std::lock_guard<std::mutex> lock(_dataBase->_mutex);
        return _codePage;
    }
    
    void OGRVectorDataSource::setCodePage(const std::string& codePage) {
        {
            std::lock_guard<std::mutex> lock(_dataBase->_mutex);
            _codePage = codePage;
        }
        notifyElementsChanged();
    }

    std::shared_ptr<GeometrySimplifier> OGRVectorDataSource::getGeometrySimplifier() const {
        std::lock_guard<std::mutex> lock(_dataBase->_mutex);
        return _geometrySimplifier;
    }

    void OGRVectorDataSource::setGeometrySimplifier(const std::shared_ptr<GeometrySimplifier>& simplifier) {
        {
            std::lock_guard<std::mutex> lock(_dataBase->_mutex);
            _geometrySimplifier = simplifier;
        }
        notifyElementsChanged();
    }

    MapBounds OGRVectorDataSource::getDataExtent() const {
        std::lock_guard<std::mutex> lock(_dataBase->_mutex);

        if (!_poLayer) {
            return MapBounds();
        }

        MapBounds mapBounds;
        OGREnvelope oEnvelope;
        _poLayer->GetExtent(&oEnvelope);
        mapBounds.expandToContain(_poLayerSpatialRef->transform(oEnvelope.MinX, oEnvelope.MinY, 0));
        mapBounds.expandToContain(_poLayerSpatialRef->transform(oEnvelope.MaxX, oEnvelope.MinY, 0));
        mapBounds.expandToContain(_poLayerSpatialRef->transform(oEnvelope.MaxX, oEnvelope.MaxY, 0));
        mapBounds.expandToContain(_poLayerSpatialRef->transform(oEnvelope.MinX, oEnvelope.MaxY, 0));
        return mapBounds;
    }
    
    int OGRVectorDataSource::getFeatureCount() const {
        std::lock_guard<std::mutex> lock(_dataBase->_mutex);
        
        if (!_poLayer) {
            return 0;
        }

        return _poLayer->GetFeatureCount();
    }
    
    OGRGeometryType::OGRGeometryType OGRVectorDataSource::getGeometryType() const {
        std::lock_guard<std::mutex> lock(_dataBase->_mutex);
        
        if (!_poLayer) {
            return OGRGeometryType::OGR_GEOMETRY_TYPE_UNKNOWN;
        }
        
        OGRGeometryType::OGRGeometryType geometryType = OGRGeometryType::OGR_GEOMETRY_TYPE_UNKNOWN;
        switch (_poLayer->GetGeomType()) {
            case wkbPoint:
                geometryType = OGRGeometryType::OGR_GEOMETRY_TYPE_POINT;
                break;
            case wkbLineString:
                geometryType = OGRGeometryType::OGR_GEOMETRY_TYPE_LINE;
                break;
            case wkbPolygon:
                geometryType = OGRGeometryType::OGR_GEOMETRY_TYPE_POLYGON;
                break;
            case wkbMultiPoint:
                geometryType = OGRGeometryType::OGR_GEOMETRY_TYPE_MULTIPOINT;
                break;
            case wkbMultiLineString:
                geometryType = OGRGeometryType::OGR_GEOMETRY_TYPE_MULTILINE;
                break;
            case wkbMultiPolygon:
                geometryType = OGRGeometryType::OGR_GEOMETRY_TYPE_MULTIPOLYGON;
                break;
            case wkbGeometryCollection:
                geometryType = OGRGeometryType::OGR_GEOMETRY_TYPE_GEOMETRYCOLLECTION;
                break;
            default:
                geometryType = OGRGeometryType::OGR_GEOMETRY_TYPE_UNKNOWN;
                break;
        }
        return geometryType;
    }
    
    void OGRVectorDataSource::add(const std::shared_ptr<VectorElement>& element) {
        {
            std::lock_guard<std::mutex> lock(_dataBase->_mutex);

            if (!_poLayer) {
                return;
            }

            long long id = _localElementId--;
            element->setId(id);
            _localElements[id] = element;
        }
        notifyElementAdded(element);
    }
    
    bool OGRVectorDataSource::remove(const std::shared_ptr<VectorElement>& elementToRemove) {
        {
            std::lock_guard<std::mutex> lock(_dataBase->_mutex);

            if (!_poLayer) {
                return false;
            }
            
            if (elementToRemove->getId() < 0) {
                auto it = _localElements.find(elementToRemove->getId());
                if (it == _localElements.end()) {
                    return false;
                }
                _localElements.erase(it);
            } else {
                _localElements[elementToRemove->getId()] = std::shared_ptr<VectorElement>();
            }
        }
        notifyElementRemoved(elementToRemove);
        return true;
    }
    
    bool OGRVectorDataSource::isCommitted() const {
        std::lock_guard<std::mutex> lock(_dataBase->_mutex);
        return _localElements.empty();
    }
    
    std::vector<std::shared_ptr<VectorElement> > OGRVectorDataSource::commit() {
        std::vector<std::shared_ptr<VectorElement> > committedElements;
        {
            std::lock_guard<std::mutex> lock(_dataBase->_mutex);

            if (!_poLayer) {
                return std::vector<std::shared_ptr<VectorElement> >();
            }

            if (!_poLayer->TestCapability(OLCRandomWrite)) {
                Log::Warnf("OGRVectorDataSource::commit: Layer does not support deleting features");
            }
            if (!_poLayer->TestCapability(OLCDeleteFeature)) {
                Log::Warnf("OGRVectorDataSource::commit: Layer does not support deleting features");
            }
            
            for (auto it = _localElements.begin(); it != _localElements.end(); ) {
                long long id = it->first;
                std::shared_ptr<VectorElement> element = it->second;
                ++it;
                if (id < 0) {
                    if (element) {
                        std::shared_ptr<OGRFeature> poFeature = createOGRFeature(element);
                        if (poFeature) {
                            OGRErr err = _poLayer->CreateFeature(poFeature.get());
                            if (err != OGRERR_NONE) {
                                Log::Errorf("OGRVectorDataSource::commit: Failed to create new feature, error code: %d", (int)err);
                                continue;
                            }
                            detachElement(element);
                            element->setId(poFeature->GetFID());
                            attachElement(element);
                        }
                    }
                } else {
                    if (element) {
                        std::shared_ptr<OGRFeature> poFeature = createOGRFeature(element);
                        if (poFeature) {
                            OGRErr err = _poLayer->SetFeature(poFeature.get());
                            if (err != OGRERR_NONE) {
                                Log::Errorf("OGRVectorDataSource::commit: Failed to update feature, error code: %d", (int)err);
                                continue;
                            }
                        }
                    } else {
                        OGRErr err = _poLayer->DeleteFeature(id);
                        if (err != OGRERR_NONE) {
                            Log::Errorf("OGRVectorDataSource::commit: Failed to remove feature, error code: %d", (int)err);
                            continue;
                        }
                    }
                }
                committedElements.push_back(element);
                it = _localElements.erase(--it);
            }
            
            OGRErr err = _poLayer->SyncToDisk();
            if (err != OGRERR_NONE) {
                Log::Errorf("OGRVectorDataSource::commit: SyncToDisk failed, error code: %d", (int)err);
            }
        }
        notifyElementsChanged();
        return committedElements;
    }
    
    std::vector<std::shared_ptr<VectorElement> > OGRVectorDataSource::rollback() {
        std::vector<std::shared_ptr<VectorElement> > rolledbackElements;
        {
            std::lock_guard<std::mutex> lock(_dataBase->_mutex);

            if (!_poLayer) {
                return std::vector<std::shared_ptr<VectorElement> >();
            }

            for (auto it = _localElements.begin(); it != _localElements.end(); it++) {
                std::shared_ptr<VectorElement> element = it->second;
                rolledbackElements.push_back(element);
            }
            _localElements.clear();
        }
        notifyElementsChanged();
        return rolledbackElements;
    }
    
    std::vector<std::string> OGRVectorDataSource::getFieldNames() const {
        std::lock_guard<std::mutex> lock(_dataBase->_mutex);
        
        if (!_poLayer) {
            return std::vector<std::string>();
        }
        
        std::vector<std::string> fieldNames;
        OGRFeatureDefn *poFDefn = _poLayer->GetLayerDefn();
        if (poFDefn) {
            for (int i = 0; i < poFDefn->GetFieldCount(); i++) {
                fieldNames.push_back(poFDefn->GetFieldDefn(i)->GetNameRef());
            }
        }
        return fieldNames;
    }

    bool OGRVectorDataSource::createField(const std::string& name, OGRFieldType::OGRFieldType type, int width) {
        std::lock_guard<std::mutex> lock(_dataBase->_mutex);
        
        if (!_poLayer) {
            return false;
        }
        
        if (!_poLayer->TestCapability(OLCCreateField)) {
            Log::Warnf("OGRVectorDataSource::createField: Layer does not support creating fields");
        }
        
        ::OGRFieldType ogrFieldType = OFTString;
        switch (type) {
            case OGRFieldType::OGR_FIELD_TYPE_INTEGER:
                ogrFieldType = OFTInteger;
                break;
            case OGRFieldType::OGR_FIELD_TYPE_REAL:
                ogrFieldType = OFTReal;
                break;
            case OGRFieldType::OGR_FIELD_TYPE_STRING:
                ogrFieldType = OFTString;
                break;
            case OGRFieldType::OGR_FIELD_TYPE_TIME:
                ogrFieldType = OFTTime;
                break;
            case OGRFieldType::OGR_FIELD_TYPE_DATE:
                ogrFieldType = OFTDate;
                break;
            case OGRFieldType::OGR_FIELD_TYPE_DATETIME:
                ogrFieldType = OFTDateTime;
                break;
            default:
                Log::Warnf("OGRVectorDataSource::createField: Setting field type to string for field %s", name.c_str());
                ogrFieldType = OFTString;
                break;
        }
        
        std::shared_ptr<OGRFieldDefn> ogrField = std::make_shared<OGRFieldDefn>(name.c_str(), ogrFieldType);
        if (ogrFieldType == OFTString) {
            ogrField->SetWidth(width);
        }
        OGRErr err = _poLayer->CreateField(ogrField.get());
        if (err != OGRERR_NONE) {
            Log::Errorf("OGRVectorDataSource::createField: Error while creating field %s, error code %d", name.c_str(), (int)err);
            return false;
        }
        return true;
    }

    bool OGRVectorDataSource::deleteField(int index) {
        std::lock_guard<std::mutex> lock(_dataBase->_mutex);
        
        if (!_poLayer) {
            return false;
        }
        
        if (!_poLayer->TestCapability(OLCDeleteField)) {
            Log::Warnf("OGRVectorDataSource::deleteField: Layer does not support deleting fields");
        }
        
        OGRErr err = _poLayer->DeleteField(index);
        if (err != OGRERR_NONE) {
            Log::Errorf("OGRVectorDataSource::deleteField: Error while deleting field %d, error code %d", index, (int)err);
            return false;
        }
        return true;
    }

    std::vector<std::shared_ptr<VectorElement>> OGRVectorDataSource::loadElements(const std::shared_ptr<CullState>& cullState) {
        std::lock_guard<std::mutex> lock(_dataBase->_mutex);
        
        if (!_poLayer) {
            return std::vector<std::shared_ptr<VectorElement> >();
        }

        float simplifierScale = calculateGeometrySimplifierScale(cullState->getViewState());

        MapBounds bounds;
        for (const MapPos& mapPosInternal : cullState->getEnvelope().getConvexHull()) {
            MapPos mapPos = _projection->fromInternal(mapPosInternal);
            bounds.expandToContain(_poLayerSpatialRef->inverseTransform(mapPos.getX(), mapPos.getY(), mapPos.getZ()));
        }
        _poLayer->SetSpatialFilterRect(bounds.getMin().getX(), bounds.getMin().getY(), bounds.getMax().getX(), bounds.getMax().getY());

        std::vector<std::shared_ptr<VectorElement>> elements;
        _poLayer->ResetReading();
        while (auto poFeature = std::shared_ptr<OGRFeature>(_poLayer->GetNextFeature(), OGRFeature::DestroyFeature)) {
            auto elementIt = _localElements.find(poFeature->GetFID());
            if (elementIt != _localElements.end()) {
                if (elementIt->second) {
                    elements.push_back(elementIt->second);
                }
                continue;
            }

            OGRGeometry* poGeometry = poFeature->GetGeometryRef();
            if (!poGeometry) {
                continue;
            }

            std::map<std::string, Variant> metaData;
            OGRFeatureDefn *poFDefn = _poLayer->GetLayerDefn();
            if (poFDefn) {
                for (int i = 0; i < poFDefn->GetFieldCount(); i++) {
                    OGRFieldDefn* poFieldDefn = poFeature->GetFieldDefnRef(i);
                    Variant value;
                    switch (poFieldDefn->getType()) {
                        case OFTInteger:
                        case OFTInteger64:
                            value = Variant(poFeature->GetFieldAsInteger64());
                            break;
                        case OFTReal:
                            value = Variant(poFeature->GetFieldAsDouble());
                            break;
                        default: {
                            const char* strValue = poFeature->GetFieldAsString(i);
                            if (!strValue) {
                                continue;
                            }
                            char* utf8Value = CPLRecode(strValue, _codePage.c_str(), "UTF-8");
                            if (utf8Value) {
                                value = Variant(utf8Value);
                                CPLFree(utf8Value);
                            } else {
                                value = Variant(strValue);
                            }
                            break;
                        }
                    }
                    metaData[poFDefn->GetFieldDefn(i)->GetNameRef()] = value;
                }
            }
                
            std::shared_ptr<Geometry> geometry = createGeometry(poGeometry);
            if (_geometrySimplifier) {
                if (geometry) {
                    geometry = _geometrySimplifier->simplify(geometry, simplifierScale);
                }
            }
            if (geometry) {
                std::shared_ptr<VectorElement> vectorElement = createVectorElement(cullState->getViewState(), geometry, metaData);
                if (vectorElement) {
                    vectorElement->setId(poFeature->GetFID());
                    vectorElement->setMetaData(metaData);
                    attachElement(vectorElement);
                    elements.push_back(std::move(vectorElement));
                }
            }
        }
        
        for (auto elementIt = _localElements.begin(); elementIt != _localElements.end(); elementIt++) {
            if (elementIt->first < 0 && elementIt->second) {
                elements.push_back(elementIt->second);
            }
        }

        return elements;
    }

    bool OGRVectorDataSource::testCapability(const std::string& capability) {
        std::lock_guard<std::mutex> lock(_dataBase->_mutex);
        return _poLayer->TestCapability(capability.c_str()) != 0;
    }

    void OGRVectorDataSource::notifyElementChanged(const std::shared_ptr<VectorElement>& element) {
        {
            std::lock_guard<std::mutex> lock(_dataBase->_mutex);
            _localElements[element->getId()] = element;
        }
        VectorDataSource::notifyElementChanged(element);
    }
    
    void OGRVectorDataSource::SetConfigOption(const std::string& name, const std::string& value) {
        CPLSetConfigOption(name.c_str(), value.c_str());
    }
    
    std::string OGRVectorDataSource::GetConfigOption(const std::string& name) {
        const char* value = CPLGetConfigOption(name.c_str(), nullptr);
        if (!value) {
            return std::string();
        }
        return value;
    }
    
    std::shared_ptr<Geometry> OGRVectorDataSource::createGeometry(const OGRGeometry* poGeometry) const {
        if (!poGeometry) {
            return std::shared_ptr<Geometry>();
        }
        
        std::shared_ptr<Geometry> geometry;
        switch (wkbFlatten(poGeometry->getGeometryType())) {
            case wkbPoint: {
                    OGRPoint* poPoint = (OGRPoint*) poGeometry;
                    MapPos mapPos = _poLayerSpatialRef->transform(poPoint->getX(), poPoint->getY(), poPoint->getZ());
                    geometry = std::make_shared<PointGeometry>(mapPos);
                }
                break;
            case wkbLineString: {
                    OGRLineString* poLineString = (OGRLineString*) poGeometry;
                    std::vector<MapPos> mapPoses(poLineString->getNumPoints());
                    for (int i = 0; i < poLineString->getNumPoints(); i++) {
                        mapPoses[i] = _poLayerSpatialRef->transform(poLineString->getX(i), poLineString->getY(i), poLineString->getZ(i));
                    }
                    geometry = std::make_shared<LineGeometry>(mapPoses);
                }
                break;
            case wkbPolygon: {
                    OGRPolygon* poPolygon = (OGRPolygon*) poGeometry;
                    OGRLineString* poLineString = poPolygon->getExteriorRing();
                    std::vector<MapPos> mapPoses(poLineString->getNumPoints());
                    for (int i = 0; i < poLineString->getNumPoints(); i++) {
                        mapPoses[i] = _poLayerSpatialRef->transform(poLineString->getX(i), poLineString->getY(i), poLineString->getZ(i));
                    }
                    std::vector<std::vector<MapPos>> interiorMapPoses(poPolygon->getNumInteriorRings());
                    for (int n = 0; n < poPolygon->getNumInteriorRings(); n++) {
                        poLineString = poPolygon->getInteriorRing(n);
                        interiorMapPoses[n].resize(poLineString->getNumPoints());
                        for (int i = 0; i < poLineString->getNumPoints(); i++) {
                            interiorMapPoses[n][i] = _poLayerSpatialRef->transform(poLineString->getX(i), poLineString->getY(i),   poLineString->getZ(i));
                        }
                    }
                    geometry = std::make_shared<PolygonGeometry>(mapPoses, interiorMapPoses);
                }
                break;
            case wkbGeometryCollection: {
                    OGRGeometryCollection* poGeomCollection = (OGRGeometryCollection*) poGeometry;
                    std::vector<std::shared_ptr<Geometry> > geoms;
                    for (int i = 0; i < poGeomCollection->getNumGeometries(); i++) {
                        std::shared_ptr<Geometry> geom = createGeometry(poGeomCollection->getGeometryRef(i));
                        if (geom) {
                            geoms.push_back(geom);
                        }
                    }
                    geometry = std::make_shared<MultiGeometry>(geoms);
                }
                break;
            case wkbMultiPoint: {
                    OGRGeometryCollection* poGeomCollection = (OGRGeometryCollection*) poGeometry;
                    std::vector<std::shared_ptr<PointGeometry> > points;
                    for (int i = 0; i < poGeomCollection->getNumGeometries(); i++) {
                        std::shared_ptr<Geometry> geom = createGeometry(poGeomCollection->getGeometryRef(i));
                        if (auto point = std::dynamic_pointer_cast<PointGeometry>(geom)) {
                            points.push_back(point);
                        }
                    }
                    geometry = std::make_shared<MultiPointGeometry>(points);
                }
                break;
            case wkbMultiLineString: {
                    OGRGeometryCollection* poGeomCollection = (OGRGeometryCollection*) poGeometry;
                    std::vector<std::shared_ptr<LineGeometry> > lines;
                    for (int i = 0; i < poGeomCollection->getNumGeometries(); i++) {
                        std::shared_ptr<Geometry> geom = createGeometry(poGeomCollection->getGeometryRef(i));
                        if (auto line = std::dynamic_pointer_cast<LineGeometry>(geom)) {
                            lines.push_back(line);
                        }
                    }
                    geometry = std::make_shared<MultiLineGeometry>(lines);
                }
                break;
            case wkbMultiPolygon: {
                    OGRGeometryCollection* poGeomCollection = (OGRGeometryCollection*) poGeometry;
                    std::vector<std::shared_ptr<PolygonGeometry> > polygons;
                    for (int i = 0; i < poGeomCollection->getNumGeometries(); i++) {
                        std::shared_ptr<Geometry> geom = createGeometry(poGeomCollection->getGeometryRef(i));
                        if (auto polygon = std::dynamic_pointer_cast<PolygonGeometry>(geom)) {
                            polygons.push_back(polygon);
                        }
                    }
                    geometry = std::make_shared<MultiPolygonGeometry>(polygons);
                }
                break;
            default: {
                    Log::Error("OGRVectorDataSource: Unimplemented geometry type!");
                }
                break;
        }

        return geometry;
    }
    
    std::shared_ptr<VectorElement> OGRVectorDataSource::createVectorElement(const ViewState& viewState, const std::shared_ptr<Geometry>& geometry, const std::map<std::string, std::string>& metaData) const {
        StyleSelectorContext context(viewState, geometry, metaData);
        std::shared_ptr<Style> style = _styleSelector->getStyle(context);
        if (auto polygonStyle = std::dynamic_pointer_cast<PolygonStyle>(style)) {
            if (auto polygonGeometry = std::dynamic_pointer_cast<PolygonGeometry>(geometry)) {
                return std::make_shared<Polygon>(polygonGeometry, polygonStyle);
            } else if (auto multiGeometry = std::dynamic_pointer_cast<MultiGeometry>(geometry)) {
                GeometryCollectionStyleBuilder builder;
                builder.setPolygonStyle(polygonStyle);
                return std::make_shared<GeometryCollection>(multiGeometry, builder.buildStyle());
            }
        } else if (auto lineStyle = std::dynamic_pointer_cast<LineStyle>(style)) {
            if (auto lineGeometry = std::dynamic_pointer_cast<LineGeometry>(geometry)) {
                return std::make_shared<Line>(lineGeometry, lineStyle);
            } else if (auto multiGeometry = std::dynamic_pointer_cast<MultiGeometry>(geometry)) {
                GeometryCollectionStyleBuilder builder;
                builder.setLineStyle(lineStyle);
                return std::make_shared<GeometryCollection>(multiGeometry, builder.buildStyle());
            }
        } else if (auto pointStyle = std::dynamic_pointer_cast<PointStyle>(style)) {
            if (auto pointGeometry = std::dynamic_pointer_cast<PointGeometry>(geometry)) {
                return std::make_shared<Point>(pointGeometry, pointStyle);
            } else if (auto multiGeometry = std::dynamic_pointer_cast<MultiGeometry>(geometry)) {
                GeometryCollectionStyleBuilder builder;
                builder.setPointStyle(pointStyle);
                return std::make_shared<GeometryCollection>(multiGeometry, builder.buildStyle());
            }
        } else if (auto markerStyle = std::dynamic_pointer_cast<MarkerStyle>(style)) {
            return std::make_shared<Marker>(geometry, markerStyle);
        } else if (auto textStyle = std::dynamic_pointer_cast<TextStyle>(style)) {
            return std::make_shared<Text>(geometry, textStyle, ""); // NOTE: we assume that textStyle uses textField property
        } else if (auto geomCollectionStyle = std::dynamic_pointer_cast<GeometryCollectionStyle>(style)) {
            if (auto multiGeometry = std::dynamic_pointer_cast<MultiGeometry>(geometry)) {
                return std::make_shared<GeometryCollection>(multiGeometry, geomCollectionStyle);
            }
        }
        return std::shared_ptr<VectorElement>();
    }
    
    std::shared_ptr<OGRGeometry> OGRVectorDataSource::createOGRGeometry(const std::shared_ptr<Geometry>& geometry) const {
        std::shared_ptr<OGRGeometry> poGeometry;
        if (auto pointGeometry = std::dynamic_pointer_cast<PointGeometry>(geometry)) {
            MapPos mapPos = _poLayerSpatialRef->inverseTransform(pointGeometry->getPos());
            poGeometry = std::make_shared<OGRPoint>(mapPos.getX(), mapPos.getY(), mapPos.getZ());
        } else if (auto lineGeometry = std::dynamic_pointer_cast<LineGeometry>(geometry)) {
            const std::vector<MapPos>& mapPoses = lineGeometry->getPoses();
            auto poLineString = std::make_shared<OGRLineString>();
            poLineString->setNumPoints(mapPoses.size());
            for (size_t i = 0; i < mapPoses.size(); i++) {
                MapPos mapPos = _poLayerSpatialRef->inverseTransform(mapPoses[i]);
                poLineString->setPoint(i, mapPos.getX(), mapPos.getY(), mapPos.getZ());
            }
            poGeometry = poLineString;
        } else if (auto polygonGeometry = std::dynamic_pointer_cast<PolygonGeometry>(geometry)) {
            const std::vector<std::vector<MapPos> >& rings = polygonGeometry->getRings();
            auto poPolygon = std::make_shared<OGRPolygon>();
            for (const std::vector<MapPos>& mapPoses : rings) {
                auto poLineString = std::make_shared<OGRLinearRing>();
                poLineString->setNumPoints(mapPoses.size());
                for (size_t i = 0; i < mapPoses.size(); i++) {
                    MapPos mapPos = _poLayerSpatialRef->inverseTransform(mapPoses[i]);
                    poLineString->setPoint(i, mapPos.getX(), mapPos.getY(), mapPos.getZ());
                }
                poPolygon->addRing(poLineString.get());
            }
            poPolygon->closeRings();
            poGeometry = poPolygon;
        } else if (auto multiGeometry = std::dynamic_pointer_cast<MultiGeometry>(geometry)) {
            std::shared_ptr<OGRGeometryCollection> poGeomCollection;
            if (std::dynamic_pointer_cast<MultiPointGeometry>(geometry)) {
                poGeomCollection = std::make_shared<OGRMultiPoint>();
            } else if (std::dynamic_pointer_cast<MultiLineGeometry>(geometry)) {
                poGeomCollection = std::make_shared<OGRMultiLineString>();
            } else if (std::dynamic_pointer_cast<MultiPolygonGeometry>(geometry)) {
                poGeomCollection = std::make_shared<OGRMultiPolygon>();
            } else {
                poGeomCollection = std::make_shared<OGRGeometryCollection>();
            }
            for (int i = 0; i < multiGeometry->getGeometryCount(); i++) {
                std::shared_ptr<OGRGeometry> poGeom = createOGRGeometry(multiGeometry->getGeometry(i));
                if (poGeom) {
                    poGeomCollection->addGeometry(poGeom.get());
                }
            }
            poGeometry = poGeomCollection;
        }
        return poGeometry;
    }
    
    std::shared_ptr<OGRFeature> OGRVectorDataSource::createOGRFeature(const std::shared_ptr<VectorElement>& element) const {
        std::shared_ptr<OGRGeometry> poGeometry = createOGRGeometry(element->getGeometry());
        if (!poGeometry) {
            return std::shared_ptr<OGRFeature>();
        }

        // Create element, set geometry
        auto poFeature = std::shared_ptr<OGRFeature>(new OGRFeature(_poLayer->GetLayerDefn()), OGRFeature::DestroyFeature);
        poFeature->SetFID(element->getId() < 0 ? OGRNullFID : element->getId());
        poFeature->SetGeometry(poGeometry.get());
        
        // Set meta data
        OGRFeatureDefn *poFDefn = _poLayer->GetLayerDefn();
        if (poFDefn) {
            std::map<std::string, Variant> metaData = element->getMetaData();
            for (int i = 0; i < poFDefn->GetFieldCount(); i++) {
                Variant value;
                auto it = metaData.find(poFDefn->GetFieldDefn(i)->GetNameRef());
                if (it != metaData.end()) {
                    value = it->second;
                }

                OGRFieldDefn* poFieldDefn = poFeature->GetFieldDefnRef(i);
                switch (poFieldDefn->getType()) {
                    case OFTInteger:
                        poFeature->SetField(i, static_cast<int>(value.getLong()));
                        break;
                    case OFTInteger64:
                        poFeature->SetField(i, value.getLong());
                        break;
                    case OFTReal:
                        poFeature->SetField(i, value.getDouble());
                        break;
                    default: {
                        std::string strValue;
                        if (value.getType() == VariantType::VARIANT_TYPE_STRING) {
                            strValue = value.getString();
                        } else {
                            strValue = value.toString();
                        }
                        char* encValue = CPLRecode(strValue.c_str(), "UTF-8", _codePage.c_str());
                        if (encValue) {
                            poFeature->SetField(i, encValue);
                            CPLFree(encValue);
                        } else {
                            poFeature->SetField(i, strValue.c_str());
                        }
                        break;
                    }
                }
            }
        }
        return poFeature;
    }

}

#endif
