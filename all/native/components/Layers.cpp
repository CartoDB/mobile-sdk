#include "Layers.h"
#include "layers/Layer.h"
#include "renderers/MapRenderer.h"
#include "utils/Log.h"

namespace carto {

    Layers::Layers(const std::shared_ptr<CancelableThreadPool>& envelopeThreadPool,
                   const std::shared_ptr<CancelableThreadPool>& tileThreadPool,
                   const std::weak_ptr<Options>& options) :
        _layers(),
        _envelopeThreadPool(envelopeThreadPool),
        _tileThreadPool(tileThreadPool),
        _options(options),
        _mapRenderer(),
        _touchHandler(),
        _mutex()
    {
    }
    
    Layers::~Layers() {
    }
    
    int Layers::count() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return static_cast<int>(_layers.size());
    }
    
    std::shared_ptr<Layer> Layers::get(int index) const {
        std::lock_guard<std::mutex> lock(_mutex);
        if (index < 0 && static_cast<std::size_t>(index) >= _layers.size()) {
            Log::Error("Layers::get: Layer index out of range");
            return std::shared_ptr<Layer>();
        }
        return _layers[index];
    }

    void Layers::set(int index, const std::shared_ptr<Layer>& layer) {
        std::shared_ptr<MapRenderer> mapRenderer;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (index < 0 && static_cast<std::size_t>(index) >= _layers.size()) {
                Log::Error("Layers::set: Layer index out of range");
                return;
            }

            _layers[index]->setComponents(std::shared_ptr<CancelableThreadPool>(), std::shared_ptr<CancelableThreadPool>(), std::shared_ptr<Options>(), std::weak_ptr<MapRenderer>(), std::weak_ptr<TouchHandler>());
            layer->setComponents(_envelopeThreadPool, _tileThreadPool, _options, _mapRenderer, _touchHandler);
            _layers[index] = layer;
        
            mapRenderer = _mapRenderer.lock();
        }
        
        if (mapRenderer) {
            mapRenderer->layerChanged(layer, false);
        }
    }
    
    void Layers::clear() {
        setAll(std::vector<std::shared_ptr<Layer> >());
    }
    
    void Layers::insert(int index, const std::shared_ptr<Layer>& layer) {
        std::shared_ptr<MapRenderer> mapRenderer;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (index < 0 && static_cast<std::size_t>(index) > _layers.size()) {
                Log::Error("Layers::insert: Layer index out of range");
                return;
            }
            layer->setComponents(_envelopeThreadPool, _tileThreadPool, _options, _mapRenderer, _touchHandler);
            _layers.insert(_layers.begin() + index, layer);

            mapRenderer = _mapRenderer.lock();
        }

        if (mapRenderer) {
            mapRenderer->layerChanged(layer, false);
        }
    }

    void Layers::add(const std::shared_ptr<Layer>& layer) {
        std::shared_ptr<MapRenderer> mapRenderer;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            layer->setComponents(_envelopeThreadPool, _tileThreadPool, _options, _mapRenderer, _touchHandler);
            _layers.push_back(layer);
        
            mapRenderer = _mapRenderer.lock();
        }
        
        if (mapRenderer) {
            mapRenderer->layerChanged(layer, false);
        }
    }
    
    bool Layers::remove(const std::shared_ptr<Layer>& layer) {
        std::shared_ptr<MapRenderer> mapRenderer;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            layer->setComponents(std::shared_ptr<CancelableThreadPool>(), std::shared_ptr<CancelableThreadPool>(), std::shared_ptr<Options>(), std::weak_ptr<MapRenderer>(), std::weak_ptr<TouchHandler>());
        
            std::vector<std::shared_ptr<Layer> >::iterator it(std::remove(_layers.begin(), _layers.end(), layer));
            if (it == _layers.end()) {
                return false;
            }
            _layers.erase(it, _layers.end());

            mapRenderer = _mapRenderer.lock();
        }
        
        if (mapRenderer) {
            mapRenderer->requestRedraw();
        }
        return true;
    }
    
    std::vector<std::shared_ptr<Layer> > Layers::getAll() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _layers;
    }

    void Layers::setAll(const std::vector<std::shared_ptr<Layer> >& layers) {
        std::shared_ptr<MapRenderer> mapRenderer;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            for (const std::shared_ptr<Layer>& layer : _layers) {
                if (std::find(layers.begin(), layers.end(), layer) == layers.end()) {
                    layer->setComponents(std::shared_ptr<CancelableThreadPool>(), std::shared_ptr<CancelableThreadPool>(), std::shared_ptr<Options>(), std::weak_ptr<MapRenderer>(), std::weak_ptr<TouchHandler>());
                }
            }
            for (const std::shared_ptr<Layer>& layer : layers) {
                if (std::find(_layers.begin(), _layers.end(), layer) == _layers.end()) {
                    layer->setComponents(_envelopeThreadPool, _tileThreadPool, _options, _mapRenderer, _touchHandler);
                }
            }
            _layers = layers;

            mapRenderer = _mapRenderer.lock();
        }

        if (mapRenderer) {
            for (const std::shared_ptr<Layer>& layer : layers) {
                mapRenderer->layerChanged(layer, false);
            }
        }
    }
    
    void Layers::setComponents(const std::weak_ptr<MapRenderer>& mapRenderer, const std::weak_ptr<TouchHandler>& touchHandler) {
        _mapRenderer = mapRenderer;
        _touchHandler = touchHandler;
    }
    
}
