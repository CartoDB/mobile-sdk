#include "VectorLayer.h"
#include "components/Exceptions.h"
#include "components/CancelableThreadPool.h"
#include "datasources/VectorDataSource.h"
#include "graphics/Bitmap.h"
#include "layers/VectorElementEventListener.h"
#include "projections/Projection.h"
#include "projections/ProjectionSurface.h"
#include "renderers/BillboardRenderer.h"
#include "renderers/GeometryCollectionRenderer.h"
#include "renderers/LineRenderer.h"
#include "renderers/MapRenderer.h"
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
#include "ui/VectorElementClickInfo.h"
#include "utils/Log.h"

#include <vector>

namespace carto {

    VectorLayer::VectorLayer(const std::shared_ptr<VectorDataSource>& dataSource) :
        Layer(),
        _dataSource(dataSource),
        _dataSourceListener(),
        _fetchingTasks(),
        _vectorElementEventListener(),
        _zBuffering(false),
        _billboardRenderer(std::make_shared<BillboardRenderer>()),
        _geometryCollectionRenderer(std::make_shared<GeometryCollectionRenderer>()),
        _lineRenderer(std::make_shared<LineRenderer>()),
        _pointRenderer(std::make_shared<PointRenderer>()),
        _polygonRenderer(std::make_shared<PolygonRenderer>()),
        _polygon3DRenderer(std::make_shared<Polygon3DRenderer>())
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

    bool VectorLayer::isZBuffering() const {
        return _zBuffering.load();
    }

    void VectorLayer::setZBuffering(bool enabled) {
        _zBuffering.store(enabled);
        refresh();
    }
    
    bool VectorLayer::isUpdateInProgress() const {
        return _fetchingTasks.getCount() > 0;
    }
    
    void VectorLayer::setComponents(const std::shared_ptr<CancelableThreadPool>& envelopeThreadPool,
                                    const std::shared_ptr<CancelableThreadPool>& tileThreadPool,
                                    const std::weak_ptr<Options>& options,
                                    const std::weak_ptr<MapRenderer>& mapRenderer,
                                    const std::weak_ptr<TouchHandler>& touchHandler)
    {
        Layer::setComponents(envelopeThreadPool, tileThreadPool, options, mapRenderer, touchHandler);
        _billboardRenderer->setComponents(std::static_pointer_cast<VectorLayer>(shared_from_this()), options, mapRenderer);
        _geometryCollectionRenderer->setComponents(options, mapRenderer);
        _lineRenderer->setComponents(options, mapRenderer);
        _pointRenderer->setComponents(options, mapRenderer);
        _polygonRenderer->setComponents(options, mapRenderer);
        _polygon3DRenderer->setComponents(options, mapRenderer);
    }
    
    void VectorLayer::loadData(const std::shared_ptr<CullState>& cullState) {
        // Cancel tasks
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            for (const std::shared_ptr<FetchTask>& task : _fetchingTasks.getAll()) {
                task->cancel();
            }
        }

        // Check if the layer should be shown
        if (!isVisible() || !getVisibleZoomRange().inRange(cullState->getViewState().getZoom()) || getOpacity() <= 0) {
            // Synchronize in case FetchTask is running
            bool billboardsChanged = false;
            {
                std::lock_guard<std::recursive_mutex> lock(_mutex);

                // Empty all renderers of draw datas
                billboardsChanged = refreshRendererElements();
            }

            if (auto mapRenderer = getMapRenderer()) {
                if (billboardsChanged) {
                    // Billboards were removed, calculate new placements
                    mapRenderer->billboardsChanged();
                }
                mapRenderer->requestRedraw();
            }
            return;
        }

        std::shared_ptr<CancelableThreadPool> envelopeThreadPool;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            envelopeThreadPool = _envelopeThreadPool;
        }
        if (envelopeThreadPool) {
            std::shared_ptr<FetchTask> task = createFetchTask(cullState);
            _fetchingTasks.insert(task);
            envelopeThreadPool->execute(task, getUpdatePriority());
        }
    }

    void VectorLayer::offsetLayerHorizontally(double offset) {
        _billboardRenderer->offsetLayerHorizontally(offset);
        _geometryCollectionRenderer->offsetLayerHorizontally(offset);
        _lineRenderer->offsetLayerHorizontally(offset);
        _pointRenderer->offsetLayerHorizontally(offset);
        _polygonRenderer->offsetLayerHorizontally(offset);
        _polygon3DRenderer->offsetLayerHorizontally(offset);
    }
    
    bool VectorLayer::onDrawFrame(float deltaSeconds, BillboardSorter& billboardSorter, const ViewState& viewState) {
        if (auto mapRenderer = getMapRenderer()) {
            float opacity = getOpacity();
            bool zBuffering = isZBuffering();

            if (opacity < 1.0f) {
                mapRenderer->clearAndBindScreenFBO(Color(0, 0, 0, 0), true, false);
            }
            if (zBuffering) {
                mapRenderer->setZBuffering(true);
            }

            bool refresh = _billboardRenderer->onDrawFrame(deltaSeconds, billboardSorter, viewState);
            _geometryCollectionRenderer->onDrawFrame(deltaSeconds, viewState);
            _lineRenderer->onDrawFrame(deltaSeconds, viewState);
            _pointRenderer->onDrawFrame(deltaSeconds, viewState);
            _polygonRenderer->onDrawFrame(deltaSeconds, viewState);
            _polygon3DRenderer->onDrawFrame(deltaSeconds, viewState);

            if (zBuffering) {
                mapRenderer->setZBuffering(false);
            }
            if (opacity < 1.0f) {
                mapRenderer->blendAndUnbindScreenFBO(opacity);
            }

            return refresh;
        }
        return false;
    }
    
    void VectorLayer::calculateRayIntersectedElements(const cglib::ray3<double>& ray, const ViewState& viewState, std::vector<RayIntersectedElement>& results) const {
        std::shared_ptr<VectorLayer> thisLayer = std::static_pointer_cast<VectorLayer>(std::const_pointer_cast<Layer>(shared_from_this()));
        _billboardRenderer->calculateRayIntersectedElements(thisLayer, ray, viewState, results);
        _geometryCollectionRenderer->calculateRayIntersectedElements(thisLayer, ray, viewState, results);
        _lineRenderer->calculateRayIntersectedElements(thisLayer, ray, viewState, results);
        _pointRenderer->calculateRayIntersectedElements(thisLayer, ray, viewState, results);
        _polygonRenderer->calculateRayIntersectedElements(thisLayer, ray, viewState, results);
        _polygon3DRenderer->calculateRayIntersectedElements(thisLayer, ray, viewState, results);
    }

    bool VectorLayer::processClick(const ClickInfo& clickInfo, const RayIntersectedElement& intersectedElement, const ViewState& viewState) const {
        std::shared_ptr<ProjectionSurface> projectionSurface = viewState.getProjectionSurface();
        if (!projectionSurface) {
            return false;
        }

        if (auto element = intersectedElement.getElement<VectorElement>()) {
            if (auto popup = std::dynamic_pointer_cast<Popup>(element)) {
                if (std::shared_ptr<BillboardDrawData> drawData = popup->getDrawData()) {
                    const std::shared_ptr<Bitmap>& bitmap = drawData->getBitmap();
                    std::vector<float> coordBuf(12);
                    if (BillboardRenderer::CalculateBillboardCoords(*drawData, viewState, coordBuf, 0)) {
                        cglib::vec3<double> originShift = viewState.getCameraPos();
                        cglib::vec3<double> topLeft     = originShift + cglib::vec3<double>(coordBuf[0], coordBuf[1], coordBuf[2]);
                        cglib::vec3<double> bottomLeft  = originShift + cglib::vec3<double>(coordBuf[3], coordBuf[4], coordBuf[5]);
                        cglib::vec3<double> topRight    = originShift + cglib::vec3<double>(coordBuf[6], coordBuf[7], coordBuf[8]);
                        cglib::vec3<double> delta = intersectedElement.getHitPos() - topLeft;

                        float x = static_cast<float>(cglib::dot_product(delta, topRight   - topLeft) / cglib::norm(topRight   - topLeft) * bitmap->getWidth());
                        float y = static_cast<float>(cglib::dot_product(delta, bottomLeft - topLeft) / cglib::norm(bottomLeft - topLeft) * bitmap->getHeight());

                        MapPos hitPos = _dataSource->getProjection()->fromInternal(projectionSurface->calculateMapPos(intersectedElement.getHitPos()));
                        if (popup->processClick(clickInfo, hitPos, ScreenPos(x, y))) {
                            return true;
                        }
                    }
                }
            }

            DirectorPtr<VectorElementEventListener> vectorElementEventListener = _vectorElementEventListener;

            if (vectorElementEventListener) {
                MapPos hitPos = _dataSource->getProjection()->fromInternal(projectionSurface->calculateMapPos(intersectedElement.getHitPos()));
                MapPos elementPos = _dataSource->getProjection()->fromInternal(projectionSurface->calculateMapPos(intersectedElement.getElementPos()));
                auto vectorElementClickInfo = std::make_shared<VectorElementClickInfo>(clickInfo, hitPos, elementPos, element, intersectedElement.getLayer());
                return vectorElementEventListener->onVectorElementClicked(vectorElementClickInfo);
            }
        }

        return clickInfo.getClickType() == ClickType::CLICK_TYPE_SINGLE || clickInfo.getClickType() == ClickType::CLICK_TYPE_LONG; // by default, disable 'click through' for single and long clicks
    }
    
    void VectorLayer::refreshElement(const std::shared_ptr<VectorElement>& element, bool remove) {
        bool billboardsChanged = false;
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);

            std::shared_ptr<CullState> lastCullState = getLastCullState();
            if (!lastCullState) {
                return;
            }
            
            billboardsChanged = syncRendererElement(element, lastCullState->getViewState(), remove);
            
            if (!isVisible() || !getVisibleZoomRange().inRange(lastCullState->getViewState().getZoom()) || getOpacity() <= 0) {
                return;
            }
        }

        if (auto mapRenderer = getMapRenderer()) {
            if (billboardsChanged) {
                // Billboards were added, calculate new placements
                mapRenderer->billboardsChanged();
            }
            mapRenderer->requestRedraw();
        }
    }
    
    void VectorLayer::addRendererElement(const std::shared_ptr<VectorElement>& element, const ViewState& viewState) {
        if (!element->isVisible()) {
            return;
        }

        std::shared_ptr<ProjectionSurface> projectionSurface = viewState.getProjectionSurface();
        if (!projectionSurface) {
            return;
        }

        if (const std::shared_ptr<Label>& label = std::dynamic_pointer_cast<Label>(element)) {
            if (!label->getDrawData() || label->getDrawData()->isOffset() || label->getDrawData()->getProjectionSurface() != projectionSurface) {
                label->setDrawData(std::make_shared<LabelDrawData>(*label, *label->getStyle(), *_dataSource->getProjection(), projectionSurface, viewState));
            }
            _billboardRenderer->addElement(label);
        } else if (const std::shared_ptr<Line>& line = std::dynamic_pointer_cast<Line>(element)) {
            if (!line->getDrawData() || line->getDrawData()->isOffset() || line->getDrawData()->getProjectionSurface() != projectionSurface) {
                line->setDrawData(std::make_shared<LineDrawData>(*line->getGeometry(), *line->getStyle(), *_dataSource->getProjection(), projectionSurface));
            }
            _lineRenderer->addElement(line);
        } else if (const std::shared_ptr<Marker>& marker = std::dynamic_pointer_cast<Marker>(element)) {
            if (!marker->getDrawData() || marker->getDrawData()->isOffset() || marker->getDrawData()->getProjectionSurface() != projectionSurface) {
                marker->setDrawData(std::make_shared<MarkerDrawData>(*marker, *marker->getStyle(), *_dataSource->getProjection(), projectionSurface));
            }
            _billboardRenderer->addElement(marker);
        } else if (const std::shared_ptr<Point>& point = std::dynamic_pointer_cast<Point>(element)) {
            if (!point->getDrawData() || point->getDrawData()->isOffset() || point->getDrawData()->getProjectionSurface() != projectionSurface) {
                point->setDrawData(std::make_shared<PointDrawData>(*point->getGeometry(), *point->getStyle(), *_dataSource->getProjection(), projectionSurface));
            }
            _pointRenderer->addElement(point);
        } else if (const std::shared_ptr<Polygon>& polygon = std::dynamic_pointer_cast<Polygon>(element)) {
            if (!polygon->getDrawData() || polygon->getDrawData()->isOffset() || polygon->getDrawData()->getProjectionSurface() != projectionSurface) {
                polygon->setDrawData(std::make_shared<PolygonDrawData>(*polygon->getGeometry(), *polygon->getStyle(), *_dataSource->getProjection(), projectionSurface));
            }
            _polygonRenderer->addElement(polygon);
        } else if (const std::shared_ptr<GeometryCollection>& geomCollection = std::dynamic_pointer_cast<GeometryCollection>(element)) {
            if (!geomCollection->getDrawData() || geomCollection->getDrawData()->isOffset() || geomCollection->getDrawData()->getProjectionSurface() != projectionSurface) {
                geomCollection->setDrawData(std::make_shared<GeometryCollectionDrawData>(*geomCollection->getGeometry(), *geomCollection->getStyle(), *_dataSource->getProjection(), projectionSurface));
            }
            _geometryCollectionRenderer->addElement(geomCollection);
        } else if (const std::shared_ptr<Polygon3D>& polygon3D = std::dynamic_pointer_cast<Polygon3D>(element)) {
            if (!polygon3D->getDrawData() || polygon3D->getDrawData()->isOffset() || polygon3D->getDrawData()->getProjectionSurface() != projectionSurface) {
                polygon3D->setDrawData(std::make_shared<Polygon3DDrawData>(*polygon3D, *polygon3D->getStyle(), *_dataSource->getProjection(), projectionSurface));
            }
            _polygon3DRenderer->addElement(polygon3D);
        } else if (const std::shared_ptr<NMLModel>& nmlModel = std::dynamic_pointer_cast<NMLModel>(element)) {
            if (!nmlModel->getDrawData() || nmlModel->getDrawData()->isOffset() || nmlModel->getDrawData()->getProjectionSurface() != projectionSurface) {
                nmlModel->setDrawData(std::make_shared<NMLModelDrawData>(*nmlModel, *nmlModel->getStyle(), *_dataSource->getProjection(), projectionSurface));
            }
            _billboardRenderer->addElement(nmlModel);
        } else if (const std::shared_ptr<Popup>& popup = std::dynamic_pointer_cast<Popup>(element)) {
            if (!popup->getDrawData() || popup->getDrawData()->isOffset() || popup->getDrawData()->getProjectionSurface() != projectionSurface) {
                if (auto options = getOptions()) {
                    popup->setDrawData(std::make_shared<PopupDrawData>(*popup, *popup->getStyle(), *_dataSource->getProjection(), projectionSurface, options, viewState));
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
        if (_billboardRenderer->getElementCount() > 0) {
            billboardsChanged = true;
        }
        return billboardsChanged;
    }
    
    bool VectorLayer::syncRendererElement(const std::shared_ptr<VectorElement>& element, const ViewState& viewState, bool remove) {
        bool visible = element->isVisible() && isVisible() && getVisibleZoomRange().inRange(viewState.getZoom());
        bool billboardsChanged = false;
        
        std::shared_ptr<ProjectionSurface> projectionSurface = viewState.getProjectionSurface();
        if (!projectionSurface) {
            return false;
        }

        // Update/remove the draw data of a single element in one of the renderers,
        if (const std::shared_ptr<Label>& label = std::dynamic_pointer_cast<Label>(element)) {
            if (visible && !remove) {
                label->setDrawData(std::make_shared<LabelDrawData>(*label, *label->getStyle(), *_dataSource->getProjection(), projectionSurface, viewState));
                _billboardRenderer->updateElement(label);
            } else {
                _billboardRenderer->removeElement(label);
            }
            billboardsChanged = true;
        } else if (const std::shared_ptr<Line>& line = std::dynamic_pointer_cast<Line>(element)) {
            if (visible && !remove) {
                line->setDrawData(std::make_shared<LineDrawData>(*line->getGeometry(), *line->getStyle(), *_dataSource->getProjection(), projectionSurface));
                _lineRenderer->updateElement(line);
            } else {
                _lineRenderer->removeElement(line);
            }
        } else if (const std::shared_ptr<Marker>& marker = std::dynamic_pointer_cast<Marker>(element)) {
            if (visible && !remove) {
                marker->setDrawData(std::make_shared<MarkerDrawData>(*marker, *marker->getStyle(), *_dataSource->getProjection(), projectionSurface));
                _billboardRenderer->updateElement(marker);
            } else {
                _billboardRenderer->removeElement(marker);
            }
            billboardsChanged = true;
        } else if (const std::shared_ptr<Point>& point = std::dynamic_pointer_cast<Point>(element)) {
            if (visible && !remove) {
                point->setDrawData(std::make_shared<PointDrawData>(*point->getGeometry(), *point->getStyle(), *_dataSource->getProjection(), projectionSurface));
                _pointRenderer->updateElement(point);
            } else {
                _pointRenderer->removeElement(point);
            }
        } else if (const std::shared_ptr<Polygon>& polygon = std::dynamic_pointer_cast<Polygon>(element)) {
            if (visible && !remove) {
                polygon->setDrawData(std::make_shared<PolygonDrawData>(*polygon->getGeometry(), *polygon->getStyle(), *_dataSource->getProjection(), projectionSurface));
                _polygonRenderer->updateElement(polygon);
            } else {
                _polygonRenderer->removeElement(polygon);
            }
        } else if (const std::shared_ptr<GeometryCollection>& geomCollection = std::dynamic_pointer_cast<GeometryCollection>(element)) {
            if (visible && !remove) {
                geomCollection->setDrawData(std::make_shared<GeometryCollectionDrawData>(*geomCollection->getGeometry(), *geomCollection->getStyle(), *_dataSource->getProjection(), projectionSurface));
                _geometryCollectionRenderer->updateElement(geomCollection);
            } else {
                _geometryCollectionRenderer->removeElement(geomCollection);
            }
        } else if (const std::shared_ptr<Polygon3D>& polygon3D = std::dynamic_pointer_cast<Polygon3D>(element)) {
            if (visible && !remove) {
                polygon3D->setDrawData(std::make_shared<Polygon3DDrawData>(*polygon3D, *polygon3D->getStyle(), *_dataSource->getProjection(), projectionSurface));
                _polygon3DRenderer->updateElement(polygon3D);
            } else {
                _polygon3DRenderer->removeElement(polygon3D);
            }
        } else if (const std::shared_ptr<NMLModel>& nmlModel = std::dynamic_pointer_cast<NMLModel>(element)) {
            if (visible && !remove) {
                nmlModel->setDrawData(std::make_shared<NMLModelDrawData>(*nmlModel, *nmlModel->getStyle(), *_dataSource->getProjection(), projectionSurface));
                _billboardRenderer->updateElement(nmlModel);
            } else {
                _billboardRenderer->removeElement(nmlModel);
            }
        } else if (const std::shared_ptr<Popup>& popup = std::dynamic_pointer_cast<Popup>(element)) {
            if (visible && !remove) {
                if (auto options = getOptions()) {
                    popup->setDrawData(std::make_shared<PopupDrawData>(*popup, *popup->getStyle(), *_dataSource->getProjection(), projectionSurface, options, viewState));
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

    std::shared_ptr<VectorLayer::FetchTask> VectorLayer::createFetchTask(const std::shared_ptr<CullState>& cullState) {
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
        _layer(layer),
        _started(false)
    {
    }
    
    void VectorLayer::FetchTask::cancel() {
        std::shared_ptr<VectorLayer> layer = _layer.lock();
        if (!layer) {
            Log::Info("VectorLayer::FetchTask: Lost connection to layer");
            return;
        }

        bool cancel = false;
        {
            std::lock_guard<std::recursive_mutex> lock(layer->_mutex);
            if (!_started) {
                cancel = true;
            }
            CancelableTask::cancel();
        }

        if (cancel) {
            layer->_fetchingTasks.remove(std::static_pointer_cast<FetchTask>(shared_from_this()));
        }
    }
    
    void VectorLayer::FetchTask::run() {
        std::shared_ptr<VectorLayer> layer = _layer.lock();
        if (!layer) {
            Log::Info("VectorLayer::FetchTask: Lost connection to layer");
            return;
        }
        
        {
            std::lock_guard<std::recursive_mutex> lock(layer->_mutex);
            if (isCanceled()) {
                return;
            }
            _started = true;
        }
        
        // Renderer access needs to be synchronized, this method may be called from multiple threads at the same time
        bool billboardsChanged = false;
        {
            std::shared_ptr<CullState> cullState;
            {
                std::lock_guard<std::recursive_mutex> lock(layer->_mutex);
                std::shared_ptr<CullState> lastCullState = layer->getLastCullState();
                if (layer->isVisible() && lastCullState && layer->getVisibleZoomRange().inRange(lastCullState->getViewState().getZoom())) {
                    cullState = lastCullState;
                }
            }
            if (cullState) {
                try {
                    billboardsChanged = loadElements(layer, cullState);
                }
                catch (const std::exception& ex) {
                    Log::Errorf("VectorLayer::FetchTask: Exception while loading elements: %s", ex.what());
                }
            } else {
                std::lock_guard<std::recursive_mutex> lock(layer->_mutex);
                billboardsChanged = layer->refreshRendererElements();
            }

            layer->_fetchingTasks.remove(std::static_pointer_cast<FetchTask>(shared_from_this()));
        }

        if (auto mapRenderer = layer->getMapRenderer()) {
            if (billboardsChanged) {
                // Billboards were added, calculate new placements
                mapRenderer->billboardsChanged();
            }
            mapRenderer->requestRedraw();
        }
    }
    
    bool VectorLayer::FetchTask::loadElements(const std::shared_ptr<VectorLayer>& layer, const std::shared_ptr<CullState>& cullState) {
        std::shared_ptr<VectorData> vectorData = layer->_dataSource->loadElements(cullState);
        if (!vectorData) {
            return false;
        }

        const ViewState& viewState = cullState->getViewState();

        std::lock_guard<std::recursive_mutex> lock(layer->_mutex);
        for (const std::shared_ptr<VectorElement>& element : vectorData->getElements()) {
            layer->addRendererElement(element, viewState);
        }
        return layer->refreshRendererElements();
    }

}
