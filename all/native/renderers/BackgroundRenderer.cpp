#include "BackgroundRenderer.h"
#include "components/Options.h"
#include "graphics/Bitmap.h"
#include "graphics/Shader.h"
#include "graphics/ShaderManager.h"
#include "graphics/TextureManager.h"
#include "graphics/shaders/TexturedShaderSource.h"
#include "graphics/Texture.h"
#include "graphics/ViewState.h"
#include "utils/Const.h"
#include "utils/GLES2.h"
#include "utils/GLUtils.h"
#include "utils/Log.h"

#include <cglib/mat.h>

namespace carto {

    BackgroundRenderer::BackgroundRenderer(const Options& options) :
        _backgroundBitmap(),
        _backgroundTex(),
        _skyBitmap(),
        _skyTex(),
        _shader(),
        _u_tex(0),
        _u_mvpMat(0),
        _a_coord(0),
        _a_texCoord(0),
        _textureManager(),
        _options(options)
    {
    }
    
    BackgroundRenderer::~BackgroundRenderer() {
    }
    
    void BackgroundRenderer::onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager) {
        // Shader and textures must be reloaded
        _shader = shaderManager->createShader(textured_shader_source);
    
        // Get shader variables locations
        glUseProgram(_shader->getProgId());
        _u_tex = _shader->getUniformLoc("u_tex");
        _u_mvpMat = _shader->getUniformLoc("u_mvpMat");
        _a_coord = _shader->getAttribLoc("a_coord");
        _a_texCoord = _shader->getAttribLoc("a_texCoord");
    
        _textureManager = textureManager;
        _backgroundBitmap.reset();
        _backgroundTex.reset();
        _skyBitmap.reset();
        _skyTex.reset();
    }
    
    void BackgroundRenderer::onDrawFrame(const ViewState& viewState) {
        std::shared_ptr<Bitmap> backgroundBitmap(_options.getBackgroundBitmap());
        if (_backgroundBitmap != backgroundBitmap) {
            if (backgroundBitmap) {
                _backgroundTex = _textureManager->createTexture(backgroundBitmap, true, true);
            } else {
                _backgroundTex.reset();
            }
            _backgroundBitmap = backgroundBitmap;
        }
    
        std::shared_ptr<Bitmap> skyBitmap(_options.getSkyBitmap());
        if (_skyBitmap != skyBitmap) {
            if (skyBitmap) {
                _skyTex = _textureManager->createTexture(skyBitmap, true, true);
            }
            else {
                _skyTex.reset();
            }
           _skyBitmap = skyBitmap;
        }
    
        if (_skyTex || _backgroundTex) {
            // Prepare for drawing
            glUseProgram(_shader->getProgId());
            // Texture
            glUniform1i(_u_tex, 0);
            // Matrix
            const cglib::mat4x4<float>& mvpMat = viewState.getRTEModelviewProjectionMat();
            glUniformMatrix4fv(_u_mvpMat, 1, GL_FALSE, mvpMat.data());
            // Coords, texCoords, colors
            glEnableVertexAttribArray(_a_coord);
            glEnableVertexAttribArray(_a_texCoord);
    
            drawSky(viewState);
            drawBackground(viewState);
    
            // Disable bound arrays
            glDisableVertexAttribArray(_a_coord);
            glDisableVertexAttribArray(_a_texCoord);
        }
    
        GLUtils::checkGLError("BackgroundRenderer::onDrawFrame");
    }
    
    void BackgroundRenderer::onSurfaceDestroyed() {
        _backgroundBitmap.reset();
        _backgroundTex.reset();
        _skyBitmap.reset();
        _skyTex.reset();

        _shader.reset();
        _textureManager.reset();
    }
    
    void BackgroundRenderer::drawBackground(const ViewState& viewState) {
        if (_backgroundTex) {
            // Texture
            glBindTexture(GL_TEXTURE_2D, _backgroundTex->getTexId());
    
            // Scale background coordinates
            float backgroundScale = static_cast<float>(viewState.getFar() * 2 / viewState.getCosHalfFOVXY());
            const MapPos cameraPos = viewState.getCameraPos();
            for (int i = 0; i < BACKGROUND_VERTEX_COUNT * 3; i += 3) {
                _backgroundCoords[i] = BACKGROUND_COORDS[i] * backgroundScale;
                _backgroundCoords[i + 1] = BACKGROUND_COORDS[i + 1] * backgroundScale;
                _backgroundCoords[i + 2] = -cameraPos.getZ();
            }
            const cglib::mat4x4<float>& mvpMat = viewState.getRTEModelviewProjectionMat();
            glUniformMatrix4fv(_shader->getUniformLoc("u_mvpMat"), 1, GL_FALSE, mvpMat.data());
    
            // Transform texture coordinates
            int intTwoPowZoom = (int) std::pow(2.0f, (int) viewState.getZoom());
            float scale = (float) (intTwoPowZoom * 0.5f / Const::HALF_WORLD_SIZE);
            double translateX = cameraPos.getX() * scale;
            double translateY = cameraPos.getY() * scale;
            translateX -= std::floor(translateX);
            translateY -= std::floor(translateY);
            for (int i = 0; i < BACKGROUND_VERTEX_COUNT * 2; i += 2) {
                _backgroundTexCoords[i] = (BACKGROUND_TEX_COORDS[i] - 0.5f) * scale * backgroundScale + translateX;
                _backgroundTexCoords[i + 1] = (BACKGROUND_TEX_COORDS[i + 1] - 0.5f) * scale * backgroundScale + translateY;
            }
    
            // Draw
            glVertexAttribPointer(_a_coord, 3, GL_FLOAT, GL_FALSE, 0, _backgroundCoords);
            glVertexAttribPointer(_a_texCoord, 2, GL_FLOAT, GL_FALSE, 0, _backgroundTexCoords);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, BACKGROUND_VERTEX_COUNT);
        }
    }
    
    void BackgroundRenderer::drawSky(const ViewState& viewState) {
        if (_skyTex) {
            // Texture
            glBindTexture(GL_TEXTURE_2D, _skyTex->getTexId());
    
            // Transform
            float skyScale = viewState.getFar() * SKY_SCALE_MULTIPLIER;
            // Scale sky coordinates
            for (int i = 0; i < SKY_VERTEX_COUNT * 3; ++i) {
                _skyCoords[i] = SKY_COORDS[i] * skyScale;
            }
            const cglib::mat4x4<float>& mvpMat = viewState.getRTEModelviewProjectionMat();
            glUniformMatrix4fv(_shader->getUniformLoc("u_mvpMat"), 1, GL_FALSE, mvpMat.data());
    
            // Draw
            glVertexAttribPointer(_a_coord, 3, GL_FLOAT, GL_FALSE, 0, _skyCoords);
            glVertexAttribPointer(_a_texCoord, 2, GL_FLOAT, GL_FALSE, 0, SKY_TEX_COORDS);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, SKY_VERTEX_COUNT);
        }
    }
    
    const float BackgroundRenderer::SKY_COORDS[] = {
          // North
          -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
          // East
          0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f,
          // South
          0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f,
          // West
          -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f };
    
    const float BackgroundRenderer::SKY_TEX_COORDS[] = {
          // North
          0.0f, 0.0f, 0.25f, 0.0f, 0.0f, 1.0f, 0.25f, 1.0f,
          // East
          0.25f, 0.0f, 0.5f, 0.0f, 0.25f, 1.0f, 0.5f, 1.0f,
          // South
          0.5f, 0.0f, 0.75f, 0.0f, 0.5f, 1.0f, 0.75f, 1.0f,
          // West
          0.75f, 0.0f, 1.0f, 0.0f, 0.75f, 1.0f, 1.0f, 1.0f };
    
    const float BackgroundRenderer::SKY_SCALE_MULTIPLIER = 2.0f / std::sqrt(3.0f);
    
    const float BackgroundRenderer::BACKGROUND_COORDS[] = {
          -0.5f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f};
    
    const float BackgroundRenderer::BACKGROUND_TEX_COORDS[] = {
          0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f};
    
}
