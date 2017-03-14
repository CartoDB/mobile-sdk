#include "Layer.h"
#include "renderers/MapRenderer.h"
#include "renderers/components/RayIntersectedElement.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <limits>

namespace carto {

    Layer::~Layer() {
    }
    
    int Layer::getUpdatePriority() const {
        return _updatePriority;
    }
    
    void Layer::setUpdatePriority(int priority) {
        _updatePriority = priority;
    }
    
    int Layer::getCullDelay() const {
        return _cullDelay;
    }

    void Layer::setCullDelay(int cullDelay) {
        _cullDelay = cullDelay;
    }
        
    bool Layer::isSurfaceCreated() {
        return _surfaceCreated;
    }
    
    bool Layer::isVisible() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _visible;
    }
    
    void Layer::setVisible(bool visible) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _visible = visible;
        }
        refresh();
    }
    
    MapRange Layer::getVisibleZoomRange() {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _visibleZoomRange;
    }
    
    void Layer::setVisibleZoomRange(const MapRange& range) {
        {
            std::lock_guard<std::recursive_mutex> lock(_mutex);
            _visibleZoomRange = range;
        }
        refresh();
    }
        
    void Layer::update(const std::shared_ptr<CullState>& cullState) {
        // Load data
        loadData(cullState);
        
        // Save current cull state, so it can be use later to reload data
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _lastCullState = cullState;
    }
    
    void Layer::refresh() {
        // Reload data using the last known cull state
        const std::shared_ptr<CullState>& cullState = getLastCullState();
        if (cullState) {
            loadData(cullState);
        }
    }

    void Layer::simulateClick(ClickType::ClickType clickType, const ScreenPos& screenPos) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        auto mapRenderer = _mapRenderer.lock();
        auto options = _options.lock();
        if (!mapRenderer || !options) {
            return;
        }

        ViewState viewState = mapRenderer->getViewState();
        std::shared_ptr<Projection> projection = options->getBaseProjection();

        MapPos worldPos = viewState.screenToWorldPlane(screenPos, options);
        MapPos rayOrigin = viewState.getCameraPos();
        MapVec rayDir = worldPos - viewState.getCameraPos();
        cglib::ray3<double> ray(cglib::vec3<double>(rayOrigin.getX(), rayOrigin.getY(), rayOrigin.getZ()), cglib::vec3<double>(rayDir.getX(), rayDir.getY(), rayDir.getZ()));

        // Calculate intersections
        std::vector<RayIntersectedElement> results;
        calculateRayIntersectedElements(*projection, ray, viewState, results);

        // Sort the results
        auto distanceComparator = [&viewState](const RayIntersectedElement& element1, const RayIntersectedElement& element2) -> bool {
            if (element1.is3D() != element2.is3D()) {
                return element1.is3D() > element2.is3D();
            }
            if (element1.is3D()) {
                double deltaDistance = element1.getDistance(viewState.getCameraPos()) - element2.getDistance(viewState.getCameraPos());
                if (deltaDistance != 0) {
                    return deltaDistance < 0;
                }
            }
            return element1.getOrder() > element2.getOrder();
        };

        std::sort(results.begin(), results.end(), distanceComparator);

        // Send click events
        for (const RayIntersectedElement& intersectedElement : results) {
            if (intersectedElement.getLayer()->processClick(clickType, intersectedElement, viewState)) {
                return;
            }
        }
    }
    
    Layer::Layer() :
        _envelopeThreadPool(),
        _tileThreadPool(),
        _options(),
        _mapRenderer(),
        _lastCullState(),
        _updatePriority(0),
        _cullDelay(DEFAULT_CULL_DELAY),
        _visible(true),
        _visibleZoomRange(0, std::numeric_limits<float>::infinity()),
        _mutex(),
        _surfaceCreated(false)
    {
    }
    
    void Layer::setComponents(const std::shared_ptr<CancelableThreadPool>& envelopeThreadPool,
                              const std::shared_ptr<CancelableThreadPool>& tileThreadPool,
                              const std::weak_ptr<Options>& options,
                              const std::weak_ptr<MapRenderer>& mapRenderer,
                              const std::weak_ptr<TouchHandler>& touchHandler) {
        // This method is called only when the layer is added/removed from Layers object,
        // access to these threadpools is thread safe
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _envelopeThreadPool = envelopeThreadPool;
        _tileThreadPool = tileThreadPool;
        _mapRenderer = mapRenderer;
        _touchHandler = touchHandler;
        _options = options;
    
        // Let the datasource know, that this layer is using it / not using it anymore, so it can
        // notify this layer when the data changes
        if (envelopeThreadPool && tileThreadPool && options.lock() && mapRenderer.lock()) {
            registerDataSourceListener();
        } else {
            unregisterDataSourceListener();
        }
    }
    
    std::shared_ptr<CullState> Layer::getLastCullState() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _lastCullState;
    }
    
    void Layer::onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager) {
        _surfaceCreated = true;
    }
    
    bool Layer::onDrawFrame3D(float deltaSeconds, BillboardSorter& billboardSorter, StyleTextureCache& styleCache, const ViewState& viewState) {
        return false;
    }
    
    void Layer::onSurfaceDestroyed() {
        _surfaceCreated = false;
    }
    
}
