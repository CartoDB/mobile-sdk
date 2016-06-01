/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NML_TEXTURE_H_
#define _CARTO_NML_TEXTURE_H_

#include "BaseTypes.h"

#include <map>
#include <memory>
#include <string>

namespace nml {
    class Texture;
    class Sampler;
}

namespace carto { namespace nmlgl {
    class Texture {
    public:
        Texture(std::shared_ptr<nml::Texture> texture);

        void create(const std::shared_ptr<GLContext>& gl);
        void dispose(const std::shared_ptr<GLContext>& gl);

        void bind(const std::shared_ptr<GLContext>& gl);

        int getTextureSize() const;

        static void uncompressTexture(nml::Texture& texture);

    private:
        static GLuint getSamplerWrapMode(int wrapMode);
        static bool hasGLExtension(const char* ext);
        
        void updateSampler(const std::shared_ptr<GLContext>& gl, bool hasSampler, nml::Sampler sampler, bool complete);
        void updateMipLevel(const std::shared_ptr<GLContext>& gl, int level, const nml::Texture& texture);
        void updateMipMaps(const std::shared_ptr<GLContext>& gl, const nml::Texture& texture);
        void uploadTexture(const std::shared_ptr<GLContext>& gl);

        int _refCount;
        std::shared_ptr<nml::Texture> _texture;

        GLuint _glTextureId;
        std::weak_ptr<GLContext> _glContext;
        int _glSize;
    };
} }

#endif
