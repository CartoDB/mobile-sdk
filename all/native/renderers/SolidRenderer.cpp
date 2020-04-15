#include "SolidRenderer.h"
#include "graphics/Bitmap.h"
#include "graphics/ViewState.h"
#include "renderers/MapRenderer.h"
#include "renderers/utils/GLResourceManager.h"
#include "renderers/utils/Shader.h"
#include "renderers/utils/Texture.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <cglib/mat.h>

namespace carto {

    SolidRenderer::SolidRenderer() :
        _mapRenderer(),
        _color(),
        _bitmap(),
        _bitmapTex(),
        _bitmapScale(1.0f),
        _shader(),
        _a_coord(0),
        _a_texCoord(0),
        _u_mvpMat(0),
        _u_tex(0),
        _u_color(0),
        _mutex()
    {
    }
    
    SolidRenderer::~SolidRenderer() {
    }
    
    void SolidRenderer::setComponents(const std::weak_ptr<Options>& options, const std::weak_ptr<MapRenderer>& mapRenderer) {
        std::lock_guard<std::mutex> lock(_mutex);

        _mapRenderer = mapRenderer;
        _shader.reset();
        _bitmapTex.reset();
    }

    void SolidRenderer::setColor(const Color& color) {
        std::lock_guard<std::mutex> lock(_mutex);

        _color = color;
    }

    void SolidRenderer::setBitmap(const std::shared_ptr<Bitmap>& bitmap, float scale) {
        std::lock_guard<std::mutex> lock(_mutex);

        if (_bitmap != bitmap) {
            _bitmapTex.reset();
        }
        _bitmap = bitmap;
        _bitmapScale = scale;
    }
    
    void SolidRenderer::onDrawFrame(const ViewState& viewState) {
        std::lock_guard<std::mutex> lock(_mutex);

        if (!initializeRenderer()) {
            return;
        }

        // Prepare for drawing
        glUseProgram(_shader->getProgId());
        // Texture, color
        glUniform1i(_u_tex, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _bitmapTex->getTexId());

        float alpha = _color.getA() / 255.0f;
        glUniform4f(_u_color, _color.getR() * alpha / 255.0f, _color.getG() * alpha / 255.0f, _color.getB() * alpha / 255.0f, alpha);
        // Coords, texCoords, colors
        glEnableVertexAttribArray(_a_coord);
        glEnableVertexAttribArray(_a_texCoord);

        // Scale bitmap coordinates
        float bitmapWScale = 1.0f, bitmapHScale = 1.0f;
        if (_bitmap) {
            bitmapWScale = 0.5f / viewState.getWidth()  * _bitmap->getWidth()  * _bitmapScale;
            bitmapHScale = 0.5f / viewState.getHeight() * _bitmap->getHeight() * _bitmapScale;
        }
        for (int i = 0; i < 4; i++) {
            _quadTexCoords[i * 2 + 0] = QUAD_TEX_COORDS[i * 2 + 0] / bitmapWScale;
            _quadTexCoords[i * 2 + 1] = QUAD_TEX_COORDS[i * 2 + 1] / bitmapHScale;
        }
        cglib::mat4x4<float> mvpMat = cglib::mat4x4<float>::identity();
        glUniformMatrix4fv(_u_mvpMat, 1, GL_FALSE, mvpMat.data());

        // Draw
        glVertexAttribPointer(_a_coord, 3, GL_FLOAT, GL_FALSE, 0, QUAD_COORDS);
        glVertexAttribPointer(_a_texCoord, 2, GL_FLOAT, GL_FALSE, 0, _quadTexCoords);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Disable bound arrays
        glDisableVertexAttribArray(_a_coord);
        glDisableVertexAttribArray(_a_texCoord);
    
        GLContext::CheckGLError("SolidRenderer::onDrawFrame");
    }
    
    bool SolidRenderer::initializeRenderer() {
        static const Shader::Source shaderSource("solid", SOLID_VERTEX_SHADER, SOLID_FRAGMENT_SHADER);
        
        if (_shader && _shader->isValid() && _bitmapTex && _bitmapTex->isValid()) {
            return true;
        }

        if (auto mapRenderer = _mapRenderer.lock()) {
            // Shader and textures must be reloaded
            _shader = mapRenderer->getGLResourceManager()->create<Shader>(shaderSource);
        
            // Get shader variables locations
            _u_mvpMat = _shader->getUniformLoc("u_mvpMat");
            _u_tex = _shader->getUniformLoc("u_tex");
            _u_color = _shader->getUniformLoc("u_color");
            _a_coord = _shader->getAttribLoc("a_coord");
            _a_texCoord = _shader->getAttribLoc("a_texCoord");

            if (_bitmap) {
                _bitmapTex = mapRenderer->getGLResourceManager()->create<Texture>(_bitmap, true, true);
            } else {
                auto defaultBitmap = std::make_shared<Bitmap>(DEFAULT_BITMAP, 1, 1, ColorFormat::COLOR_FORMAT_RGBA, 4);
                _bitmapTex = mapRenderer->getGLResourceManager()->create<Texture>(defaultBitmap, true, true);
            }
        }

        return _shader && _shader->isValid() && _bitmapTex && _bitmapTex->isValid();
    }
    
    const unsigned char SolidRenderer::DEFAULT_BITMAP[] = {
        255, 255, 255, 255
    };

    const float SolidRenderer::QUAD_COORDS[] = {
        -1.0f, 1.0f, 0.0f, -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f, 0.0f
    };
    
    const float SolidRenderer::QUAD_TEX_COORDS[] = {
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f
    };

    const std::string SolidRenderer::SOLID_VERTEX_SHADER = R"GLSL(
        #version 100
        attribute vec4 a_coord;
        attribute vec2 a_texCoord;
        varying vec2 v_texCoord;
        uniform mat4 u_mvpMat;
        void main() {
            v_texCoord = a_texCoord;
            gl_Position = u_mvpMat * a_coord;
        }
    )GLSL";

    const std::string SolidRenderer::SOLID_FRAGMENT_SHADER = R"GLSL(
        #version 100
        precision mediump float;
        varying mediump vec2 v_texCoord;
        uniform sampler2D u_tex;
        uniform vec4 u_color;
        void main() {
            gl_FragColor = texture2D(u_tex, v_texCoord) * u_color;
        }
    )GLSL";

}
