/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NML_GLTEXTURE_H_
#define _CARTO_NML_GLTEXTURE_H_

#include "GLBase.h"

#include <map>
#include <memory>
#include <string>

namespace carto { namespace nml {
    class Texture;
    class Sampler;

    class GLTexture final {
    public:
        explicit GLTexture(std::shared_ptr<Texture> texture);

        void create();
        void dispose();

        void bind(int texUnit);

        int getTextureSize() const;

        static void uncompressTexture(Texture& texture);

    private:
        static GLuint getSamplerWrapMode(int wrapMode);
        static bool hasGLExtension(const char* ext);
        
        void updateSampler(bool hasSampler, const Sampler& sampler, bool complete);
        void updateMipLevel(int level, const Texture& texture);
        void updateMipMaps(const Texture& texture);
        void uploadTexture();

        int _refCount;
        std::shared_ptr<Texture> _texture;

        GLuint _glTextureId;
    };
} }

#endif
