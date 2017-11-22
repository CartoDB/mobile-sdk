/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NML_GLTEXTURE_H_
#define _CARTO_NML_GLTEXTURE_H_

#include "GLBase.h"

#include <map>
#include <mutex>
#include <memory>
#include <string>
#include <unordered_set>

namespace carto { namespace nml {
    class Texture;
    class Sampler;
    class GLResourceManager;

    class GLTexture final : public std::enable_shared_from_this<GLTexture> {
    public:
        explicit GLTexture(std::shared_ptr<Texture> texture);

        void create(GLResourceManager& resourceManager);

        void bind(GLResourceManager& resourceManager, int texUnit);

        int getTextureSize() const;

        static void transcodeIfNeeded(Texture& texture);

        static void registerGLExtensions();

    private:
        static GLuint getSamplerWrapMode(int wrapMode);
        static bool hasGLExtension(const char* ext);
        static void uncompressTexture(Texture& texture);

        void updateSampler(bool hasSampler, const Sampler& sampler, bool complete);
        void updateMipLevel(int level, const Texture& texture);
        void updateMipMaps(const Texture& texture);
        void uploadTexture(GLResourceManager& resourceManager);

        std::shared_ptr<Texture> _texture;

        GLuint _glTextureId;

        static std::mutex _mutex;
        static std::unordered_set<std::string> _extensions;
    };
} }

#endif
