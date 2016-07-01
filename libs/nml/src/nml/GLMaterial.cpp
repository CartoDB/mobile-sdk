#include "GLMaterial.h"
#include "GLTexture.h"
#include "GLShaderManager.h"
#include "Package.h"

namespace {

    static const std::string constantVsh = R"GLSL(
        attribute vec3 aVertexPosition;
        attribute vec2 aVertexUV;
        uniform mat4 uProjMatrix;
        uniform mat4 uMVMatrix;
        varying vec2 vUV;

        void main(void) {
            vUV = aVertexUV;
            gl_Position = uProjMatrix * (uMVMatrix * vec4(aVertexPosition, 1.0));
        }
    )GLSL";

    static const std::string constantFsh = R"GLSL(
        precision mediump float;
        varying vec2 vUV;

        #ifdef EMISSION_TEXTURE
        uniform sampler2D uEmissionTex;
        #else
        uniform vec4 uEmissionColor;
        #endif

        void main(void) {
        #ifdef EMISSION_TEXTURE
            vec4 emission = texture2D(uEmissionTex, vUV);
        #else
            vec4 emission = uEmissionColor;
        #endif

            vec4 color = emission;
            if (color.a == 0.0) {
                discard;
            }
            gl_FragColor = color;
        }
    )GLSL";

    static const std::string prebakedVsh = R"GLSL(
        attribute vec3 aVertexPosition;
        attribute vec2 aVertexUV;
        attribute vec4 aVertexColor;
        uniform mat4 uProjMatrix;
        uniform mat4 uMVMatrix;
        varying vec2 vUV;
        varying lowp vec4 vColor;
    
        void main(void) {
            vUV = aVertexUV;
            vColor = aVertexColor;
            gl_Position = uProjMatrix * (uMVMatrix * vec4(aVertexPosition, 1.0));
        }
    )GLSL";
    
    static const std::string prebakedFsh = R"GLSL(
        precision mediump float;
        varying vec2 vUV;
        varying lowp vec4 vColor;
    
        #ifdef DIFFUSE_TEXTURE
        uniform sampler2D uDiffuseTex;
        #else
        uniform vec4 uDiffuseColor;
        #endif
    
        void main(void) {
        #ifdef DIFFUSE_TEXTURE
            vec4 diffuse = texture2D(uDiffuseTex, vUV);
        #else
            vec4 diffuse = uDiffuseColor;
        #endif
        
            vec4 color = diffuse * vColor;
            if (color.a == 0.0) {
                discard;
            }
            gl_FragColor = color;
        }
    )GLSL";
    
    static const std::string lambertPhongBlinnVsh = R"GLSL(
        attribute vec3 aVertexPosition;
        attribute vec2 aVertexUV;
        attribute vec3 aVertexNormal;
        uniform mat4 uProjMatrix;
        uniform mat4 uMVMatrix;
        uniform mat4 uInvTransMVMatrix;
        varying vec2 vUV;
        varying vec3 vNormal;
        varying vec3 vPos;

        void main(void) {
            vec4 pos = uMVMatrix * vec4(aVertexPosition, 1.0);
            vUV = aVertexUV;
            vNormal = vec3(uInvTransMVMatrix * vec4(aVertexNormal, 0.0));
            vPos = vec3(pos) / pos.w;
            gl_Position = uProjMatrix * pos;
        }
    )GLSL";

    static const std::string lambertPhongBlinnFsh = R"GLSL(
        #ifdef GL_FRAGMENT_PRECISION_HIGH
        precision highp float;
        #else
        precision mediump float;
        #endif
        varying vec2 vUV;
        varying vec3 vNormal;
        varying vec3 vPos;

        uniform vec4 uAmbientLightColor;
        uniform vec3 uMainLightDir;
        uniform vec4 uMainLightColor;

        #ifdef EMISSION_TEXTURE
        uniform sampler2D uEmissionTex;
        #else
        uniform vec4 uEmissionColor;
        #endif

        #ifdef AMBIENT_TEXTURE
        uniform sampler2D uAmbientTex;
        #else
        uniform vec4 uAmbientColor;
        #endif

        #ifdef DIFFUSE_TEXTURE
        uniform sampler2D uDiffuseTex;
        #else
        uniform vec4 uDiffuseColor;
        #endif

        #ifdef TRANSPARENT_TEXTURE
        uniform sampler2D uTransparentTex;
        #else
        uniform vec4 uTransparentColor;
        #endif
        uniform float uTransparency;

        #if defined(BLINN) || defined(PHONG)
        #ifdef SPECULAR_TEXTURE
        uniform sampler2D uSpecularTex;
        #else
        uniform vec4 uSpecularColor;
        #endif
        uniform float uShininess;
        #endif
    
        void main(void) {
        #ifdef EMISSION_TEXTURE
            vec4 emission = texture2D(uEmissionTex, vUV);
        #else
            vec4 emission = uEmissionColor;
        #endif

        #ifdef AMBIENT_TEXTURE
            vec4 ambient = texture2D(uAmbientTex, vUV);
        #else
            vec4 ambient = uAmbientColor;
        #endif

        #ifdef DIFFUSE_TEXTURE
            vec4 diffuse = texture2D(uDiffuseTex, vUV);
        #else
            vec4 diffuse = uDiffuseColor;
        #endif
            
            vec3 n = normalize(vNormal);
            float n_dot_l = max(dot(n, uMainLightDir), 0.0);

            vec4 color = emission + (ambient + diffuse) * uAmbientLightColor + n_dot_l * diffuse * uMainLightColor;

        #if defined(BLINN) || defined(PHONG)
            if (n_dot_l > 0.0) {
            #ifdef SPECULAR_TEXTURE
                vec4 specular = texture2D(uSpecularTex, vUV);
            #else
                vec4 specular = uSpecularColor;
            #endif

            #ifdef BLINN
                vec3 v = normalize(-vPos);
                vec3 h = normalize(v + uMainLightDir);
                float refl = pow(max(dot(h, n), 0.0), uShininess);
            #else
                vec3 v = normalize(-vPos);
                vec3 r = reflect(uMainLightDir, n);
                float refl = pow(max(dot(r, v), 0.0), uShininess);
            #endif

                color = color + refl * specular * uMainLightColor;
            }
        #endif

        #ifdef TRANSPARENT_TEXTURE
            vec4 transparent = texture2D(uTransparentTex, vUV);
        #else
            vec4 transparent = uTransparentColor;
        #endif
            vec4 opacity = vec4(1.0, 1.0, 1.0, 1.0) - transparent;
            float alpha = max(opacity.r, max(opacity.g, opacity.b));
            
            if (alpha == 0.0) {
                discard;
            }
            gl_FragColor = vec4((color * opacity).rgb * alpha, alpha);
        }
    )GLSL";

}

namespace carto { namespace nml {

    GLMaterial::GLColorOrTexture::GLColorOrTexture() :
        textureId(),
        texture(),
        color(cglib::vec4<float>::zero())
    {
    }
    
    GLMaterial::GLColorOrTexture::GLColorOrTexture(const ColorOrTexture& colorOrTexture, const std::map<std::string, std::shared_ptr<GLTexture>>& textureMap) :
        textureId(),
        texture(),
        color(cglib::vec4<float>::zero())
    {
        if (colorOrTexture.has_texture_id()) {
            textureId = colorOrTexture.texture_id();
            auto it = textureMap.find(colorOrTexture.texture_id());
            if (it != textureMap.end()) {
                texture = it->second;
            }
        }
        if (colorOrTexture.has_color()) {
            ColorRGBA c = colorOrTexture.color();
            color = cglib::vec4<float>(c.r(), c.g(),c.b(), c.a());
        }
    }
    
    GLMaterial::GLMaterial(const Material& material, const std::map<std::string, std::shared_ptr<GLTexture>>& textureMap) :
        _type(Material::CONSTANT),
        _culling(Material::NONE),
        _opaqueMode(Material::OPAQUE),
        _emission(),
        _ambient(),
        _diffuse(),
        _transparent(),
        _transparency(),
        _specular(),
        _shininess(),
        _glProgramId()
    {
        _type = material.type();
        _culling = material.culling();
        _opaqueMode = material.has_opaque_mode() ? material.opaque_mode() : Material::OPAQUE;
        _emission = material.has_emission() ? GLColorOrTexture(material.emission(), textureMap) : GLColorOrTexture();
        _ambient = material.has_ambient() ? GLColorOrTexture(material.ambient(), textureMap) : GLColorOrTexture();
        _diffuse = material.has_diffuse() ? GLColorOrTexture(material.diffuse(), textureMap) : GLColorOrTexture();
        _transparent = material.has_transparent() ? GLColorOrTexture(material.transparent(), textureMap) : GLColorOrTexture();
        _transparency = material.has_transparency() ? material.transparency() : 0.0f;
        _specular = material.has_specular() ? GLColorOrTexture(material.specular(), textureMap) : GLColorOrTexture();
        _shininess = material.has_shininess() ? material.shininess() : 0.0f;
    }
    
    void GLMaterial::create(GLShaderManager& shaderManager) {
        std::set<std::string> defs;
        if (!_emission.textureId.empty()) {
            defs.insert("EMISSION_TEXTURE");
        }
        if (!_ambient.textureId.empty()) {
            defs.insert("AMBIENT_TEXTURE");
        }
        if (!_diffuse.textureId.empty()) {
            defs.insert("DIFFUSE_TEXTURE");
        }
        if (!_transparent.textureId.empty()) {
            defs.insert("TRANSPARENT_TEXTURE");
        }
        if (!_specular.textureId.empty()) {
            defs.insert("SPECULAR_TEXTURE");
        }

        switch (_type) {
        case Material::CONSTANT:
            _glProgramId = shaderManager.createProgram(constantVsh, constantFsh, defs);
            break;
        case Material::PREBAKED:
            _glProgramId = shaderManager.createProgram(prebakedVsh, prebakedFsh, defs);
            break;
        case Material::LAMBERT:
            _glProgramId = shaderManager.createProgram(lambertPhongBlinnVsh, lambertPhongBlinnFsh, defs);
            break;
        case Material::PHONG:
            defs.insert("PHONG");
            _glProgramId = shaderManager.createProgram(lambertPhongBlinnVsh, lambertPhongBlinnFsh, defs);
            break;
        case Material::BLINN:
            defs.insert("BLINN");
            _glProgramId = shaderManager.createProgram(lambertPhongBlinnVsh, lambertPhongBlinnFsh, defs);
            break;
        default:
            assert(false);
        }
    }

    void GLMaterial::dispose() {
    }

    int GLMaterial::getCulling() const {
        return _culling;
    }

    void GLMaterial::replaceTexture(const std::string &textureId, const std::shared_ptr<GLTexture>& texture) {
        GLColorOrTexture* channels[] = {
            &_emission,
            &_ambient,
            &_diffuse,
            &_specular,
            nullptr
        };

        for (int i = 0; channels[i]; i++) {
            if (channels[i]->textureId == textureId) {
                channels[i]->texture = texture;
            }
        }
    }
    
    void GLMaterial::bind(const RenderState& renderState, const cglib::mat4x4<float>& mvMatrix, const cglib::mat4x4<float>& invTransMVMatrix) {
        glDepthMask(_opaqueMode == Material::OPAQUE ? GL_TRUE : GL_FALSE);
        
        glUseProgram(_glProgramId);
        
        glUniformMatrix4fv(glGetUniformLocation(_glProgramId, "uProjMatrix"), 1, GL_FALSE, renderState.projMatrix.data());
        glUniformMatrix4fv(glGetUniformLocation(_glProgramId, "uMVMatrix"), 1, GL_FALSE, mvMatrix.data());

        if (_culling == Material::NONE) {
            glDisable(GL_CULL_FACE);
        } else {
            glEnable(GL_CULL_FACE);
            glCullFace(_culling == Material::FRONT ? GL_FRONT : GL_BACK);
        }

        if (_type != Material::PREBAKED) {
            if (_emission.texture) {
                glUniform1i(glGetUniformLocation(_glProgramId, "uEmissionTex"), 0);
                _emission.texture->bind(0);
            } else {
                glUniform4fv(glGetUniformLocation(_glProgramId, "uEmissionColor"), 1, _emission.color.data());
            }
        }
        
        if (_type != Material::CONSTANT) {
            if (_diffuse.texture) {
                glUniform1i(glGetUniformLocation(_glProgramId, "uDiffuseTex"), 1);
                _diffuse.texture->bind(1);
            } else {
                glUniform4fv(glGetUniformLocation(_glProgramId, "uDiffuseColor"), 1, _diffuse.color.data());
            }

            if (_transparent.texture) {
                glUniform1i(glGetUniformLocation(_glProgramId, "uTransparentTex"), 2);
                _transparent.texture->bind(2);
            } else {
                glUniform4fv(glGetUniformLocation(_glProgramId, "uTransparentColor"), 1, _transparent.color.data());
            }
            glUniform1f(glGetUniformLocation(_glProgramId, "uTransparency"), 1.0f);

            if (_type != Material::PREBAKED) {
                if (_ambient.texture) {
                    glUniform1i(glGetUniformLocation(_glProgramId, "uAmbientTex"), 3);
                    _ambient.texture->bind(3);
                } else {
                    glUniform4fv(glGetUniformLocation(_glProgramId, "uAmbientColor"), 1, _ambient.color.data());
                }
                
                if (_type != Material::LAMBERT) {
                    if (_specular.texture) {
                        glUniform1i(glGetUniformLocation(_glProgramId, "uSpecularTex"), 4);
                        _specular.texture->bind(4);
                    } else {
                        glUniform4fv(glGetUniformLocation(_glProgramId, "uSpecularColor"), 1, _specular.color.data());
                    }
                    glUniform1f(glGetUniformLocation(_glProgramId, "uShininess"), _shininess);
                }

                glUniformMatrix4fv(glGetUniformLocation(_glProgramId, "uInvTransMVMatrix"), 1, GL_FALSE, invTransMVMatrix.data());
                
                glUniform4fv(glGetUniformLocation(_glProgramId, "uAmbientLightColor"), 1, renderState.ambientLightColor.data());
                glUniform4fv(glGetUniformLocation(_glProgramId, "uMainLightColor"), 1, renderState.mainLightColor.data());
                glUniform3fv(glGetUniformLocation(_glProgramId, "uMainLightDir"), 1, renderState.mainLightDir.data());
            }
        }
    }
    
} }
