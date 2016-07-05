/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_TEXTURE_H_
#define _CARTO_TEXTURE_H_

#include "graphics/utils/GLContext.h"

#include <memory>

#include <cglib/vec.h>

namespace carto {
    class Bitmap;
    class TextureManager;
    
    class Texture {
    public:
        virtual ~Texture();
        
        bool isMipmaps() const;
        
        bool isRepeat() const;
        
        std::size_t getSize() const;
        
        const cglib::vec2<float>& getTexCoordScale() const;
        
        GLuint getTexId() const;

    protected:
        friend class TextureManager;

        Texture(const std::shared_ptr<TextureManager>& textureManager, const std::shared_ptr<Bitmap>& bitmap, bool genMipmaps, bool repeat);

        void load() const;
        void unload() const;

    private:
        static const int MAX_ANISOTROPY;
        
        static const double MIPMAP_SIZE_MULTIPLIER;
    
        GLuint loadFromBitmap(const Bitmap& bitmap, bool genMipmaps, bool repeat) const;
        
        std::shared_ptr<Bitmap> _bitmap;
        bool _mipmaps;
        bool _repeat;
        
        std::size_t _sizeInBytes;
        
        cglib::vec2<float> _texCoordScale;
    
        mutable GLuint _texId;

        std::shared_ptr<TextureManager> _textureManager;
    };
    
}

#endif
