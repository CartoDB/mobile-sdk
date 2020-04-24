#include "Layers.h"
#include "components/Exceptions.h"
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
    
    void Layers::clear() {
        setAll(std::vector<std::shared_ptr<Layer> >());
    }
    
    std::shared_ptr<Layer> Layers::get(int index) const {
        std::lock_guard<std::mutex> lock(_mutex);
        if (index < 0 || static_cast<std::size_t>(index) >= _layers.size()) {
            throw OutOfRangeException("Layer index out of range");
        }
        return _layers[index];
    }

    std::vector<std::shared_ptr<Layer> > Layers::getAll() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _layers;
    }

    void Layers::set(int index, const std::shared_ptr<Layer>& layer) {
        if (!layer) {
            throw NullArgumentException("Null layer");
        }

        std::shared_ptr<MapRenderer> mapRenderer;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (index < 0 || static_cast<std::size_t>(index) >= _layers.size()) {
                throw OutOfRangeException("Layer index out of range");
            }

            std::shared_ptr<Layer> oldLayer = _layers[index];
            if (std::find(_layers.begin(), _layers.end(), layer) == _layers.end()) {
                layer->setComponents(_envelopeThreadPool, _tileThreadPool, _options, _mapRenderer, _touchHandler);
            }
            _layers[index] = layer;
            if (std::find(_layers.begin(), _layers.end(), oldLayer) == _layers.end()) {
                oldLayer->setComponents(std::shared_ptr<CancelableThreadPool>(), std::shared_ptr<CancelableThreadPool>(), std::shared_ptr<Options>(), std::weak_ptr<MapRenderer>(), std::weak_ptr<TouchHandler>());
            }
        
            mapRenderer = _mapRenderer.lock();
        }
        
        if (mapRenderer) {
            mapRenderer->layerChanged(layer, false);
        }
    }
    
    void Layers::setAll(const std::vector<std::shared_ptr<Layer> >& layers) {
        if (!std::all_of(layers.begin(), layers.end(), [](const std::shared_ptr<Layer>& layer) -> bool {
            return layer.get();
        })) {
            throw NullArgumentException("Null layer");
        }

        std::shared_ptr<MapRenderer> mapRenderer;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            std::vector<std::shared_ptr<Layer> > oldLayers = _layers;
            for (const std::shared_ptr<Layer>& layer : layers) {
                if (std::find(_layers.begin(), _layers.end(), layer) == _layers.end()) {
                    layer->setComponents(_envelopeThreadPool, _tileThreadPool, _options, _mapRenderer, _touchHandler);
                }
            }
            _layers = layers;
            for (const std::shared_ptr<Layer>& oldLayer : oldLayers) {
                if (std::find(_layers.begin(), _layers.end(), oldLayer) == _layers.end()) {
                    oldLayer->setComponents(std::shared_ptr<CancelableThreadPool>(), std::shared_ptr<CancelableThreadPool>(), std::shared_ptr<Options>(), std::weak_ptr<MapRenderer>(), std::weak_ptr<TouchHandler>());
                }
            }

            mapRenderer = _mapRenderer.lock();
        }

        if (mapRenderer) {
            for (const std::shared_ptr<Layer>& layer : layers) {
                mapRenderer->layerChanged(layer, false);
            }
        }
    }
    
    void Layers::insert(int index, const std::shared_ptr<Layer>& layer) {
        if (!layer) {
            throw NullArgumentException("Null layer");
        }

        std::shared_ptr<MapRenderer> mapRenderer;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (index < 0 || static_cast<std::size_t>(index) > _layers.size()) {
                throw OutOfRangeException("Layer index out of range");
            }

            if (std::find(_layers.begin(), _layers.end(), layer) == _layers.end()) {
                layer->setComponents(_envelopeThreadPool, _tileThreadPool, _options, _mapRenderer, _touchHandler);
            }
            _layers.insert(_layers.begin() + index, layer);

            mapRenderer = _mapRenderer.lock();
        }

        if (mapRenderer) {
            mapRenderer->layerChanged(layer, false);
        }
    }

    void Layers::add(const std::shared_ptr<Layer>& layer) {
        addAll(std::vector<std::shared_ptr<Layer> > { layer });
    }

    void Layers::addAll(const std::vector<std::shared_ptr<Layer> >& layers) {
        if (!std::all_of(layers.begin(), layers.end(), [](const std::shared_ptr<Layer>& layer) -> bool {
            return layer.get();
        })) {
            throw NullArgumentException("Null layer");
        }

        std::shared_ptr<MapRenderer> mapRenderer;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            for (const std::shared_ptr<Layer>& layer : layers) {
                if (std::find(_layers.begin(), _layers.end(), layer) == _layers.end()) {
                    layer->setComponents(_envelopeThreadPool, _tileThreadPool, _options, _mapRenderer, _touchHandler);
                }
                _layers.push_back(layer);
            }
        
            mapRenderer = _mapRenderer.lock();
        }
        
        if (mapRenderer) {
            for (const std::shared_ptr<Layer>& layer : layers) {
                mapRenderer->layerChanged(layer, false);
            }
        }
    }

    bool Layers::remove(const std::shared_ptr<Layer>& layer) {
        return removeAll(std::vector<std::shared_ptr<Layer> > { layer });
    }
    
    bool Layers::removeAll(const std::vector<std::shared_ptr<Layer> >& layers) {
        if (!std::all_of(layers.begin(), layers.end(), [](const std::shared_ptr<Layer>& layer) -> bool {
            return layer.get();
        })) {
            throw NullArgumentException("Null layer");
        }

        bool removedAll = true;
        std::shared_ptr<MapRenderer> mapRenderer;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            for (const std::shared_ptr<Layer>& layer : layers) {
                auto it = std::remove(_layers.begin(), _layers.end(), layer);
                if (it == _layers.end()) {
                    removedAll = false;
                    continue;
                }
                _layers.erase(it);
                if (std::find(_layers.begin(), _layers.end(), layer) == _layers.end()) {
                    layer->setComponents(std::shared_ptr<CancelableThreadPool>(), std::shared_ptr<CancelableThreadPool>(), std::shared_ptr<Options>(), std::weak_ptr<MapRenderer>(), std::weak_ptr<TouchHandler>());
                }
            }

            mapRenderer = _mapRenderer.lock();
        }
        
        if (mapRenderer) {
            mapRenderer->requestRedraw();
        }
        return removedAll;
    }
    
    void Layers::setComponents(const std::weak_ptr<MapRenderer>& mapRenderer, const std::weak_ptr<TouchHandler>& touchHandler) {
        _mapRenderer = mapRenderer;
        _touchHandler = touchHandler;
    }
    
}
