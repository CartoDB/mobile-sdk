/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NML_GLMATERIAL_H_
#define _CARTO_NML_GLMATERIAL_H_

#include "GLBase.h"

#include <memory>
#include <map>
#include <string>

namespace carto { namespace nml {
    class Material;
    class ColorOrTexture;
    class GLTexture;
    class GLShaderManager;

    class GLMaterial final {
    public:
        explicit GLMaterial(const Material& material, const std::map<std::string, std::shared_ptr<GLTexture>>& textureMap);

        void create(GLShaderManager& shaderManager);
        void dispose();

        int getCulling() const;

        void replaceTexture(const std::string& textureId, const std::shared_ptr<GLTexture>& texture);

        void bind(const RenderState& renderState, const cglib::mat4x4<float>& mvMatrix, const cglib::mat4x4<float>& invTransMVMatrix);

    private:
        struct GLColorOrTexture {
            std::string textureId;
            std::shared_ptr<GLTexture> texture;
            cglib::vec4<float> color;

            GLColorOrTexture();
            GLColorOrTexture(const ColorOrTexture& colorOrTexture, const std::map<std::string, std::shared_ptr<GLTexture>>& textureMap);
        };

        int _type;
        int _culling;
        int _opaqueMode;
        GLColorOrTexture _emission;
        GLColorOrTexture _ambient;
        GLColorOrTexture _diffuse;
        GLColorOrTexture _transparent;
        float _transparency;
        GLColorOrTexture _specular;
        float _shininess;
        GLuint _glProgramId;
    };
} }

#endif
