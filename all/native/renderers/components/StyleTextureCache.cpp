#include "StyleTextureCache.h"
#include "graphics/Bitmap.h"
#include "graphics/Texture.h"
#include "graphics/TextureManager.h"

namespace carto {
    
    StyleTextureCache::StyleTextureCache(const std::shared_ptr<TextureManager>& textureManager, unsigned int capacityInBytes) :
        _textureManager(textureManager),
        _cache(capacityInBytes),
        _mutex()
    {
    }
    
    StyleTextureCache::~StyleTextureCache() {
    }
    
    unsigned int StyleTextureCache::getCapacity() const {
        std::lock_guard<std::mutex> lock(_mutex);

        return _cache.capacity();
    }
    
    void StyleTextureCache::setCapacity(unsigned int capacityInBytes) {
        std::lock_guard<std::mutex> lock(_mutex);

        _cache.resize(capacityInBytes);
    }
        
    void StyleTextureCache::setTextureManager(const std::shared_ptr<TextureManager>& textureManager) {
        std::lock_guard<std::mutex> lock(_mutex);

        _textureManager = textureManager;
    }

    std::shared_ptr<Texture> StyleTextureCache::get(const std::shared_ptr<Bitmap>& bitmap) {
        std::lock_guard<std::mutex> lock(_mutex);

        std::shared_ptr<Texture> texture;
        _cache.read(bitmap, texture);
        return texture;
    }
    
    std::shared_ptr<Texture> StyleTextureCache::create(const std::shared_ptr<Bitmap>& bitmap, bool genMipmaps, bool repeat) {
        std::lock_guard<std::mutex> lock(_mutex);

        std::shared_ptr<Texture> texture = _textureManager->createTexture(bitmap, genMipmaps, repeat);
        _cache.put(bitmap, texture, texture->getSize());
        return texture;
    }
        
}
