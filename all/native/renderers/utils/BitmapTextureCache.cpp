#include "BitmapTextureCache.h"
#include "graphics/Bitmap.h"
#include "renderers/utils/GLResourceManager.h"
#include "renderers/utils/Texture.h"
#include "utils/Log.h"

namespace carto {
    
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
        std::shared_ptr<Texture> texture;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _cache.read(bitmap, texture);
        }
        return (texture && texture->isValid() ? texture : std::shared_ptr<Texture>());
    }
    
    BitmapTextureCache::BitmapTextureCache(const std::weak_ptr<GLResourceManager>& manager, std::size_t capacityInBytes) :
        GLResource(manager),
        _cache(capacityInBytes),
        _mutex()
    {
    }
    
    std::shared_ptr<Texture> BitmapTextureCache::create(const std::shared_ptr<Bitmap>& bitmap, bool genMipmaps, bool repeat) {
        std::shared_ptr<Texture> texture;
        if (std::shared_ptr<GLResourceManager> manager = _manager.lock()) {
            texture = manager->create<Texture>(bitmap, genMipmaps, repeat);

            std::lock_guard<std::mutex> lock(_mutex);
            if (texture->getSize() > _cache.capacity()) {
                _cache.resize(texture->getSize());
            }
            _cache.put(bitmap, texture, texture->getSize());
        } else {
            Log::Error("BitmapTextureCache::create: GLResourceManager lost");
        }
        return texture;
    }
        
    void BitmapTextureCache::create() {
        std::lock_guard<std::mutex> lock(_mutex);
        _cache.clear();
    }

    void BitmapTextureCache::destroy() {
        std::lock_guard<std::mutex> lock(_mutex);
        _cache.clear();
    }

}
