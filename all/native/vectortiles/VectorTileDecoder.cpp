#include "VectorTileDecoder.h"
#include "vt/TileId.h"

#include <algorithm>

namespace carto {

    VectorTileDecoder::~VectorTileDecoder()
    {
    }

    void VectorTileDecoder::notifyDecoderChanged() {
        std::vector<std::shared_ptr<OnChangeListener> > onChangeListeners;
        {
            std::lock_guard<std::mutex> lock(_onChangeListenersMutex);
            onChangeListeners = _onChangeListeners;
        }
        for (const std::shared_ptr<OnChangeListener>& listener : onChangeListeners) {
            listener->onDecoderChanged();
        }
    }
        
    void VectorTileDecoder::registerOnChangeListener(const std::shared_ptr<OnChangeListener>& listener) {
        std::lock_guard<std::mutex> lock(_onChangeListenersMutex);
        _onChangeListeners.push_back(listener);
    }
        
    void VectorTileDecoder::unregisterOnChangeListener(const std::shared_ptr<OnChangeListener>& listener) {
        std::lock_guard<std::mutex> lock(_onChangeListenersMutex);
        _onChangeListeners.erase(std::remove(_onChangeListeners.begin(), _onChangeListeners.end(), listener), _onChangeListeners.end());
    }
    
    VectorTileDecoder::VectorTileDecoder() : 
        _onChangeListeners(),
        _onChangeListenersMutex()
    {
    }
    
    cglib::mat3x3<float> VectorTileDecoder::calculateTileTransform(const carto::vt::TileId& tileId, const carto::vt::TileId& targetTileId) {
        int deltaMask = (1 << (targetTileId.zoom - tileId.zoom)) - 1;
        float s = 1 << (targetTileId.zoom - tileId.zoom);
        float x = targetTileId.x & deltaMask;
        float y = targetTileId.y & deltaMask;
        return cglib::translate3_matrix(cglib::vec3<float>(-x, -y, 1)) * cglib::scale3_matrix(cglib::vec3<float>(s, s, 1));
    }

}
