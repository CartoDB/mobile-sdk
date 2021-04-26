#include "WatermarkRenderer.h"
#include "components/Options.h"
#include "components/LicenseManager.h"
#include "graphics/Bitmap.h"
#include "graphics/ViewState.h"
#include "renderers/utils/GLResourceManager.h"
#include "renderers/utils/Shader.h"
#include "renderers/utils/Texture.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "utils/GeneralUtils.h"

#include <cglib/mat.h>

namespace carto {

    WatermarkRenderer::WatermarkRenderer(const Options& options) :
        _watermarkBitmap(),
        _watermarkTex(),
        _watermarkCoords(),
        _watermarkTexCoords(),
        _shader(),
        _u_tex(0),
        _u_mvpMat(0),
        _a_coord(0),
        _a_texCoord(0),
        _glResourceManager(),
        _options(options)
    {
        std::fill_n(_watermarkCoords, sizeof(_watermarkCoords) / sizeof(float), 0.0f);
        for (int i = 0; i * 2 < sizeof(_watermarkTexCoords) / sizeof(float); i++) {
            _watermarkTexCoords[i * 2 + 0] = static_cast<float>(i / 2);
            _watermarkTexCoords[i * 2 + 1] = static_cast<float>(1 - i % 2);
        }
    }
    
    WatermarkRenderer::~WatermarkRenderer() {
    }
    
    void WatermarkRenderer::onSurfaceCreated(const std::shared_ptr<GLResourceManager>& resourceManager) {
        _glResourceManager = resourceManager;

        _shader = _glResourceManager->create<Shader>("watermark", WATERMARK_VERTEX_SHADER, WATERMARK_FRAGMENT_SHADER);

        // Get shader variables locations
        _u_tex = _shader->getUniformLoc("u_tex");
        _u_mvpMat = _shader->getUniformLoc("u_mvpMat");
        _a_coord = _shader->getAttribLoc("a_coord");
        _a_texCoord = _shader->getAttribLoc("a_texCoord");
    
        _watermarkBitmap.reset();
        _watermarkTex.reset();
    }
        
    void WatermarkRenderer::onSurfaceChanged(int width, int height) {
        cglib::mat4x4<float> modelviewMat = cglib::lookat4_matrix(cglib::vec3<float>(0, 0, 1), cglib::vec3<float>(0, 0, 0), cglib::vec3<float>(0, 1, 0));
        
        float aspectRatio = static_cast<float>(width) / height;
        cglib::mat4x4<float> projectionMat = cglib::ortho4_matrix(-aspectRatio, aspectRatio, -1.0f, 1.0f, -2.0f, 2.0f);
        
        _modelviewProjectionMat = projectionMat * modelviewMat;
    }
    
    void WatermarkRenderer::onDrawFrame(const ViewState& viewState) {
        std::shared_ptr<Bitmap> watermarkBitmap;
        std::string watermark;
        if (LicenseManager::GetInstance().getParameter("watermark", watermark, false)) {
            if (watermark == "custom") {
                watermarkBitmap = _options.getWatermarkBitmap();
            } else if (watermark == "carto" || watermark == "cartodb" || watermark == "nutiteq" || watermark == "development") {
                watermarkBitmap = Options::GetCartoWatermarkBitmap();
            } else if (watermark == "evaluation") {
                watermarkBitmap = Options::GetEvaluationWatermarkBitmap();
            } else if (watermark == "expired") {
                watermarkBitmap = Options::GetExpiredWatermarkBitmap();
            } else {
                Log::Error("WatermarkRenderer::onDrawFrame: Unsupported watermark type!");
            }
        }
    
        if (_watermarkBitmap != watermarkBitmap) {
            _watermarkTex.reset();
            _watermarkBitmap = watermarkBitmap;
        }

        if (_watermarkBitmap) {
            if (!_watermarkTex || !_watermarkTex->isValid()) {
                _watermarkTex = _glResourceManager->create<Texture>(_watermarkBitmap, true, false);
            }

            // If the license is limited, draw the watermark in a random corner with a fixed padding
            float watermarkAlignmentX = _options.getWatermarkAlignmentX();
            float watermarkAlignmentY = _options.getWatermarkAlignmentY();
            float watermarkPaddingX = _options.getWatermarkPadding().getX();
            float watermarkPaddingY = _options.getWatermarkPadding().getY();
            float watermarkScale = _options.getWatermarkScale();
            
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

            // Do actual rendering
            glDisable(GL_DEPTH_TEST);

            drawWatermark(viewState);

            glEnable(GL_DEPTH_TEST);
        }
    
        GLContext::CheckGLError("WatermarkRenderer::onDrawFrame");
    }
    
    void WatermarkRenderer::onSurfaceDestroyed() {
        _watermarkBitmap.reset();
        _watermarkTex.reset();

        _shader.reset();

        _glResourceManager.reset();
    }
    
    void WatermarkRenderer::drawWatermark(const ViewState& viewState) {
        // Prepare for drawing
        glUseProgram(_shader->getProgId());
        // Texture
        glUniform1i(_u_tex, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _watermarkTex->getTexId());
        // Matrix
        glUniformMatrix4fv(_u_mvpMat, 1, GL_FALSE, _modelviewProjectionMat.data());
        // Coords, texCoords, colors
        glEnableVertexAttribArray(_a_coord);
        glEnableVertexAttribArray(_a_texCoord);
        // Draw
        glVertexAttribPointer(_a_coord, 3, GL_FLOAT, GL_FALSE, 0, _watermarkCoords);
        glVertexAttribPointer(_a_texCoord, 2, GL_FLOAT, GL_FALSE, 0, _watermarkTexCoords);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(_watermarkCoords) / sizeof(float) / 3);

        // Disable bound arrays
        glDisableVertexAttribArray(_a_coord);
        glDisableVertexAttribArray(_a_texCoord);
    }
        
    const int WatermarkRenderer::WATERMARK_WIDTH_DP = 100;

    const std::string WatermarkRenderer::WATERMARK_VERTEX_SHADER = R"GLSL(
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

    const std::string WatermarkRenderer::WATERMARK_FRAGMENT_SHADER = R"GLSL(
        #version 100
        precision mediump float;
        varying mediump vec2 v_texCoord;
        uniform sampler2D u_tex;
        void main() {
            gl_FragColor = texture2D(u_tex, v_texCoord);
        }
    )GLSL";

}
