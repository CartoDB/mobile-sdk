/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_TEXTUREMANAGER_H_
#define _CARTO_TEXTUREMANAGER_H_

#include "graphics/Texture.h"

#include <memory>
#include <mutex>
#include <thread>
#include <string>
#include <unordered_map>
#include <vector>

namespace carto {
    class TextureManager : public std::enable_shared_from_this<TextureManager> {
    public:
        TextureManager();
        virtual ~TextureManager();

        std::thread::id getGLThreadId() const;
        void setGLThreadId(std::thread::id id);
    
        std::shared_ptr<Texture> createTexture(const std::shared_ptr<Bitmap>& bitmap, bool genMipmaps, bool repeat);

        void processTextures();
    
    private:
        void deleteTexture(Texture* texture);

        std::thread::id _glThreadId;
        std::vector<std::weak_ptr<Texture> > _createQueue;
        std::vector<GLuint> _deleteTexIdQueue;
        mutable std::mutex _mutex;
    };
    
}

#endif
