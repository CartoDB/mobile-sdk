#include "WatermarkRenderer.h"
#include "components/Options.h"
#include "components/LicenseManager.h"
#include "graphics/Bitmap.h"
#include "graphics/Shader.h"
#include "graphics/ShaderManager.h"
#include "graphics/TextureManager.h"
#include "graphics/Texture.h"
#include "graphics/ViewState.h"
#include "graphics/shaders/TexturedShaderSource.h"
#include "graphics/utils/GLContext.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "utils/GeneralUtils.h"
#include "assets/EvaluationWatermarkPNG.h"
#include "assets/ExpiredWatermarkPNG.h"
#include "assets/CartoWatermarkPNG.h"

#include <random>

#include <cglib/mat.h>

namespace carto {

    WatermarkRenderer::WatermarkRenderer(const Options& options) :
        _randomAlignmentX(),
        _randomAlignmentY(),
        _watermarkBitmap(),
        _watermarkTex(),
        _watermarkCoords(),
        _watermarkTexCoords(),
        _shader(),
        _u_tex(0),
        _u_mvpMat(0),
        _a_coord(0),
        _a_texCoord(0),
        _textureManager(),
        _options(options)
    {
        std::fill_n(_watermarkCoords, WATERMARK_VERTEX_COUNT * 3, 0.0f);
        for (int i = 0; i < WATERMARK_VERTEX_COUNT; i++) {
            _watermarkTexCoords[i * 2 + 0] = static_cast<float>(i / 2);
            _watermarkTexCoords[i * 2 + 1] = static_cast<float>(1 - i % 2);
        }
    }
    
    WatermarkRenderer::~WatermarkRenderer() {
    }
    
    void WatermarkRenderer::onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager) {
        // Shader and textures must be reloaded
        _shader = shaderManager->createShader(textured_shader_source);
    
        // Get shader variables locations
        glUseProgram(_shader->getProgId());
        _u_tex = _shader->getUniformLoc("u_tex");
        _u_mvpMat = _shader->getUniformLoc("u_mvpMat");
        _a_coord = _shader->getAttribLoc("a_coord");
        _a_texCoord = _shader->getAttribLoc("a_texCoord");
    
        _textureManager = textureManager;
        _watermarkBitmap.reset();
        _watermarkTex.reset();
    }
        
    void WatermarkRenderer::onSurfaceChanged(int width, int height) {
        // Choose a random corner, this is used for placing the watermark with
        // the evaluation license
        std::mt19937 rng;
        rng.seed(static_cast<int>(time(NULL)));
        for (int i = 0; i < 16; i++) {
            rng();
        }
        std::uniform_int_distribution<int> dist(0, 1);
        _randomAlignmentX = dist(rng) * 2 - 1;
        _randomAlignmentY = dist(rng) * 2 - 1;
        
        cglib::mat4x4<float> modelviewMat = cglib::lookat4_matrix(cglib::vec3<float>(0, 0, 1), cglib::vec3<float>(0, 0, 0), cglib::vec3<float>(0, 1, 0));
        
        float aspectRatio = static_cast<float>(width) / height;
        cglib::mat4x4<float> projectionMat = cglib::ortho4_matrix(-aspectRatio, aspectRatio, -1.0f, 1.0f, -2.0f, 2.0f);
        
        _modelviewProjectionMat = projectionMat * modelviewMat;
        
        _surfaceChanged = true;
    }
    
    void WatermarkRenderer::onDrawFrame(const ViewState& viewState) {
        bool limitedLicense = false;
        std::shared_ptr<Bitmap> watermarkBitmap;
        std::string watermark;
        if (LicenseManager::GetInstance().getParameter("watermark", watermark)) {
            if (watermark == "custom") {
                watermarkBitmap = _options.getWatermarkBitmap();
            } else if (watermark == "carto" || watermark == "cartodb" || watermark == "nutiteq") {
                watermarkBitmap = GetCartoWatermarkBitmap();
            } else if (watermark == "evaluation" || watermark == "development" || watermark == "expired") {
                limitedLicense = true;
                watermarkBitmap = (watermark == "expired" ? GetExpiredWatermarkBitmap() : GetEvaluationWatermarkBitmap());
            } else {
                Log::Error("WatermarkRenderer::onDrawFrame: Unsupported watermark type!");
            }
        }
    
        bool watermarkChanged = false;
        if (_watermarkBitmap != watermarkBitmap) {
            if (watermarkBitmap) {
                _watermarkTex = _textureManager->createTexture(watermarkBitmap, true, false);
            }
            else {
                _watermarkTex.reset();
            }

            watermarkChanged = true;
            _watermarkBitmap = watermarkBitmap;
        }
        
        if ((_surfaceChanged || watermarkChanged) && _watermarkBitmap) {
            _surfaceChanged = false;
            
            // If the license is limited, draw the watermark in a random corner with a fixed padding
            float watermarkAlignmentX = limitedLicense ? _randomAlignmentX : _options.getWatermarkAlignmentX();
            float watermarkAlignmentY = limitedLicense ? _randomAlignmentY : _options.getWatermarkAlignmentY();
            float watermarkPaddingX = limitedLicense ? FIXED_WATERMARK_PADDING_X : _options.getWatermarkPadding().getX();
            float watermarkPaddingY = limitedLicense ? FIXED_WATERMARK_PADDING_Y : _options.getWatermarkPadding().getY();
            float watermarkScale = limitedLicense ? 1.0f : _options.getWatermarkScale();
            
            // Calculate some params
            float bitmapAspectRatio = static_cast<float>(_watermarkBitmap->getWidth()) / _watermarkBitmap->getHeight();
            float dpToPx = viewState.getDPI() / Const::UNSCALED_DPI;
            
            // Calculate watermark's width in the [-aspect, aspect] range and height in [-1, 1] range
            float halfWidth = watermarkScale * WATERMARK_WIDTH_DP * dpToPx / viewState.getWidth() * viewState.getAspectRatio();
            float halfHeight = halfWidth / bitmapAspectRatio;
            
            // Calculate watermark's padding width in the [-aspect, aspect] range and padding height in [-1, 1] range
            float paddingCoef = dpToPx / viewState.getWidth() * viewState.getAspectRatio() * 2;
            float paddingX = watermarkPaddingX * paddingCoef;
            float paddingY = watermarkPaddingY * paddingCoef;
            
            // Calculate center coordinates of the watermark, so it's always completely visible on the screen
            float thresholdX = viewState.getAspectRatio() - halfWidth;
            float thresholdY = 1 - halfHeight;
            
            float centerX = GeneralUtils::Clamp(thresholdX - paddingX, -thresholdX, thresholdX) * watermarkAlignmentX;
            float centerY = GeneralUtils::Clamp(thresholdY - paddingY, -thresholdY, thresholdY) * watermarkAlignmentY;
            
            _watermarkCoords[0] = centerX - halfWidth;
            _watermarkCoords[1] = centerY + halfHeight;
            _watermarkCoords[3] = centerX - halfWidth;
            _watermarkCoords[4] = centerY - halfHeight;
            _watermarkCoords[6] = centerX + halfWidth;
            _watermarkCoords[7] = centerY + halfHeight;
            _watermarkCoords[9] = centerX + halfWidth;
            _watermarkCoords[10] = centerY - halfHeight;
            
            const cglib::vec2<float>& texCoordScale = _watermarkTex->getTexCoordScale();
            _watermarkTexCoords[0] = 0.0f;
            _watermarkTexCoords[1] = texCoordScale(1);
            _watermarkTexCoords[2] = 0.0f;
            _watermarkTexCoords[3] = 0.0f;
            _watermarkTexCoords[4] = texCoordScale(0);
            _watermarkTexCoords[5] = texCoordScale(1);
            _watermarkTexCoords[6] = texCoordScale(0);
            _watermarkTexCoords[7] = 0.0f;
        }
    
        if (_watermarkTex) {
            drawWatermark(viewState);
        }
    
        GLContext::CheckGLError("WatermarkRenderer::onDrawFrame");
    }
    
    void WatermarkRenderer::onSurfaceDestroyed() {
        _watermarkBitmap.reset();
        _watermarkTex.reset();

        _shader.reset();
        _textureManager.reset();
    }
    
    void WatermarkRenderer::drawWatermark(const ViewState& viewState) {
        // Prepare for drawing
        glUseProgram(_shader->getProgId());
        // Texture
        glUniform1i(_u_tex, 0);
        // Matrix
        glUniformMatrix4fv(_u_mvpMat, 1, GL_FALSE, _modelviewProjectionMat.data());
        // Coords, texCoords, colors
        glEnableVertexAttribArray(_a_coord);
        glEnableVertexAttribArray(_a_texCoord);
        // Texture
        glBindTexture(GL_TEXTURE_2D, _watermarkTex->getTexId());
        // Draw
        glVertexAttribPointer(_a_coord, 3, GL_FLOAT, GL_FALSE, 0, _watermarkCoords);
        glVertexAttribPointer(_a_texCoord, 2, GL_FLOAT, GL_FALSE, 0, _watermarkTexCoords);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, WATERMARK_VERTEX_COUNT);
        // Disable bound arrays
        glDisableVertexAttribArray(_a_coord);
        glDisableVertexAttribArray(_a_texCoord);
    }
        
    std::shared_ptr<Bitmap> WatermarkRenderer::GetEvaluationWatermarkBitmap() {
        if (!_EvaluationWatermarkBitmap) {
            _EvaluationWatermarkBitmap = Bitmap::CreateFromCompressed(evaluation_watermark_png, evaluation_watermark_png_len);
        }
        return _EvaluationWatermarkBitmap;
    }
    
    std::shared_ptr<Bitmap> WatermarkRenderer::GetExpiredWatermarkBitmap() {
        if (!_ExpiredWatermarkBitmap) {
            _ExpiredWatermarkBitmap = Bitmap::CreateFromCompressed(expired_watermark_png, expired_watermark_png_len);
        }
        return _ExpiredWatermarkBitmap;
    }
        
    std::shared_ptr<Bitmap> WatermarkRenderer::GetCartoWatermarkBitmap() {
        if (!_CartoWatermarkBitmap) {
            _CartoWatermarkBitmap = Bitmap::CreateFromCompressed(carto_watermark_png, carto_watermark_png_len);
        }
        return _CartoWatermarkBitmap;
    }
        
    std::shared_ptr<Bitmap> WatermarkRenderer::_EvaluationWatermarkBitmap;
    std::shared_ptr<Bitmap> WatermarkRenderer::_ExpiredWatermarkBitmap;
    std::shared_ptr<Bitmap> WatermarkRenderer::_CartoWatermarkBitmap;
    
}
