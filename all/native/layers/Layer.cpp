#include "Layer.h"
#include "renderers/MapRenderer.h"
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
    
    Layer::Layer() :
        _envelopeThreadPool(),
        _tileThreadPool(),
        _options(),
        _mapRenderer(),
        _lastCullState(),
        _updatePriority(0),
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
    
    int Layer::getCullDelay() const {
        return CULL_DELAY;
    }
        
    void Layer::onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager) {
        _surfaceCreated = true;
    }
    
    bool Layer::onDrawFrame3D(float deltaSeconds, BillboardSorter& BillboardSorter, StyleTextureCache& styleCache, const ViewState& viewState) {
        return false;
    }
    
    void Layer::onSurfaceDestroyed() {
        _surfaceCreated = false;
    }
    
}
