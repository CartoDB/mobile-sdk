/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BITMAPTEXTURECACHE_H_
#define _CARTO_BITMAPTEXTURECACHE_H_

#include "renderers/utils/GLResource.h"

#include <memory>
#include <mutex>

#include <stdext/timed_lru_cache.h>

namespace carto {
    class Bitmap;
    class Texture;
    
    class BitmapTextureCache : public GLResource {
    public:
        virtual ~BitmapTextureCache();
        
        std::size_t getCapacity() const;
        void setCapacity(std::size_t capacityInBytes);

        void clear();

        std::shared_ptr<Texture> get(const std::shared_ptr<Bitmap>& bitmap) const;
        std::shared_ptr<Texture> create(const std::shared_ptr<Bitmap>& bitmap, bool genMipmaps, bool repeat);
    
    protected:
        friend GLResourceManager;

        BitmapTextureCache(const std::weak_ptr<GLResourceManager>& manager, std::size_t capacityInBytes);

        virtual void create();
        virtual void destroy();

    private:
        mutable cache::timed_lru_cache<std::shared_ptr<Bitmap>, std::shared_ptr<Texture> > _cache;
        
        mutable std::mutex _mutex;
    };
        
}

#endif
