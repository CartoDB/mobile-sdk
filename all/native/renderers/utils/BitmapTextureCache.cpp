#include "BitmapTextureCache.h"
#include "graphics/Bitmap.h"
#include "renderers/utils/GLResourceManager.h"
#include "renderers/utils/Texture.h"

namespace carto {
    
    BitmapTextureCache::BitmapTextureCache(const std::shared_ptr<GLResourceManager>& manager, std::size_t capacityInBytes) :
        GLResource(manager),
        _cache(capacityInBytes),
        _mutex()
    {
    }
    
    BitmapTextureCache::~BitmapTextureCache() {
    }
    
    std::size_t BitmapTextureCache::getCapacity() const {
        std::lock_guard<std::mutex> lock(_mutex);

        return _cache.capacity();
    }
    
    void BitmapTextureCache::setCapacity(std::size_t capacityInBytes) {
        std::lock_guard<std::mutex> lock(_mutex);

        _cache.resize(capacityInBytes);
    }

    void BitmapTextureCache::clear() {
        std::lock_guard<std::mutex> lock(_mutex);

        _cache.clear();
    }
        
    std::shared_ptr<Texture> BitmapTextureCache::get(const std::shared_ptr<Bitmap>& bitmap) const {
        std::lock_guard<std::mutex> lock(_mutex);

        std::shared_ptr<Texture> texture;
        _cache.read(bitmap, texture);
        return (texture && texture->isValid() ? texture : std::shared_ptr<Texture>());
    }
    
    std::shared_ptr<Texture> BitmapTextureCache::create(const std::shared_ptr<Bitmap>& bitmap, bool genMipmaps, bool repeat) {
        std::lock_guard<std::mutex> lock(_mutex);

        std::shared_ptr<Texture> texture = _manager->create<Texture>(bitmap, genMipmaps, repeat);
        if (texture->getSize() > _cache.capacity()) {
            _cache.resize(texture->getSize());
        }
        _cache.put(bitmap, texture, texture->getSize());
        return texture;
    }
        
    void BitmapTextureCache::create() const {
        std::lock_guard<std::mutex> lock(_mutex);

        _cache.clear();
    }

    void BitmapTextureCache::destroy() const {
        std::lock_guard<std::mutex> lock(_mutex);

        _cache.clear();
    }

}
