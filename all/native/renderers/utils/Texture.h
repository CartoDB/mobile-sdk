/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_TEXTURE_H_
#define _CARTO_TEXTURE_H_

#include "renderers/utils/GLResource.h"

#include <memory>

#include <cglib/vec.h>

namespace carto {
    class Bitmap;
    
    class Texture : public GLResource {
    public:
        virtual ~Texture();
        
        bool isMipmaps() const;
        
        bool isRepeat() const;
        
        std::size_t getSize() const;
        
        const cglib::vec2<float>& getTexCoordScale() const;

        GLuint getTexId() const;

    protected:
        friend GLResourceManager;

        Texture(const std::weak_ptr<GLResourceManager>& manager, const std::shared_ptr<Bitmap>& bitmap, bool genMipmaps, bool repeat);

        virtual void create();
        virtual void destroy();

    private:
        static const int MAX_ANISOTROPY;
        
        static const double MIPMAP_SIZE_MULTIPLIER;
    
        static GLuint LoadFromBitmap(const Bitmap& bitmap, bool genMipmaps, bool repeat);
        
        std::shared_ptr<Bitmap> _bitmap;
        bool _mipmaps;
        bool _repeat;
        std::size_t _sizeInBytes;
        cglib::vec2<float> _texCoordScale;
    
        GLuint _texId;
    };
    
}

#endif
