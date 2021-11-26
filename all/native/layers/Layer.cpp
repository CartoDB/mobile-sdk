#include "Layer.h"
#include "components/Exceptions.h"
#include "components/Options.h"
#include "graphics/Bitmap.h"
#include "graphics/ViewState.h"
#include "renderers/MapRenderer.h"
#include "renderers/components/RayIntersectedElement.h"
#include "renderers/components/RayIntersectedElementComparator.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <limits>

namespace carto {

    Layer::~Layer() {
    }
    
    std::map<std::string, Variant> Layer::getMetaData() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _metaData;
    }
        
    void Layer::setMetaData(const std::map<std::string, Variant>& metaData) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _metaData = metaData;
    }

    bool Layer::containsMetaDataKey(const std::string& key) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _metaData.find(key) != _metaData.end();
    }
    
    Variant Layer::getMetaDataElement(const std::string& key) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        auto it = _metaData.find(key);
        if (it == _metaData.end()) {
            return Variant();
        }
        return it->second;
    }
    
    void Layer::setMetaDataElement(const std::string& key, const Variant& element) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _metaData[key] = element;
    }
    
    int Layer::getUpdatePriority() const {
        return _updatePriority.load();
    }
    
    void Layer::setUpdatePriority(int priority) {
        _updatePriority.store(priority);
    }
    
    int Layer::getCullDelay() const {
        return _cullDelay.load();
    }

    void Layer::setCullDelay(int cullDelay) {
        _cullDelay.store(std::max(0, cullDelay));
    }
        
    float Layer::getOpacity() const {
        return _opacity.load();
    }
    
    void Layer::setOpacity(float opacity) {
        _opacity.store(std::max(0.0f, std::min(1.0f, opacity)));
        refresh();
    }
    
    bool Layer::isVisible() const {
        return _visible.load();
    }
    
    void Layer::setVisible(bool visible) {
        _visible.store(visible);
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
        std::shared_ptr<CullState> cullState = getLastCullState();
        if (cullState) {
            // Reload data using the last known cull state.
            loadData(cullState);
        } else {
            // Last cullstate not known yet. Let renderer do async update.
            if (auto mapRenderer = getMapRenderer()) {
                mapRenderer->layerChanged(shared_from_this(), false);
                mapRenderer->requestRedraw();
            }
        }
    }

    void Layer::simulateClick(ClickType::ClickType clickType, const ScreenPos& screenPos, const ViewState& viewState) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        auto options = getOptions();
        if (!options) {
            return;
        }
        if (!viewState.getProjectionSurface()) {
            return;
        }

        cglib::vec3<double> target = viewState.screenToWorld(cglib::vec2<float>(screenPos.getX(), screenPos.getY()), 0, options);
        if (std::isnan(cglib::norm(target))) {
            return; // point not on surface
        }
        cglib::vec3<double> origin = viewState.getCameraPos();
        cglib::ray3<double> ray(origin, target - origin);

        // Calculate intersections
        std::vector<RayIntersectedElement> results;
        calculateRayIntersectedElements(ray, viewState, results);

        // Sort the results but do 'reverse stable sort' to be consistent with the rendering order
        std::stable_sort(results.begin(), results.end(), RayIntersectedElementComparator(viewState));
        std::reverse(results.begin(), results.end());

        // Send click events
        for (const RayIntersectedElement& intersectedElement : results) {
            ClickInfo clickInfo(clickType, 0);
            if (intersectedElement.getLayer()->processClick(clickInfo, intersectedElement, viewState)) {
                return;
            }
        }
    }
    
    Layer::Layer() :
        _envelopeThreadPool(),
        _tileThreadPool(),
        _mutex(),
        _updatePriority(0),
        _cullDelay(DEFAULT_CULL_DELAY),
        _opacity(1.0f),
        _visible(true),
        _visibleZoomRange(0, std::numeric_limits<float>::infinity()),
        _metaData(),
        _lastCullState(),
        _options(),
        _mapRenderer(),
        _touchHandler()
    {
    }
    
    void Layer::setComponents(const std::shared_ptr<CancelableThreadPool>& envelopeThreadPool,
                              const std::shared_ptr<CancelableThreadPool>& tileThreadPool,
                              const std::weak_ptr<Options>& options,
                              const std::weak_ptr<MapRenderer>& mapRenderer,
                              const std::weak_ptr<TouchHandler>& touchHandler)
    {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        if (mapRenderer.lock() == _mapRenderer.lock()) {
            return;
        } else if (mapRenderer.lock() && _mapRenderer.lock()) {
            throw InvalidArgumentException("Layer already attached to a different renderer");
        }

        // This method is called only when the layer is added/removed from Layers object,
        // access to these threadpools is thread safe
        _envelopeThreadPool = envelopeThreadPool;
        _tileThreadPool = tileThreadPool;
        _mapRenderer = mapRenderer;
        _touchHandler = touchHandler;
        _options = options;
    
        // Let the datasource know, that this layer is using it / not using it anymore, so it can
        // notify this layer when the data changes
        if (mapRenderer.lock()) {
            registerDataSourceListener();
        } else {
            unregisterDataSourceListener();
        }
    }
    
    std::shared_ptr<CullState> Layer::getLastCullState() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _lastCullState;
    }

    std::shared_ptr<Options> Layer::getOptions() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _options.lock();
    }

    std::shared_ptr<MapRenderer> Layer::getMapRenderer() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _mapRenderer.lock();
    }

    std::shared_ptr<TouchHandler> Layer::getTouchHandler() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _touchHandler.lock();
    }

    void Layer::redraw() const {
        if (auto mapRenderer = getMapRenderer()) {
            mapRenderer->requestRedraw();
        }
    }
    
    bool Layer::onDrawFrame3D(float deltaSeconds, BillboardSorter& billboardSorter, const ViewState& viewState) {
        return false;
    }
    
    std::shared_ptr<Bitmap> Layer::getBackgroundBitmap() const {
        return std::shared_ptr<Bitmap>();
    }

    std::shared_ptr<Bitmap> Layer::getSkyBitmap() const {
        return std::shared_ptr<Bitmap>();
    }

    const int Layer::DEFAULT_CULL_DELAY = 400;

}
