#include "Material.h"
#include "Texture.h"

#include "nmlpackage/NMLPackage.pb.h"

namespace carto { namespace nmlgl {

    Material::ColorOrTexture::ColorOrTexture() :
        textureId(),
        texture(),
        color(cglib::vec4<float>::zero())
    {
    }
    
    Material::ColorOrTexture::ColorOrTexture(const nml::ColorOrTexture& colorOrTexture, const std::map<std::string, std::shared_ptr<Texture>>& textureMap) :
        textureId(),
        texture(),
        color(cglib::vec4<float>::zero())
    {
        if (colorOrTexture.has_texture_id()) {
            textureId = colorOrTexture.texture_id();
            auto it = textureMap.find(colorOrTexture.texture_id());
            if (it != textureMap.end())
                texture = it->second;
        }
        if (colorOrTexture.has_color()) {
            nml::ColorRGBA c = colorOrTexture.color();
            color = cglib::vec4<float>(c.r(), c.g(),c.b(), c.a());
        }
    }
    
    Material::Material(const nml::Material& material, const std::map<std::string, std::shared_ptr<Texture>>& textureMap) :
        _type(nml::Material::LAMBERT),
        _culling(nml::Material::NONE),
        _translucent(false),
        _diffuse(),
        _nullTexture()
    {
        _type = material.type();
        _culling = material.culling();
        _translucent = material.has_translucent() ? material.translucent() : false;
        _diffuse = material.has_diffuse() ? ColorOrTexture(material.diffuse(), textureMap) : ColorOrTexture();
        _nullTexture = textureMap.find("__null")->second;
    }
    
    int Material::getCulling() const {
        return _culling;
    }
    
    void Material::replaceTexture(const std::string &textureId, const std::shared_ptr<Texture>& Texture) {
        if (_diffuse.textureId == textureId) {
            _diffuse.texture = Texture;
        }
    }
    
    void Material::bind(const std::shared_ptr<GLContext>& gl) {
        glDepthMask(_translucent ? GL_FALSE : GL_TRUE);
    
        if (_culling == nml::Material::NONE) {
            glDisable(GL_CULL_FACE);
        } else {
            glEnable(GL_CULL_FACE);
            glCullFace(_culling == nml::Material::FRONT ? GL_FRONT : GL_BACK);
        }
    
        if (_diffuse.texture) {
            glUniform4f(gl->getUniformLocation("color"), 1, 1, 1, 1);
            glUniform1i(gl->getUniformLocation("tex"), 0);
            glActiveTexture(GL_TEXTURE0);
            _diffuse.texture->bind(gl);
        } else {
            glUniform4f(gl->getUniformLocation("color"), _diffuse.color[0], _diffuse.color[1], _diffuse.color[2], _diffuse.color[3]);
            glUniform1i(gl->getUniformLocation("tex"), 0);
            glActiveTexture(GL_TEXTURE0);
            _nullTexture->bind(gl);
        }
    }
    
} }
