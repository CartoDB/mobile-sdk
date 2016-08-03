#include "SolidRenderer.h"
#include "graphics/Bitmap.h"
#include "graphics/Shader.h"
#include "graphics/ShaderManager.h"
#include "graphics/TextureManager.h"
#include "graphics/Texture.h"
#include "graphics/ViewState.h"
#include "graphics/shaders/SolidShaderSource.h"
#include "graphics/utils/GLContext.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <cglib/mat.h>

namespace carto {

    SolidRenderer::SolidRenderer() :
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
        _u_scale(),
        _textureManager()
    {
    }
    
    SolidRenderer::~SolidRenderer() {
    }
    
    void SolidRenderer::onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager) {
        // Shader and textures must be reloaded
        _shader = shaderManager->createShader(solid_shader_source);
    
        // Get shader variables locations
        glUseProgram(_shader->getProgId());
        _u_mvpMat = _shader->getUniformLoc("u_mvpMat");
        _u_tex = _shader->getUniformLoc("u_tex");
        _u_color = _shader->getUniformLoc("u_color");
        _u_scale = _shader->getUniformLoc("u_scale");
        _a_coord = _shader->getAttribLoc("a_coord");
        _a_texCoord = _shader->getAttribLoc("a_texCoord");
    
        _textureManager = textureManager;
        
        _color = Color();
        _bitmap.reset();
        _bitmapTex.reset();
        _bitmapScale = 1.0f;
    }

    void SolidRenderer::setColor(const Color& color) {
        _color = color;
    }

    void SolidRenderer::setBitmap(const std::shared_ptr<Bitmap>& bitmap, float scale) {
        if (_bitmap != bitmap) {
            if (bitmap) {
                _bitmapTex = _textureManager->createTexture(bitmap, true, true);
            }
            else {
                auto defaultBitmap = std::make_shared<Bitmap>(DEFAULT_BITMAP, 1, 1, ColorFormat::COLOR_FORMAT_RGBA, 4);
                _bitmapTex = _textureManager->createTexture(defaultBitmap, true, true);
            }
            _bitmap = bitmap;
        }
        _bitmapScale = scale;
    }
    
    void SolidRenderer::onDrawFrame(const ViewState& viewState) {
        // Prepare for drawing
        glUseProgram(_shader->getProgId());
        // Texture, color
        glUniform1i(_u_tex, 0);
        glActiveTexture(GL_TEXTURE0);
        glUniform1f(_u_scale, _bitmapScale);
        glUniform4f(_u_color, _color.getR() / 255.0f, _color.getG() / 255.0f, _color.getB() / 255.0f, _color.getA() / 255.0f);
        // Coords, texCoords, colors
        glEnableVertexAttribArray(_a_coord);
        glEnableVertexAttribArray(_a_texCoord);

        // Texture
        glBindTexture(GL_TEXTURE_2D, _bitmapTex->getTexId());

        // Scale bitmap coordinates
        const MapPos cameraPos = viewState.getCameraPos();
        for (int i = 0; i < QUAD_VERTEX_COUNT * 3; i += 3) {
            _quadCoords[i + 0] = QUAD_COORDS[i + 0] * viewState.getWidth();
            _quadCoords[i + 1] = QUAD_COORDS[i + 1] * viewState.getHeight();
            _quadCoords[i + 2] = QUAD_COORDS[i + 2];
        }
        cglib::mat4x4<float> mvpMat = cglib::ortho4_matrix(0.0f, 0.0f, static_cast<float>(viewState.getWidth()), static_cast<float>(viewState.getHeight()), -1.0f, 1.0f);
        glUniformMatrix4fv(_u_mvpMat, 1, GL_FALSE, mvpMat.data());

        // Draw
        glVertexAttribPointer(_a_coord, 3, GL_FLOAT, GL_FALSE, 0, _quadCoords);
        glVertexAttribPointer(_a_texCoord, 2, GL_FLOAT, GL_FALSE, 0, QUAD_TEX_COORDS);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, QUAD_VERTEX_COUNT);

        // Disable bound arrays
        glDisableVertexAttribArray(_a_coord);
        glDisableVertexAttribArray(_a_texCoord);
    
        GLContext::CheckGLError("SolidRenderer::onDrawFrame");
    }
    
    void SolidRenderer::onSurfaceDestroyed() {
        _bitmap.reset();
        _bitmapTex.reset();

        _shader.reset();
        _textureManager.reset();
    }
    
    const unsigned char SolidRenderer::DEFAULT_BITMAP[] = {
        255, 255, 255, 255
    };

    const float SolidRenderer::QUAD_COORDS[] = {
        0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f
    };
    
    const float SolidRenderer::QUAD_TEX_COORDS[] = {
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f
    };

}
