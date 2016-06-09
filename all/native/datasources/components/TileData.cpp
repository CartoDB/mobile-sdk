#include "TileData.h"
#include "core/BinaryData.h"

namespace carto {
    
    TileData::TileData(const std::shared_ptr<BinaryData>& data) :
        _expirationTime(), _replaceWithParent(false), _data(data)
    {
    }

    TileData::~TileData() {
    }

    long long TileData::getMaxAge() const {
        std::lock_guard<std::mutex> lock(_mutex);

        if (!_expirationTime) {
            return -1;
        } else {
            long long maxAge = std::chrono::duration_cast<std::chrono::milliseconds>(*_expirationTime - std::chrono::steady_clock::now()).count();
            return maxAge > 0 ? maxAge : 0;
        }
    }

    void TileData::setMaxAge(long long maxAge) {
        std::lock_guard<std::mutex> lock(_mutex);

        if (maxAge < 0) {
            _expirationTime.reset();
        } else {
            _expirationTime = std::make_shared<std::chrono::steady_clock::time_point>(std::chrono::steady_clock::now() + std::chrono::milliseconds(maxAge));
        }
    }
    
    bool TileData::isReplaceWithParent() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _replaceWithParent;
    }
    
    void TileData::setReplaceWithParent(bool flag) {
        std::lock_guard<std::mutex> lock(_mutex);
        _replaceWithParent = flag;
    }
    
    std::shared_ptr<BinaryData> TileData::getData() const {
        std::lock_guard<std::mutex> lock(_mutex);
    return _data;
    }

    void TileData::setData(const std::shared_ptr<BinaryData>& data) {
        std::lock_guard<std::mutex> lock(_mutex);
    _data = data;
    }

}
