#include "VectorLayer.h"
#include "components/Exceptions.h"
#include "components/CancelableThreadPool.h"
#include "datasources/VectorDataSource.h"
#include "geometry/PointGeometry.h"
#include "layers/VectorElementEventListener.h"
#include "renderers/BillboardRenderer.h"
#include "renderers/GeometryCollectionRenderer.h"
#include "renderers/LineRenderer.h"
#include "renderers/MapRenderer.h"
#include "renderers/NMLModelRenderer.h"
#include "renderers/PointRenderer.h"
#include "renderers/Polygon3DRenderer.h"
#include "renderers/PolygonRenderer.h"
#include "renderers/components/CullState.h"
#include "renderers/components/RayIntersectedElement.h"
#include "renderers/drawdatas/GeometryCollectionDrawData.h"
#include "renderers/drawdatas/LabelDrawData.h"
#include "renderers/drawdatas/LineDrawData.h"
#include "renderers/drawdatas/MarkerDrawData.h"
#include "renderers/drawdatas/NMLModelDrawData.h"
#include "renderers/drawdatas/PointDrawData.h"
#include "renderers/drawdatas/Polygon3DDrawData.h"
#include "renderers/drawdatas/PolygonDrawData.h"
#include "renderers/drawdatas/PopupDrawData.h"
#include "vectorelements/GeometryCollection.h"
#include "vectorelements/Label.h"
#include "vectorelements/Line.h"
#include "vectorelements/Marker.h"
#include "vectorelements/NMLModel.h"
#include "vectorelements/Point.h"
#include "vectorelements/Polygon3D.h"
#include "vectorelements/Polygon.h"
#include "vectorelements/Popup.h"
#include "vectorelements/CustomPopup.h"
#include "ui/VectorElementClickInfo.h"
#include "utils/Log.h"

#include <vector>

namespace carto {

    VectorLayer::VectorLayer(const std::shared_ptr<VectorDataSource>& dataSource) :
        Layer(),
        _dataSource(dataSource),
        _dataSourceListener(),
        _vectorElementEventListener(),
        _billboardRenderer(std::make_shared<BillboardRenderer>()),
        _geometryCollectionRenderer(std::make_shared<GeometryCollectionRenderer>()),
        _lineRenderer(std::make_shared<LineRenderer>()),
        _pointRenderer(std::make_shared<PointRenderer>()),
        _polygonRenderer(std::make_shared<PolygonRenderer>()),
        _polygon3DRenderer(std::make_shared<Polygon3DRenderer>()),
        _nmlModelRenderer(std::make_shared<NMLModelRenderer>()),
        _lastTask()
    {
        if (!dataSource) {
            throw NullArgumentException("Null dataSource");
        }
    }
    
    VectorLayer::~VectorLayer() {
    }
        
    std::shared_ptr<VectorDataSource> VectorLayer::getDataSource() const {
        return _dataSource.get();
    }
    
    std::shared_ptr<VectorElementEventListener> VectorLayer::getVectorElementEventListener() const {
        return _vectorElementEventListener.get();
    }
    
    void VectorLayer::setVectorElementEventListener(const std::shared_ptr<VectorElementEventListener>& eventListener) {
        _vectorElementEventListener.set(eventListener);
    }
    
    bool VectorLayer::isUpdateInProgress() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return static_cast<bool>(_lastTask);
    }
    
    void VectorLayer::setComponents(const std::shared_ptr<CancelableThreadPool>& envelopeThreadPool,
                                    const std::shared_ptr<CancelableThreadPool>& tileThreadPool,
                                    const std::weak_ptr<Options>& options,
                                    const std::weak_ptr<MapRenderer>& mapRenderer,
                                    const std::weak_ptr<TouchHandler>& touchHandler)
    {
        Layer::setComponents(envelopeThreadPool, tileThreadPool, options, mapRenderer, touchHandler);
        _billboardRenderer->setLayer(std::static_pointer_cast<VectorLayer>(shared_from_this()));
        _polygon3DRenderer->setOptions(options);
        _nmlModelRenderer->setOptions(options);
    }
    
    void VectorLayer::loadData(const std::shared_ptr<CullState>& cullState) {
        // Cancel last task, if it has not started yet
        std::shared_ptr<CancelableTask> lastTask;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            lastTask = _lastTask;
        }
        if (lastTask) {
            lastTask->cancel();
        }

        // Check if the layer should be shown
        if (!isVisible() || !getVisibleZoomRange().inRange(cullState->getViewState().getZoom())) {
            // Synchronize in case FetchTask is running
            std::shared_ptr<MapRenderer> mapRenderer;
            bool billboardsChanged = false;
            {
                std::lock_guard<std::recursive_mutex> lock(_mutex);

                // Empty all renderers of draw datas
                billboardsChanged = refreshRendererElements();
                
                mapRenderer = _mapRenderer.lock();
            }

            if (mapRenderer) {
                if (billboardsChanged) {
                    // Billboards were removed, calculate new placements
                    mapRenderer->billboardsChanged();
                }

                mapRenderer->requestRedraw();
            }
            return;
        }

        lastTask = createFetchTask(cullState);
        std::shared_ptr<CancelableThreadPool> envelopeThreadPool;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _lastTask = lastTask;
            envelopeThreadPool = _envelopeThreadPool;
        }
        if (envelopeThreadPool) {
            envelopeThreadPool->execute(lastTask, getUpdatePriority());
        }
    }

    void VectorLayer::offsetLayerHorizontally(double offset) {
        _billboardRenderer->offsetLayerHorizontally(offset);
        _geometryCollectionRenderer->offsetLayerHorizontally(offset);
        _lineRenderer->offsetLayerHorizontally(offset);
        _pointRenderer->offsetLayerHorizontally(offset);
        _polygonRenderer->offsetLayerHorizontally(offset);
        _polygon3DRenderer->offsetLayerHorizontally(offset);
        _nmlModelRenderer->offsetLayerHorizontally(offset);
    }
    
    void VectorLayer::onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager) {
        Layer::onSurfaceCreated(shaderManager, textureManager);
        _billboardRenderer->onSurfaceCreated(shaderManager, textureManager);
        _geometryCollectionRenderer->onSurfaceCreated(shaderManager, textureManager);
        _lineRenderer->onSurfaceCreated(shaderManager, textureManager);
        _pointRenderer->onSurfaceCreated(shaderManager, textureManager);
        _polygonRenderer->onSurfaceCreated(shaderManager, textureManager);
        _polygon3DRenderer->onSurfaceCreated(shaderManager, textureManager);
        _nmlModelRenderer->onSurfaceCreated(shaderManager, textureManager);
    }
    
    bool VectorLayer::onDrawFrame(float deltaSeconds, BillboardSorter& billboardSorter,
                                  StyleTextureCache& styleCache,
                                  const ViewState& viewState)
    {
        _billboardRenderer->onDrawFrame(deltaSeconds, billboardSorter, styleCache, viewState);
        _geometryCollectionRenderer->onDrawFrame(deltaSeconds, styleCache, viewState);
        _lineRenderer->onDrawFrame(deltaSeconds, styleCache, viewState);
        _pointRenderer->onDrawFrame(deltaSeconds, styleCache, viewState);
        _polygonRenderer->onDrawFrame(deltaSeconds, styleCache, viewState);
        _polygon3DRenderer->onDrawFrame(deltaSeconds, viewState);
        _nmlModelRenderer->onDrawFrame(deltaSeconds, viewState);
        return false;
    }
    
    void VectorLayer::onSurfaceDestroyed(){
        _billboardRenderer->onSurfaceDestroyed();
        _geometryCollectionRenderer->onSurfaceDestroyed();
        _lineRenderer->onSurfaceDestroyed();
        _pointRenderer->onSurfaceDestroyed();
        _polygonRenderer->onSurfaceDestroyed();
        _polygon3DRenderer->onSurfaceDestroyed();
        _nmlModelRenderer->onSurfaceDestroyed();
        Layer::onSurfaceDestroyed();
    }
    
    void VectorLayer::calculateRayIntersectedElements(const Projection& projection, const cglib::ray3<double>& ray,
                const ViewState& viewState, std::vector<RayIntersectedElement>& results) const {
        std::shared_ptr<VectorLayer> thisLayer = std::static_pointer_cast<VectorLayer>(std::const_pointer_cast<Layer>(shared_from_this()));
        _billboardRenderer->calculateRayIntersectedElements(thisLayer, ray, viewState, results);
        _geometryCollectionRenderer->calculateRayIntersectedElements(thisLayer, ray, viewState, results);
        _lineRenderer->calculateRayIntersectedElements(thisLayer, ray, viewState, results);
        _pointRenderer->calculateRayIntersectedElements(thisLayer, ray, viewState, results);
        _polygonRenderer->calculateRayIntersectedElements(thisLayer, ray, viewState, results);
        _polygon3DRenderer->calculateRayIntersectedElements(thisLayer, ray, viewState, results);
        _nmlModelRenderer->calculateRayIntersectedElements(thisLayer, ray, viewState, results);
    }

    bool VectorLayer::processClick(ClickType::ClickType clickType, const RayIntersectedElement& intersectedElement, const ViewState& viewState) const {
        if (std::shared_ptr<VectorElement> element = intersectedElement.getElement<VectorElement>()) {
            if (auto customPopup = std::dynamic_pointer_cast<CustomPopup>(element)) {
                if (auto drawData = customPopup->getDrawData()) {
                    std::vector<float> coordBuf(12);
                    BillboardRenderer::CalculateBillboardCoords(*drawData, viewState, coordBuf, 0);
                    
                    MapPos topLeft = viewState.getCameraPos() + MapVec(coordBuf[0], coordBuf[1], coordBuf[2]);
                    MapPos bottomLeft = viewState.getCameraPos() + MapVec(coordBuf[3], coordBuf[4], coordBuf[5]);
                    MapPos topRight = viewState.getCameraPos() + MapVec(coordBuf[6], coordBuf[7], coordBuf[8]);
                    MapVec delta = _dataSource->getProjection()->toInternal(intersectedElement.getHitPos()) - topLeft;

                    float x = static_cast<float>(delta.dotProduct(topRight - topLeft) / (topRight - topLeft).lengthSqr());
                    float y = static_cast<float>(delta.dotProduct(bottomLeft - topLeft) / (bottomLeft - topLeft).lengthSqr());
                    return customPopup->processClick(clickType, intersectedElement.getHitPos(), ScreenPos(x, y));
                }
            }

            DirectorPtr<VectorElementEventListener> vectorElementEventListener = _vectorElementEventListener;

            if (vectorElementEventListener) {
                auto vectorElementClickInfo = std::make_shared<VectorElementClickInfo>(clickType, intersectedElement.getHitPos(), intersectedElement.getElementPos(), element, intersectedElement.getLayer());
                return vectorElementEventListener->onVectorElementClicked(vectorElementClickInfo);
            }
        }

        return clickType == ClickType::CLICK_TYPE_SINGLE || clickType == ClickType::CLICK_TYPE_LONG; // by default, disable 'click through' for single and long clicks
    }
    
    void VectorLayer::refreshElement(const std::shared_ptr<VectorElement>& element, bool remove) {
        std::shared_ptr<MapRenderer> mapRenderer;
        bool billboardsChanged = false;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);

            if (!_lastCullState) {
                return;
            }
            
            billboardsChanged = syncRendererElement(element, _lastCullState->getViewState(), remove);
            
            if (!isVisible() || !getVisibleZoomRange().inRange(_lastCullState->getViewState().getZoom())) {
                return;
            }
            
            mapRenderer = _mapRenderer.lock();
        }

        if (mapRenderer) {
            if (billboardsChanged) {
                // Billboards were added, calculate new placements
                mapRenderer->billboardsChanged();
            }
            
            mapRenderer->requestRedraw();
        }
    }
    
    void VectorLayer::addRendererElement(const std::shared_ptr<VectorElement>& element) {
        if (!element->isVisible()) {
            return;
        }
        
        if (const std::shared_ptr<Label>& label = std::dynamic_pointer_cast<Label>(element)) {
            if (!label->getDrawData() || label->getDrawData()->isOffset()) {
                label->setDrawData(std::make_shared<LabelDrawData>(*label, *label->getStyle(), *_dataSource->getProjection(), _lastCullState->getViewState()));
            }
            _billboardRenderer->addElement(label);
        } else if (const std::shared_ptr<Line>& line = std::dynamic_pointer_cast<Line>(element)) {
            if (!line->getDrawData() || line->getDrawData()->isOffset()) {
                line->setDrawData(std::make_shared<LineDrawData>(*line->getGeometry(), *line->getStyle(), *_dataSource->getProjection()));
            }
            _lineRenderer->addElement(line);
        } else if (const std::shared_ptr<Marker>& marker = std::dynamic_pointer_cast<Marker>(element)) {
            if (!marker->getDrawData() || marker->getDrawData()->isOffset()) {
                marker->setDrawData(std::make_shared<MarkerDrawData>(*marker, *marker->getStyle(), *_dataSource->getProjection()));
            }
            _billboardRenderer->addElement(marker);
        } else if (const std::shared_ptr<Point>& point = std::dynamic_pointer_cast<Point>(element)) {
            if (!point->getDrawData() || point->getDrawData()->isOffset()) {
                point->setDrawData(std::make_shared<PointDrawData>(*point->getGeometry(), *point->getStyle(), *_dataSource->getProjection()));
            }
            _pointRenderer->addElement(point);
        } else if (const std::shared_ptr<Polygon>& polygon = std::dynamic_pointer_cast<Polygon>(element)) {
            if (!polygon->getDrawData() || polygon->getDrawData()->isOffset()) {
                polygon->setDrawData(std::make_shared<PolygonDrawData>(*polygon->getGeometry(), *polygon->getStyle(), *_dataSource->getProjection()));
            }
            _polygonRenderer->addElement(polygon);
        } else if (const std::shared_ptr<GeometryCollection>& geomCollection = std::dynamic_pointer_cast<GeometryCollection>(element)) {
            if (!geomCollection->getDrawData() || geomCollection->getDrawData()->isOffset()) {
                geomCollection->setDrawData(std::make_shared<GeometryCollectionDrawData>(*geomCollection->getGeometry(), *geomCollection->getStyle(), *_dataSource->getProjection()));
            }
            _geometryCollectionRenderer->addElement(geomCollection);
        } else if (const std::shared_ptr<Polygon3D>& polygon3D = std::dynamic_pointer_cast<Polygon3D>(element)) {
            if (!polygon3D->getDrawData() || polygon3D->getDrawData()->isOffset()) {
                polygon3D->setDrawData(std::make_shared<Polygon3DDrawData>(*polygon3D, *polygon3D->getStyle(), *_dataSource->getProjection()));
            }
            _polygon3DRenderer->addElement(polygon3D);
        } else if (const std::shared_ptr<NMLModel>& nmlModel = std::dynamic_pointer_cast<NMLModel>(element)) {
            if (!nmlModel->getDrawData() || nmlModel->getDrawData()->isOffset()) {
                nmlModel->setDrawData(std::make_shared<NMLModelDrawData>(nmlModel->getSourceModel(), ViewState::GetLocalMat(nmlModel->getGeometry()->getCenterPos(), *_dataSource->getProjection()) * cglib::mat4x4<double>::convert(nmlModel->getLocalMat())));
            }
            _nmlModelRenderer->addElement(nmlModel);
        } else if (const std::shared_ptr<Popup>& popup = std::dynamic_pointer_cast<Popup>(element)) {
            if (!popup->getDrawData() || popup->getDrawData()->isOffset()) {
                if (auto options = _options.lock()) {
                    popup->setDrawData(std::make_shared<PopupDrawData>(*popup, *popup->getStyle(), *_dataSource->getProjection(), *options, _lastCullState->getViewState()));
                } else {
                    return;
                }
            }
            _billboardRenderer->addElement(popup);
        }
    }
    
    bool VectorLayer::refreshRendererElements() {
        bool billboardsChanged = _billboardRenderer->getElementCount() > 0; // if there are any billboards currently, assume they have changed (or removed)
        _billboardRenderer->refreshElements();
        _geometryCollectionRenderer->refreshElements();
        _lineRenderer->refreshElements();
        _pointRenderer->refreshElements();
        _polygonRenderer->refreshElements();
        _polygon3DRenderer->refreshElements();
        _nmlModelRenderer->refreshElements();
        if (_billboardRenderer->getElementCount() > 0) {
            billboardsChanged = true;
        }
        return billboardsChanged;
    }
    
    bool VectorLayer::syncRendererElement(const std::shared_ptr<VectorElement> &element, const ViewState& viewState, bool remove) {
        bool visible = element->isVisible() && isVisible() && getVisibleZoomRange().inRange(viewState.getZoom());
        bool billboardsChanged = false;
        
        // Update/remove the draw data of a single element in one of the renderers,
        if (const std::shared_ptr<Label>& label = std::dynamic_pointer_cast<Label>(element)) {
            if (visible && !remove) {
                label->setDrawData(std::make_shared<LabelDrawData>(*label, *label->getStyle(), *_dataSource->getProjection(), viewState));
                _billboardRenderer->updateElement(label);
            } else {
                _billboardRenderer->removeElement(label);
            }
            billboardsChanged = true;
        } else if (const std::shared_ptr<Line>& line = std::dynamic_pointer_cast<Line>(element)) {
            if (visible && !remove) {
                line->setDrawData(std::make_shared<LineDrawData>(*line->getGeometry(), *line->getStyle(), *_dataSource->getProjection()));
                _lineRenderer->updateElement(line);
            } else {
                _lineRenderer->removeElement(line);
            }
        } else if (const std::shared_ptr<Marker>& marker = std::dynamic_pointer_cast<Marker>(element)) {
            if (visible && !remove) {
                marker->setDrawData(std::make_shared<MarkerDrawData>(*marker, *marker->getStyle(), *_dataSource->getProjection()));
                _billboardRenderer->updateElement(marker);
            } else {
                _billboardRenderer->removeElement(marker);
            }
            billboardsChanged = true;
        } else if (const std::shared_ptr<Point>& point = std::dynamic_pointer_cast<Point>(element)) {
            if (visible && !remove) {
                point->setDrawData(std::make_shared<PointDrawData>(*point->getGeometry(), *point->getStyle(), *_dataSource->getProjection()));
                _pointRenderer->updateElement(point);
            } else {
                _pointRenderer->removeElement(point);
            }
        } else if (const std::shared_ptr<Polygon>& polygon = std::dynamic_pointer_cast<Polygon>(element)) {
            if (visible && !remove) {
                polygon->setDrawData(std::make_shared<PolygonDrawData>(*polygon->getGeometry(), *polygon->getStyle(), *_dataSource->getProjection()));
                _polygonRenderer->updateElement(polygon);
            } else {
                _polygonRenderer->removeElement(polygon);
            }
        } else if (const std::shared_ptr<GeometryCollection>& geomCollection = std::dynamic_pointer_cast<GeometryCollection>(element)) {
            if (visible && !remove) {
                geomCollection->setDrawData(std::make_shared<GeometryCollectionDrawData>(*geomCollection->getGeometry(), *geomCollection->getStyle(), *_dataSource->getProjection()));
                _geometryCollectionRenderer->updateElement(geomCollection);
            } else {
                _geometryCollectionRenderer->removeElement(geomCollection);
            }
        } else if (const std::shared_ptr<Polygon3D>& polygon3D = std::dynamic_pointer_cast<Polygon3D>(element)) {
            if (visible && !remove) {
                polygon3D->setDrawData(std::make_shared<Polygon3DDrawData>(*polygon3D, *polygon3D->getStyle(), *_dataSource->getProjection()));
                _polygon3DRenderer->updateElement(polygon3D);
            } else {
                _polygon3DRenderer->removeElement(polygon3D);
            }
        } else if (const std::shared_ptr<NMLModel>& nmlModel = std::dynamic_pointer_cast<NMLModel>(element)) {
            if (visible && !remove) {
                nmlModel->setDrawData(std::make_shared<NMLModelDrawData>(nmlModel->getSourceModel(), ViewState::GetLocalMat(nmlModel->getGeometry()->getCenterPos(), *_dataSource->getProjection()) * cglib::mat4x4<double>::convert(nmlModel->getLocalMat())));
                _nmlModelRenderer->updateElement(nmlModel);
            } else {
                _nmlModelRenderer->removeElement(nmlModel);
            }
        } else if (const std::shared_ptr<Popup>& popup = std::dynamic_pointer_cast<Popup>(element)) {
            if (visible && !remove) {
                if (auto options = _options.lock()) {
                    popup->setDrawData(std::make_shared<PopupDrawData>(*popup, *popup->getStyle(), *_dataSource->getProjection(), *options, viewState));
                    _billboardRenderer->updateElement(popup);
                }
            } else {
                _billboardRenderer->removeElement(popup);
            }
            billboardsChanged = true;
        }
        return billboardsChanged;
    }
    
    void VectorLayer::registerDataSourceListener() {
        _dataSourceListener = std::make_shared<DataSourceListener>(std::static_pointer_cast<VectorLayer>(shared_from_this()));
        _dataSource->registerOnChangeListener(_dataSourceListener);
    }
    
    void VectorLayer::unregisterDataSourceListener() {
        _dataSource->unregisterOnChangeListener(_dataSourceListener);
        _dataSourceListener.reset();
    }

    std::shared_ptr<CancelableTask> VectorLayer::createFetchTask(const std::shared_ptr<CullState>& cullState) {
        return std::make_shared<FetchTask>(std::static_pointer_cast<VectorLayer>(shared_from_this()));
    }
    
    VectorLayer::DataSourceListener::DataSourceListener(const std::shared_ptr<VectorLayer>& layer) :
        _layer(layer)
    {
    }
        
    void VectorLayer::DataSourceListener::onElementAdded(const std::shared_ptr<VectorElement>& element) {
        if (std::shared_ptr<VectorLayer> layer = _layer.lock()) {
            layer->refreshElement(element, false);
        } else {
            Log::Error("VectorLayer::DataSourceListener: Lost connection to layer");
        }
    }
    
    void VectorLayer::DataSourceListener::onElementChanged(const std::shared_ptr<VectorElement>& element) {
        if (std::shared_ptr<VectorLayer> layer = _layer.lock()) {
            layer->refreshElement(element, false);
        } else {
            Log::Error("VectorLayer::DataSourceListener: Lost connection to layer");
        }
    }
    
    void VectorLayer::DataSourceListener::onElementRemoved(const std::shared_ptr<VectorElement>& element) {
        if (std::shared_ptr<VectorLayer> layer = _layer.lock()) {
            layer->refreshElement(element, true);
        } else {
            Log::Error("VectorLayer::DataSourceListener: Lost connection to layer");
        }
    }
        
    void VectorLayer::DataSourceListener::onElementsAdded(const std::vector<std::shared_ptr<VectorElement> >& elements) {
        if (std::shared_ptr<VectorLayer> layer = _layer.lock()) {
            layer->refresh();
        } else {
            Log::Error("VectorLayer::DataSourceListener: Lost connection to layer");
        }
    }
        
    void VectorLayer::DataSourceListener::onElementsChanged() {
        if (std::shared_ptr<VectorLayer> layer = _layer.lock()) {
            layer->refresh();
        } else {
            Log::Error("VectorLayer::DataSourceListener: Lost connection to layer");
        }
    }
    
    void VectorLayer::DataSourceListener::onElementsRemoved() {
        if (std::shared_ptr<VectorLayer> layer = _layer.lock()) {
            layer->refresh();
        } else {
            Log::Error("VectorLayer::DataSourceListener: Lost connection to layer");
        }
    }
    
    VectorLayer::FetchTask::FetchTask(const std::weak_ptr<VectorLayer>& layer) :
        _layer(layer), _started(false)
    {
    }
    
    void VectorLayer::FetchTask::cancel() {
        bool cancel = false;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (!_started) {
                _canceled = true;
                cancel = true;
            }
        }

        if (cancel) {
            if (std::shared_ptr<VectorLayer> layer = _layer.lock()) {
                std::lock_guard<std::recursive_mutex> lock(layer->_mutex);
                if (layer->_lastTask == shared_from_this()) {
                    layer->_lastTask.reset();
                }
            }
        }
    }
    
    void VectorLayer::FetchTask::run() {
        const std::shared_ptr<VectorLayer>& layer = _layer.lock();
        if (!layer) {
            return;
        }
        
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_canceled) {
                return;
            }
            _started = true;
        }
        
        // Renderer access needs to be synchronized, this method may be called from multiple threads at the same time
        std::shared_ptr<MapRenderer> mapRenderer;
        bool billboardsChanged = false;
        {
            std::shared_ptr<CullState> lastCullState;
            {
                std::lock_guard<std::recursive_mutex> lock(layer->_mutex);
                if (layer->isVisible() && layer->_lastCullState && layer->getVisibleZoomRange().inRange(layer->_lastCullState->getViewState().getZoom())) {
                    lastCullState = layer->_lastCullState;
                }
            }
            if (lastCullState) {
                try {
                    billboardsChanged = loadElements(lastCullState);
                }
                catch (const std::exception& ex) {
                    Log::Errorf("VectorLayer::FetchTask: Exception while loading elements: %s", ex.what());
                }
            } else {
                std::lock_guard<std::recursive_mutex> lock(layer->_mutex);
                billboardsChanged = layer->refreshRendererElements();
            }
        
            std::lock_guard<std::recursive_mutex> lock(layer->_mutex);
        
            mapRenderer = layer->_mapRenderer.lock();

            if (layer->_lastTask == shared_from_this()) {
                layer->_lastTask.reset();
            }
        }

        if (mapRenderer) {
            if (billboardsChanged) {
                // Billboards were added, calculate new placements
                mapRenderer->billboardsChanged();
            }

            mapRenderer->requestRedraw();
        }
    }
    
    bool VectorLayer::FetchTask::loadElements(const std::shared_ptr<CullState>& cullState) {
        const std::shared_ptr<VectorLayer>& layer = _layer.lock();

        std::shared_ptr<VectorData> vectorData = layer->_dataSource->loadElements(cullState);
        if (!vectorData) {
            return false;
        }

        std::lock_guard<std::recursive_mutex> lock(layer->_mutex);
        for (const std::shared_ptr<VectorElement>& element : vectorData->getElements()) {
            layer->addRendererElement(element);
        }
        return layer->refreshRendererElements();
    }

}
