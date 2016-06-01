/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NML_MATERIAL_H_
#define _CARTO_NML_MATERIAL_H_

#include "BaseTypes.h"

#include <memory>
#include <map>
#include <string>

namespace nml {
    class Material;
    class ColorOrTexture;
}

namespace carto { namespace nmlgl {
    class Texture;

    class Material {
    public:
        Material(const nml::Material& material, const std::map<std::string, std::shared_ptr<Texture>>& textureMap);

        void replaceTexture(const std::string& textureId, const std::shared_ptr<Texture>& glTexture);

        void bind(const std::shared_ptr<GLContext>& gl);

        int getCulling() const;

    private:

        struct ColorOrTexture {
            std::string textureId;
            std::shared_ptr<Texture> texture;
            cglib::vec4<float> color;

            ColorOrTexture();
            ColorOrTexture(const nml::ColorOrTexture& colorOrTexture, const std::map<std::string, std::shared_ptr<Texture>>& textureMap);
        };

        int _type;
        int _culling;
        bool _translucent;
        ColorOrTexture _diffuse;
        std::shared_ptr<Texture> _nullTexture;
    };
} }

#endif
