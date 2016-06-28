/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_STYLETEXTURECACHE_H_
#define _CARTO_STYLETEXTURECACHE_H_

#include <memory>
#include <mutex>

#include <stdext/timed_lru_cache.h>

namespace carto {
    class Bitmap;
    class Texture;
    class TextureManager;
    
    class StyleTextureCache {
    public:
        StyleTextureCache(const std::shared_ptr<TextureManager>& textureManager, unsigned int capacityInBytes);
        virtual ~StyleTextureCache();
        
        std::size_t getCapacity() const;
        void setCapacity(std::size_t capacityInBytes);

        void setTextureManager(const std::shared_ptr<TextureManager>& textureManager);
        
        std::shared_ptr<Texture> create(const std::shared_ptr<Bitmap>& bitmap, bool genMipmaps, bool repeat);
    
        std::shared_ptr<Texture> get(const std::shared_ptr<Bitmap>& bitmap);

    private:
        std::shared_ptr<TextureManager> _textureManager;

        cache::timed_lru_cache<std::shared_ptr<Bitmap>, std::shared_ptr<Texture> > _cache;
        
        mutable std::mutex _mutex;
    };
        
}

#endif
