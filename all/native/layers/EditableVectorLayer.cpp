#ifdef _CARTO_EDITABLE_SUPPORT

#include "EditableVectorLayer.h"
#include "components/CancelableThreadPool.h"
#include "datasources/VectorDataSource.h"
#include "geometry/PointGeometry.h"
#include "geometry/LineGeometry.h"
#include "geometry/PolygonGeometry.h"
#include "geometry/MultiGeometry.h"
#include "layers/VectorEditEventListener.h"
#include "renderers/BillboardRenderer.h"
#include "renderers/LineRenderer.h"
#include "renderers/MapRenderer.h"
#include "renderers/NMLModelRenderer.h"
#include "renderers/PointRenderer.h"
#include "renderers/Polygon3DRenderer.h"
#include "renderers/PolygonRenderer.h"
#include "renderers/components/CullState.h"
#include "renderers/drawdatas/LabelDrawData.h"
#include "renderers/drawdatas/LineDrawData.h"
#include "renderers/drawdatas/MarkerDrawData.h"
#include "renderers/drawdatas/NMLModelDrawData.h"
#include "renderers/drawdatas/PointDrawData.h"
#include "renderers/drawdatas/Polygon3DDrawData.h"
#include "renderers/drawdatas/PolygonDrawData.h"
#include "renderers/drawdatas/PopupDrawData.h"
#include "vectorelements/Label.h"
#include "vectorelements/Line.h"
#include "vectorelements/Marker.h"
#include "vectorelements/NMLModel.h"
#include "vectorelements/Point.h"
#include "vectorelements/Polygon3D.h"
#include "vectorelements/Polygon.h"
#include "vectorelements/Popup.h"
#include "ui/TouchHandler.h"
#include "ui/VectorElementDragInfo.h"
#include "utils/Log.h"

#include <vector>

namespace carto {

    EditableVectorLayer::EditableVectorLayer(const std::shared_ptr<VectorDataSource>& dataSource) :
        VectorLayer(dataSource),
        _dataSourceListener(),
        _selectedVectorElement(),
        _overlayRenderer(std::make_shared<PointRenderer>()),
        _overlayStyleNormal(),
        _overlayStyleVirtual(),
        _overlayStyleSelected(),
        _overlayPoints(),
        _overlayDragPoint(),
        _overlayDragGeometry(),
        _overlayDragGeometryPos(),
        _overlayDragStarted(false),
        _overlayDragMode(VectorElementDragMode::VECTOR_ELEMENT_DRAG_MODE_VERTEX),
        _vectorEditEventListener()
    {
    }

    EditableVectorLayer::~EditableVectorLayer() {
    }

    std::shared_ptr<VectorElement> EditableVectorLayer::getSelectedVectorElement() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _selectedVectorElement;
    }
    
    void EditableVectorLayer::setSelectedVectorElement(const std::shared_ptr<VectorElement>& element) {
        std::shared_ptr<VectorElement> oldSelectedVectorElement;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            oldSelectedVectorElement = _selectedVectorElement;
            if (element == oldSelectedVectorElement) {
                return;
            }

            _selectedVectorElement = std::shared_ptr<VectorElement>();

            _overlayPoints.clear(); // do not cache overlay points
            _overlayDragPoint.reset();
            _overlayDragGeometry.reset();
            _overlayDragStarted = false;

            _overlayStyleNormal.reset();
            _overlayStyleVirtual.reset();
            _overlayStyleSelected.reset();
        }

        DirectorPtr<VectorEditEventListener> vectorEditEventListener = _vectorEditEventListener;

        if (vectorEditEventListener) {
            if (oldSelectedVectorElement) {
                vectorEditEventListener->onElementDeselected(oldSelectedVectorElement);
            }
            if (element) {
                if (vectorEditEventListener->onElementSelect(element)) {
                    std::shared_ptr<PointStyle> overlayStyleNormal = vectorEditEventListener->onSelectDragPointStyle(element, VectorElementDragPointStyle::VECTOR_ELEMENT_DRAG_POINT_STYLE_NORMAL);
                    std::shared_ptr<PointStyle> overlayStyleVirtual = vectorEditEventListener->onSelectDragPointStyle(element, VectorElementDragPointStyle::VECTOR_ELEMENT_DRAG_POINT_STYLE_VIRTUAL);
                    std::shared_ptr<PointStyle> overlayStyleSelected = vectorEditEventListener->onSelectDragPointStyle(element, VectorElementDragPointStyle::VECTOR_ELEMENT_DRAG_POINT_STYLE_SELECTED);
                        
                    std::lock_guard<std::recursive_mutex> lock(_mutex);
                    _selectedVectorElement = element;
                    _overlayStyleNormal = overlayStyleNormal;
                    _overlayStyleVirtual = overlayStyleVirtual;
                    _overlayStyleSelected = overlayStyleSelected;
                }
            }
        }
        
        refresh();
    }

    std::shared_ptr<VectorEditEventListener> EditableVectorLayer::getVectorEditEventListener() const {
        return _vectorEditEventListener.get();
    }
    
    void EditableVectorLayer::setVectorEditEventListener(const std::shared_ptr<VectorEditEventListener>& listener) {
        _vectorEditEventListener.set(listener);
    }

    void EditableVectorLayer::setComponents(const std::shared_ptr<CancelableThreadPool>& envelopeThreadPool,
        const std::shared_ptr<CancelableThreadPool>& tileThreadPool,
        const std::weak_ptr<Options>& options,
        const std::weak_ptr<MapRenderer>& mapRenderer,
        const std::weak_ptr<TouchHandler>& touchHandler)
    {
        VectorLayer::setComponents(envelopeThreadPool, tileThreadPool, options, mapRenderer, touchHandler);

        if (touchHandler.lock()) {
            registerTouchHandlerListener();
        } else {
            unregisterTouchHandlerListener();
        }
    }

    void EditableVectorLayer::offsetLayerHorizontally(double offset) {
        VectorLayer::offsetLayerHorizontally(offset);
        _overlayRenderer->offsetLayerHorizontally(offset);
    }

    void EditableVectorLayer::onSurfaceCreated(ShaderManager& shaderManager) {
        VectorLayer::onSurfaceCreated(shaderManager);
        _overlayRenderer->onSurfaceCreated(shaderManager);
    }

    bool EditableVectorLayer::onDrawFrame(float deltaSeconds, BillboardSorter& billboardSorter, StyleTextureCache& styleCache, const ViewState& viewState)
    {
        bool refresh = VectorLayer::onDrawFrame(deltaSeconds, billboardSorter, styleCache, viewState);
        _overlayRenderer->onDrawFrame(deltaSeconds, styleCache, viewState);
        return refresh;
    }

    void EditableVectorLayer::onSurfaceDestroyed() {
        _overlayRenderer->onSurfaceDestroyed();
        VectorLayer::onSurfaceDestroyed();
    }

    void EditableVectorLayer::addRendererElement(const std::shared_ptr<VectorElement>& element) {
        if (!IsSameElement(element, _selectedVectorElement)) { // NOTE: locked already
            VectorLayer::addRendererElement(element);
        }
    }
    
    bool EditableVectorLayer::refreshRendererElements() {
        if (_selectedVectorElement) { // NOTE: locked already
            VectorLayer::addRendererElement(_selectedVectorElement);
        }
        bool billboardChanged = VectorLayer::refreshRendererElements();
        syncElementOverlayPoints(_selectedVectorElement);
        return billboardChanged;
    }
    
    bool EditableVectorLayer::syncRendererElement(const std::shared_ptr<VectorElement>& element, const ViewState& viewState, bool remove) {
        if (IsSameElement(element, _selectedVectorElement)) { // NOTE: locked already
            syncElementOverlayPoints(_selectedVectorElement);
        }
        return VectorLayer::syncRendererElement(element, viewState, remove);
    }
    
    void EditableVectorLayer::registerDataSourceListener() {
        _dataSourceListener = std::make_shared<DataSourceListener>(std::static_pointer_cast<EditableVectorLayer>(shared_from_this()));
        _dataSource->registerOnChangeListener(_dataSourceListener);
    }

    void EditableVectorLayer::unregisterDataSourceListener() {
        _dataSource->unregisterOnChangeListener(_dataSourceListener);
        _dataSourceListener.reset();
    }

    void EditableVectorLayer::registerTouchHandlerListener() {
        if (std::shared_ptr<TouchHandler> touchHandler = _touchHandler.lock()) {
            _touchHandlerListener = std::make_shared<TouchHandlerListener>(std::static_pointer_cast<EditableVectorLayer>(shared_from_this()));
            touchHandler->registerOnTouchListener(_touchHandlerListener);
        }
    }

    void EditableVectorLayer::unregisterTouchHandlerListener() {
        if (std::shared_ptr<TouchHandler> touchHandler = _touchHandler.lock()) {
            touchHandler->unregisterOnTouchListener(_touchHandlerListener);
            _touchHandlerListener.reset();
        }
    }

    EditableVectorLayer::DataSourceListener::DataSourceListener(const std::shared_ptr<EditableVectorLayer>& layer) :
        _layer(layer)
    {
    }

    void EditableVectorLayer::DataSourceListener::onElementAdded(const std::shared_ptr<VectorElement>& element) {
        if (std::shared_ptr<EditableVectorLayer> layer = _layer.lock()) {
            layer->refreshElement(element, false);
        }
        else {
            Log::Error("EditableVectorLayer::DataSourceListener: lost connection to layer");
        }
    }

    void EditableVectorLayer::DataSourceListener::onElementChanged(const std::shared_ptr<VectorElement>& element) {
        if (std::shared_ptr<EditableVectorLayer> layer = _layer.lock()) {
            layer->refreshElement(element, false);
        }
        else {
            Log::Error("EditableVectorLayer::DataSourceListener: lost connection to layer");
        }
    }

    void EditableVectorLayer::DataSourceListener::onElementRemoved(const std::shared_ptr<VectorElement>& element) {
        if (std::shared_ptr<EditableVectorLayer> layer = _layer.lock()) {
            if (element == layer->getSelectedVectorElement()) {
                layer->setSelectedVectorElement(std::shared_ptr<VectorElement>());
            }
            layer->refreshElement(element, true);
        }
        else {
            Log::Error("EditableVectorLayer::DataSourceListener: lost connection to layer");
        }
    }

    void EditableVectorLayer::DataSourceListener::onElementsAdded(const std::vector<std::shared_ptr<VectorElement> >& elements) {
        if (std::shared_ptr<EditableVectorLayer> layer = _layer.lock()) {
            layer->refresh();
        }
        else {
            Log::Error("EditableVectorLayer::DataSourceListener: lost connection to layer");
        }
    }

    void EditableVectorLayer::DataSourceListener::onElementsChanged() {
        if (std::shared_ptr<EditableVectorLayer> layer = _layer.lock()) {
            layer->refresh();
        }
        else {
            Log::Error("EditableVectorLayer::DataSourceListener: lost connection to layer");
        }
    }

    void EditableVectorLayer::DataSourceListener::onElementsRemoved() {
        if (std::shared_ptr<EditableVectorLayer> layer = _layer.lock()) {
            layer->setSelectedVectorElement(std::shared_ptr<VectorElement>());
            layer->refresh();
        }
        else {
            Log::Error("EditableVectorLayer::DataSourceListener: lost connection to layer");
        }
    }

    EditableVectorLayer::TouchHandlerListener::TouchHandlerListener(const std::shared_ptr<EditableVectorLayer>& layer) :
        _layer(layer)
    {
    }

    bool EditableVectorLayer::TouchHandlerListener::onTouchEvent(int action, const ScreenPos& screenPos1, const ScreenPos& screenPos2) {
        std::shared_ptr<EditableVectorLayer> layer = _layer.lock();
        if (!layer) {
            return false;
        }

        DirectorPtr<VectorEditEventListener> vectorEditEventListener = _vectorEditEventListener;

        std::lock_guard<std::recursive_mutex> lock(layer->_mutex);

        std::shared_ptr<MapRenderer> mapRenderer = layer->_mapRenderer.lock();
        if (!mapRenderer) {
            return false;
        }

        std::shared_ptr<VectorElement> selectedElement = layer->getSelectedVectorElement();
        if (!selectedElement) {
            return false;
        }
        std::shared_ptr<Point> selectedPoint = layer->_overlayDragPoint;
        std::shared_ptr<Geometry> selectedGeometry = layer->_overlayDragGeometry;

        switch (action) {
        case TouchHandler::ACTION_POINTER_1_DOWN:
            {
                MapPos mapPos1 = layer->_dataSource->getProjection()->fromInternal(mapRenderer->screenToWorld(screenPos1));
                MapPos cameraPos = mapRenderer->getCameraPos();
                MapPos touchPos = mapRenderer->screenToWorld(screenPos1);
                std::vector<RayIntersectedElement> results;
                layer->_overlayRenderer->calculateRayIntersectedElements(layer, cameraPos, touchPos - cameraPos, mapRenderer->getViewState(), results);
                if (!results.empty()) {
                    VectorElementDragResult::VectorElementDragResult result = VectorElementDragResult::VECTOR_ELEMENT_DRAG_RESULT_IGNORE;
                    if (vectorEditEventListener) {
                        auto dragInfo = std::make_shared<VectorElementDragInfo>(selectedElement, VectorElementDragMode::VECTOR_ELEMENT_DRAG_MODE_VERTEX, screenPos1, mapPos1);
                        result = vectorEditEventListener->onDragStart(dragInfo);
                    }
                    layer->_overlayDragMode = VectorElementDragMode::VECTOR_ELEMENT_DRAG_MODE_VERTEX;
                    layer->_overlayDragPoint = std::static_pointer_cast<Point>(results.front().getElement<VectorElement>());
                    switch (result) {
                        case VectorElementDragResult::VECTOR_ELEMENT_DRAG_RESULT_IGNORE:
                            layer->_overlayDragPoint.reset();
                            break;
                        case VectorElementDragResult::VECTOR_ELEMENT_DRAG_RESULT_STOP:
                            layer->_overlayDragPoint.reset();
                            return true;
                        case VectorElementDragResult::VECTOR_ELEMENT_DRAG_RESULT_MODIFY:
                            layer->_overlayDragStarted = true;
                            layer->updateElementPoint(selectedElement, selectedPoint, mapPos1);
                            return true;
                        case VectorElementDragResult::VECTOR_ELEMENT_DRAG_RESULT_DELETE:
                            layer->removeElementPoint(selectedElement, selectedPoint);
                            return true;
                    }
                }
                
                results.clear();
                layer->calculateRayIntersectedElements(*layer->_dataSource->getProjection(), cameraPos, touchPos - cameraPos, mapRenderer->getViewState(), results);
                for (const RayIntersectedElement& result : results) {
                    if (result.getElement<VectorElement>() != selectedElement) {
                        continue;
                    }
                    
                    VectorElementDragResult::VectorElementDragResult result = VectorElementDragResult::VECTOR_ELEMENT_DRAG_RESULT_IGNORE;
                    if (vectorEditEventListener) {
                        auto dragInfo = std::make_shared<VectorElementDragInfo>(selectedElement, VectorElementDragMode::VECTOR_ELEMENT_DRAG_MODE_ELEMENT, screenPos1, mapPos1);
                        result = vectorEditEventListener->onDragStart(dragInfo);
                    }
                    layer->_overlayDragMode = VectorElementDragMode::VECTOR_ELEMENT_DRAG_MODE_ELEMENT;
                    layer->_overlayDragGeometry = selectedElement->getGeometry();
                    layer->_overlayDragGeometryPos = layer->_dataSource->getProjection()->fromInternal(touchPos);
                    switch (result) {
                        case VectorElementDragResult::VECTOR_ELEMENT_DRAG_RESULT_IGNORE:
                            layer->_overlayDragGeometry.reset();
                            break;
                        case VectorElementDragResult::VECTOR_ELEMENT_DRAG_RESULT_STOP:
                            layer->_overlayDragGeometry.reset();
                            return false;
                        case VectorElementDragResult::VECTOR_ELEMENT_DRAG_RESULT_MODIFY:
                            layer->_overlayDragStarted = true;
                            layer->updateElementGeometry(selectedElement, selectedElement->getGeometry(), MapVec());
                            return true;
                        case VectorElementDragResult::VECTOR_ELEMENT_DRAG_RESULT_DELETE:
                            layer->removeElement(selectedElement);
                            return true;
                    }
                }
            }
            break;
        case TouchHandler::ACTION_MOVE:
            {
                if (!layer->_overlayDragStarted) {
                    return false;
                }

                MapPos mapPos1 = layer->_dataSource->getProjection()->fromInternal(mapRenderer->screenToWorld(screenPos1));
                VectorElementDragResult::VectorElementDragResult result = VectorElementDragResult::VECTOR_ELEMENT_DRAG_RESULT_IGNORE;
                if (vectorEditEventListener) {
                    auto dragInfo = std::make_shared<VectorElementDragInfo>(selectedElement, layer->_overlayDragMode, screenPos1, mapPos1);
                    result = vectorEditEventListener->onDragMove(dragInfo);
                }

                switch (result) {
                    case VectorElementDragResult::VECTOR_ELEMENT_DRAG_RESULT_IGNORE:
                        break;
                    case VectorElementDragResult::VECTOR_ELEMENT_DRAG_RESULT_STOP:
                        layer->_overlayDragPoint.reset();
                        layer->_overlayDragGeometry.reset();
                        layer->_overlayDragStarted = false;
                        layer->refresh();
                        return true;
                    case VectorElementDragResult::VECTOR_ELEMENT_DRAG_RESULT_MODIFY:
                        if (layer->_overlayDragMode == VectorElementDragMode::VECTOR_ELEMENT_DRAG_MODE_VERTEX) {
                            layer->updateElementPoint(selectedElement, selectedPoint, mapPos1);
                        } else {
                            MapVec delta = mapPos1 - layer->_overlayDragGeometryPos;
                            layer->updateElementGeometry(selectedElement, selectedGeometry, delta);
                        }
                        return true;
                    case VectorElementDragResult::VECTOR_ELEMENT_DRAG_RESULT_DELETE:
                        layer->_overlayDragPoint.reset();
                        layer->_overlayDragGeometry.reset();
                        layer->_overlayDragStarted = false;
                        if (layer->_overlayDragMode == VectorElementDragMode::VECTOR_ELEMENT_DRAG_MODE_VERTEX) {
                            layer->removeElementPoint(selectedElement, selectedPoint);
                        } else {
                            layer->removeElement(selectedElement);
                        }
                        return true;
                }
            }
            break;
        case TouchHandler::ACTION_POINTER_1_UP:
            {
                if (!layer->_overlayDragStarted) {
                    return false;
                }

                MapPos mapPos1 = layer->_dataSource->getProjection()->fromInternal(mapRenderer->screenToWorld(screenPos1));
                VectorElementDragResult::VectorElementDragResult result = VectorElementDragResult::VECTOR_ELEMENT_DRAG_RESULT_IGNORE;
                if (vectorEditEventListener) {
                    auto dragInfo = std::make_shared<VectorElementDragInfo>(selectedElement, layer->_overlayDragMode, screenPos1, mapPos1);
                    result = vectorEditEventListener->onDragEnd(dragInfo);
                }
                layer->_overlayDragStarted = false;
                layer->_overlayDragPoint.reset();
                layer->_overlayDragGeometry.reset();
                switch (result) {
                    case VectorElementDragResult::VECTOR_ELEMENT_DRAG_RESULT_IGNORE:
                        layer->refresh();
                        break;
                    case VectorElementDragResult::VECTOR_ELEMENT_DRAG_RESULT_STOP:
                        layer->refresh();
                        return true;
                    case VectorElementDragResult::VECTOR_ELEMENT_DRAG_RESULT_MODIFY:
                        if (layer->_overlayDragMode == VectorElementDragMode::VECTOR_ELEMENT_DRAG_MODE_VERTEX) {
                            layer->updateElementPoint(selectedElement, selectedPoint, mapPos1);
                        } else {
                            MapVec delta = mapPos1 - layer->_overlayDragGeometryPos;
                            layer->updateElementGeometry(selectedElement, selectedGeometry, delta);
                        }
                        layer->refresh();
                        return true;
                    case VectorElementDragResult::VECTOR_ELEMENT_DRAG_RESULT_DELETE:
                        layer->_overlayDragPoint.reset();
                        layer->_overlayDragGeometry.reset();
                        if (layer->_overlayDragMode == VectorElementDragMode::VECTOR_ELEMENT_DRAG_MODE_VERTEX) {
                            layer->removeElementPoint(selectedElement, selectedPoint);
                        } else {
                            layer->removeElement(selectedElement);
                        }
                        layer->refresh();
                        return true;
                }
            }
            break;
        }
        return false;
    }

    void EditableVectorLayer::updateElementGeometry(std::shared_ptr<VectorElement> element, std::shared_ptr<Geometry> geometry, const MapVec& delta) {
        if (!element) {
            return;
        }
        
        geometry = updateGeometryPoints(geometry, delta);

        DirectorPtr<VectorEditEventListener> vectorEditEventListener = _vectorEditEventListener;

        if (vectorEditEventListener) {
            vectorEditEventListener->onElementModify(element, geometry);
        }

        syncElementOverlayPoints(element);
        
        if (const std::shared_ptr<MapRenderer>& mapRenderer = _mapRenderer.lock()) {
            mapRenderer->requestRedraw();
        }
    }

    std::shared_ptr<Geometry> EditableVectorLayer::updateGeometryPoints(std::shared_ptr<Geometry> geometry, const MapVec& delta) {
        if (auto pointGeometry = std::dynamic_pointer_cast<PointGeometry>(geometry)) {
            MapPos mapPos = pointGeometry->getPos() + delta;
            geometry = std::make_shared<PointGeometry>(mapPos);
        } else if (auto lineGeometry = std::dynamic_pointer_cast<LineGeometry>(geometry)) {
            std::vector<MapPos> mapPoses = lineGeometry->getPoses();
            std::for_each(mapPoses.begin(), mapPoses.end(), [delta](MapPos& mapPos) { mapPos = mapPos + delta; });
            geometry = std::make_shared<LineGeometry>(mapPoses);
        } else if (auto polygonGeometry = std::dynamic_pointer_cast<PolygonGeometry>(geometry)) {
            std::vector<std::vector<MapPos> > rings = polygonGeometry->getRings();
            for (std::vector<MapPos>& ring : rings) {
                std::for_each(ring.begin(), ring.end(), [delta](MapPos& mapPos) { mapPos = mapPos + delta; });
            }
            geometry = std::make_shared<PolygonGeometry>(rings);
        } else if (auto multiGeometry = std::dynamic_pointer_cast<MultiGeometry>(geometry)) {
            std::vector<std::shared_ptr<Geometry> > geometries;
            for (int i = 0; i < multiGeometry->getGeometryCount(); i++) {
                geometries.push_back(updateGeometryPoints(multiGeometry->getGeometry(i), delta));
            }
            geometry = std::make_shared<MultiGeometry>(geometries);
        }
        return geometry;
    }
    
    void EditableVectorLayer::removeElement(std::shared_ptr<VectorElement> element) {
        DirectorPtr<VectorEditEventListener> vectorEditEventListener = _vectorEditEventListener;

        if (vectorEditEventListener) {
            vectorEditEventListener->onElementDelete(element);
        }
        element.reset();
        
        syncElementOverlayPoints(element);
        
        if (const std::shared_ptr<MapRenderer>& mapRenderer = _mapRenderer.lock()) {
            mapRenderer->requestRedraw();
        }
    }
    
    void EditableVectorLayer::updateElementPoint(std::shared_ptr<VectorElement> element, const std::shared_ptr<Point>& dragPoint, const MapPos& mapPos) {
        if (!element) {
            return;
        }

        auto it = std::find(_overlayPoints.begin(), _overlayPoints.end(), dragPoint);
        if (it == _overlayPoints.end()) {
            return;
        }
        int offset = 0;
        int index = static_cast<int>(it - _overlayPoints.begin());

        std::shared_ptr<Geometry> geometry = element->getGeometry();
        geometry = updateGeometryPoint(geometry, offset, index, mapPos);

        DirectorPtr<VectorEditEventListener> vectorEditEventListener = _vectorEditEventListener;

        if (geometry) {
            if (vectorEditEventListener) {
                vectorEditEventListener->onElementModify(element, geometry);
            }
        } else {
            if (vectorEditEventListener) {
                vectorEditEventListener->onElementDelete(element);
            }
            element.reset();
        }

        syncElementOverlayPoints(element);
        
        if (const std::shared_ptr<MapRenderer>& mapRenderer = _mapRenderer.lock()) {
            mapRenderer->requestRedraw();
        }
    }

    std::shared_ptr<Geometry> EditableVectorLayer::updateGeometryPoint(std::shared_ptr<Geometry> geometry, int& offset, int index, const MapPos& mapPos) {
        if (index < offset) {
            return geometry;
        }

        int points = 0;
        if (auto pointGeometry = std::dynamic_pointer_cast<PointGeometry>(geometry)) {
            points = 1;
            if (index - offset < points) {
                geometry = std::make_shared<PointGeometry>(mapPos);
            }
        } else if (auto lineGeometry = std::dynamic_pointer_cast<LineGeometry>(geometry)) {
            std::vector<MapPos> mapPoses = lineGeometry->getPoses();
            points = static_cast<int>(mapPoses.size() * 2) - 1;
            int localIndex = index - offset;
            if (localIndex < points) {
                if (localIndex % 2 == 0) {
                    mapPoses[localIndex / 2] = mapPos;
                } else {
                    mapPoses.insert(mapPoses.begin() + localIndex / 2 + 1, mapPos);
                    _overlayPoints.insert(_overlayPoints.begin() + index + 1, createOverlayPoint(mapPos, true, std::numeric_limits<int>::max()));
                    _overlayPoints.insert(_overlayPoints.begin() + index - 0, createOverlayPoint(mapPos, false, std::numeric_limits<int>::max()));
                }
                geometry = std::make_shared<LineGeometry>(mapPoses);
            }
        } else if (auto polygonGeometry = std::dynamic_pointer_cast<PolygonGeometry>(geometry)) {
            std::vector<std::vector<MapPos> > rings = polygonGeometry->getRings();
            for (std::vector<MapPos>& ring : rings) {
                bool closedRing = !ring.empty() && ring.front() == ring.back();
                offset += points;
                points = static_cast<int>(ring.size() * 2) - (closedRing ? 2 : 0);
                int localIndex = index - offset;
                if (localIndex < points) {
                    if (localIndex % 2 == 0) {
                        ring[localIndex / 2] = mapPos;
                        if (closedRing && localIndex == 0) {
                            ring.back() = ring.front();
                        }
                    } else {
                        ring.insert(ring.begin() + localIndex / 2 + 1, mapPos);
                        _overlayPoints.insert(_overlayPoints.begin() + index + 1, createOverlayPoint(mapPos, true, std::numeric_limits<int>::max()));
                        _overlayPoints.insert(_overlayPoints.begin() + index - 0, createOverlayPoint(mapPos, false, std::numeric_limits<int>::max()));
                    }
                    geometry = std::make_shared<PolygonGeometry>(rings);
                    break;
                }
            }
        } else if (auto multiGeometry = std::dynamic_pointer_cast<MultiGeometry>(geometry)) {
            std::vector<std::shared_ptr<Geometry> > geometries;
            for (int i = 0; i < multiGeometry->getGeometryCount(); i++) {
                std::shared_ptr<Geometry> geom = updateGeometryPoint(multiGeometry->getGeometry(i), offset, index, mapPos);
                if (geom) {
                    geometries.push_back(geom);
                }
            }
            geometry = std::make_shared<MultiGeometry>(geometries);
        }
        
        offset += points;
        return geometry;
    }
    
    void EditableVectorLayer::removeElementPoint(std::shared_ptr<VectorElement> element, const std::shared_ptr<Point>& dragPoint) {
        if (!element) {
            return;
        }

        auto it = std::find(_overlayPoints.begin(), _overlayPoints.end(), dragPoint);
        if (it == _overlayPoints.end()) {
            return;
        }
        int offset = 0;
        int index = static_cast<int>(it - _overlayPoints.begin());

        std::shared_ptr<Geometry> geometry = element->getGeometry();
        geometry = removeGeometryPoint(geometry, offset, index);
    
        DirectorPtr<VectorEditEventListener> vectorEditEventListener = _vectorEditEventListener;

        if (geometry) {
            if (vectorEditEventListener) {
                vectorEditEventListener->onElementModify(element, geometry);
            }
        } else {
            if (vectorEditEventListener) {
                vectorEditEventListener->onElementDelete(element);
            }
            element.reset();
        }

        syncElementOverlayPoints(element);

        if (const std::shared_ptr<MapRenderer>& mapRenderer = _mapRenderer.lock()) {
            mapRenderer->requestRedraw();
        }
    }

    std::shared_ptr<Geometry> EditableVectorLayer::removeGeometryPoint(std::shared_ptr<Geometry> geometry, int& offset, int index) {
        if (index < offset) {
            return geometry;
        }

        int points = 0;
        if (auto pointGeometry = std::dynamic_pointer_cast<PointGeometry>(geometry)) {
            points = 1;
            if (index - offset < points) {
                geometry = std::shared_ptr<Geometry>();
            }
        } else if (auto lineGeometry = std::dynamic_pointer_cast<LineGeometry>(geometry)) {
            std::vector<MapPos> mapPoses = lineGeometry->getPoses();
            points = static_cast<int>(mapPoses.size() * 2) - 1;
            int localIndex = index - offset;
            if (localIndex < points) {
                if (localIndex % 2 == 0) {
                    if (mapPoses.size() > 2) {
                        mapPoses.erase(mapPoses.begin() + localIndex / 2);
                        _overlayPoints.erase(_overlayPoints.begin() + index);
                        _overlayPoints.erase(_overlayPoints.begin() + (localIndex > 0 ? index - 1 : index));
                        geometry = std::make_shared<LineGeometry>(mapPoses);
                    } else {
                        geometry = std::shared_ptr<Geometry>();
                    }
                }
            }
        } else if (auto polygonGeometry = std::dynamic_pointer_cast<PolygonGeometry>(geometry)) {
            std::vector<std::vector<MapPos> > rings = polygonGeometry->getRings();
            for (std::vector<MapPos>& ring : rings) {
                bool closedRing = !ring.empty() && ring.front() == ring.back();
                offset += points;
                points = static_cast<int>(ring.size() * 2) - (closedRing ? 2 : 0);
                int localIndex = index - offset;
                if (localIndex < points) {
                    if (localIndex % 2 == 0) {
                        if (points > 6) {
                            ring.erase(ring.begin() + localIndex / 2);
                            if (closedRing && localIndex == 0) {
                                ring.back() = ring.front();
                            }
                            _overlayPoints.erase(_overlayPoints.begin() + index + 1);
                            _overlayPoints.erase(_overlayPoints.begin() + index);
                            geometry = std::make_shared<PolygonGeometry>(rings);
                        } else {
                            size_t n = std::find(rings.begin(), rings.end(), ring) - rings.begin();
                            if (n > 0) {
                                rings.erase(rings.begin() + n);
                                geometry = std::make_shared<PolygonGeometry>(rings);
                            } else {
                                geometry = std::shared_ptr<Geometry>();
                            }
                        }
                    }
                    break;
                }
            }
        } else if (auto multiGeometry = std::dynamic_pointer_cast<MultiGeometry>(geometry)) {
            std::vector<std::shared_ptr<Geometry> > geometries;
            for (int i = 0; i < multiGeometry->getGeometryCount(); i++) {
                std::shared_ptr<Geometry> geom = removeGeometryPoint(multiGeometry->getGeometry(i), offset, index);
                if (geom) {
                    geometries.push_back(geom);
                }
            }
            if (!geometries.empty()) {
                geometry = std::make_shared<MultiGeometry>(geometries);
            } else {
                geometry = std::shared_ptr<Geometry>();
            }
        }

        offset += points;
        return geometry;
    }
    
    void EditableVectorLayer::syncElementOverlayPoints(const std::shared_ptr<VectorElement>& element) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        std::vector<std::shared_ptr<Point> > overlayPoints;
        if (element && element->isVisible()) {
            int index = 0;
            std::shared_ptr<Geometry> geometry = element->getGeometry();
            createGeometryOverlayPoints(geometry, index, overlayPoints);
        }
        
        std::swap(overlayPoints, _overlayPoints);
        for (const std::shared_ptr<Point>& overlayPoint : _overlayPoints) {
            if (overlayPoint->getDrawData()) {
                _overlayRenderer->addElement(overlayPoint);
            }
        }
        _overlayRenderer->refreshElements();
    }

    void EditableVectorLayer::createGeometryOverlayPoints(const std::shared_ptr<Geometry>& geometry, int& index, std::vector<std::shared_ptr<Point> >& overlayPoints) const {
        if (auto pointGeometry = std::dynamic_pointer_cast<PointGeometry>(geometry)) {
            MapPos mapPos = pointGeometry->getPos();
            overlayPoints.push_back(createOverlayPoint(mapPos, false, index++));
        } else if (auto lineGeometry = std::dynamic_pointer_cast<LineGeometry>(geometry)) {
            const std::vector<MapPos>& mapPoses = lineGeometry->getPoses();
            for (size_t i = 0; i < mapPoses.size(); i++) {
                MapPos mapPos = mapPoses[i];
                if (i > 0) {
                    MapPos prevMapPos = mapPoses[i - 1];
                    overlayPoints.push_back(createOverlayPoint(prevMapPos + (mapPos - prevMapPos) * 0.5, true, index++));
                }
                overlayPoints.push_back(createOverlayPoint(mapPos, false, index++));
            }
        } else if (auto polygonGeometry = std::dynamic_pointer_cast<PolygonGeometry>(geometry)) {
            for (const std::vector<MapPos>& ring : polygonGeometry->getRings()) {
                bool closedRing = !ring.empty() && ring.front() == ring.back();
                for (size_t i = 0; i < ring.size() - (closedRing ? 1 : 0); i++) {
                    MapPos mapPos = ring[i];
                    overlayPoints.push_back(createOverlayPoint(mapPos, false, index++));
                    MapPos nextMapPos = ring[i + 1 < ring.size() ? i + 1 : 0];
                    overlayPoints.push_back(createOverlayPoint(mapPos + (nextMapPos - mapPos) * 0.5, true, index++));
                }
            }
        } else if (auto multiGeometry = std::dynamic_pointer_cast<MultiGeometry>(geometry)) {
            for (int i = 0; i < multiGeometry->getGeometryCount(); i++) {
                createGeometryOverlayPoints(multiGeometry->getGeometry(i), index, overlayPoints);
            }
        }
    }

    std::shared_ptr<Point> EditableVectorLayer::createOverlayPoint(const MapPos& mapPos, bool virtualPoint, int index) const {
        std::shared_ptr<Point> overlayPoint;
        if (index >= 0 && index < static_cast<int>(_overlayPoints.size())) {
            overlayPoint = _overlayPoints[index];
            overlayPoint->setPos(mapPos);
            overlayPoint->setStyle(overlayPoint == _overlayDragPoint ? _overlayStyleSelected : (virtualPoint ? _overlayStyleVirtual : _overlayStyleNormal));
        } else {
            overlayPoint = std::make_shared<Point>(mapPos, virtualPoint ? _overlayStyleVirtual : _overlayStyleNormal);
        }
        if (overlayPoint->getStyle()) {
            overlayPoint->setDrawData(std::make_shared<PointDrawData>(*overlayPoint->getGeometry(), *overlayPoint->getStyle(), *_dataSource->getProjection()));
        }
        return overlayPoint;
    }
    
    bool EditableVectorLayer::IsSameElement(const std::shared_ptr<VectorElement>& element1, const std::shared_ptr<VectorElement>& element2) {
        if (!element1 || !element2) {
            return element1 == element2;
        }
        if (element1 == element2) {
            return true;
        }
        if (element1->getId() == -1) {
            return false;
        }
        return element1->getId() == element2->getId();
    }
    
}

#endif
