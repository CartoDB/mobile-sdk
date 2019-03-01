#include "BackgroundRenderer.h"
#include "components/Layers.h"
#include "components/Options.h"
#include "graphics/Bitmap.h"
#include "graphics/Shader.h"
#include "graphics/ShaderManager.h"
#include "graphics/TextureManager.h"
#include "graphics/Texture.h"
#include "graphics/ViewState.h"
#include "graphics/shaders/BackgroundShaderSource.h"
#include "graphics/utils/GLContext.h"
#include "layers/Layer.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <cglib/mat.h>

namespace carto {

    BackgroundRenderer::BackgroundRenderer(const Options& options, const Layers& layers) :
        _backgroundBitmap(),
        _backgroundTex(),
        _backgroundCoords(),
        _backgroundTexCoords(),
        _skyBitmap(),
        _skyTex(),
        _skyCoords(),
        _surfaceVertices(),
        _surfaceNormals(),
        _surfaceTexCoords(),
        _surfaceIndices(),
        _skyVertices(),
        _skyTexCoords(),
        _skyIndices(),
        _shader(),
        _a_coord(0),
        _a_normal(0),
        _a_texCoord(0),
        _u_tex(0),
        _u_lightDir(0),
        _u_mvpMat(0),
        _textureManager(),
        _options(options),
        _layers(layers)
    {
    }
    
    BackgroundRenderer::~BackgroundRenderer() {
    }
    
    void BackgroundRenderer::onSurfaceCreated(const std::shared_ptr<ShaderManager>& shaderManager, const std::shared_ptr<TextureManager>& textureManager) {
        // Shader and textures must be reloaded
        _shader = shaderManager->createShader(background_shader_source);
    
        // Get shader variables locations
        glUseProgram(_shader->getProgId());
        _u_tex = _shader->getUniformLoc("u_tex");
        _u_lightDir = _shader->getUniformLoc("u_lightDir");
        _u_mvpMat = _shader->getUniformLoc("u_mvpMat");
        _a_coord = _shader->getAttribLoc("a_coord");
        _a_normal = _shader->getAttribLoc("a_normal");
        _a_texCoord = _shader->getAttribLoc("a_texCoord");
    
        _textureManager = textureManager;
        _backgroundBitmap.reset();
        _backgroundTex.reset();
        _skyBitmap.reset();
        _skyTex.reset();
    }
    
    void BackgroundRenderer::onDrawFrame(const ViewState& viewState) {
        std::vector<std::shared_ptr<Layer> > layers = _layers.getAll();

        std::shared_ptr<Bitmap> backgroundBitmap;
        if (!layers.empty() && _options.getBackgroundBitmap() == Options::GetDefaultBackgroundBitmap()) {
            backgroundBitmap = layers.front()->getBackgroundBitmap();
        }
        if (!backgroundBitmap) {
            backgroundBitmap = _options.getBackgroundBitmap();
        }
        if (_backgroundBitmap != backgroundBitmap) {
            if (backgroundBitmap) {
                _backgroundTex = _textureManager->createTexture(backgroundBitmap, true, true);
            } else {
                _backgroundTex.reset();
            }
            _backgroundBitmap = backgroundBitmap;
        }

        std::shared_ptr<Bitmap> skyBitmap;
        if (!layers.empty()) {
            skyBitmap = layers.front()->getSkyBitmap();
        }
        if (!skyBitmap) {
            skyBitmap = _options.getSkyBitmap();
        }
        if (_skyBitmap != skyBitmap) {
            if (skyBitmap) {
                _skyTex = _textureManager->createTexture(skyBitmap, true, true);
            } else {
                _skyTex.reset();
            }
           _skyBitmap = skyBitmap;
        }
    
        if (_skyTex || _backgroundTex) {
            // Prepare for drawing
            glUseProgram(_shader->getProgId());
            // Texture
            glUniform1i(_u_tex, 0);
            glActiveTexture(GL_TEXTURE0);
            // Matrix
            const cglib::mat4x4<float>& mvpMat = viewState.getRTEModelviewProjectionMat();
            glUniformMatrix4fv(_u_mvpMat, 1, GL_FALSE, mvpMat.data());
            // Default lighting
            glUniform3f(_u_lightDir, 0, 0, 1);
            // Coords, texCoords, colors
            glEnableVertexAttribArray(_a_coord);
            glEnableVertexAttribArray(_a_texCoord);
            glVertexAttrib3f(_a_normal, 0, 0, 1);
    
            if (viewState.isSkyVisible()) {
                drawSky(viewState);
            }
            drawBackground(viewState);
    
            // Disable bound arrays
            glDisableVertexAttribArray(_a_coord);
            glDisableVertexAttribArray(_a_texCoord);
        }
    
        GLContext::CheckGLError("BackgroundRenderer::onDrawFrame");
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
        if (!_backgroundTex) {
            return;
        }

        // Texture
        glBindTexture(GL_TEXTURE_2D, _backgroundTex->getTexId());

        // Spherical mode?
        if (_options.getRenderProjectionMode() == RenderProjectionMode::RENDER_PROJECTION_MODE_SPHERICAL) {
            // Build sphere surface on first call
            if (_surfaceIndices.empty()) {
                BuildSphereSurface(_surfaceVertices, _surfaceNormals, _surfaceTexCoords, _surfaceIndices, SURFACE_TESSELATION_LEVELS, SURFACE_TESSELATION_LEVELS);
            }

            // Transform coordinates
            float coordScale = Const::WORLD_SIZE / Const::PI;
            const cglib::vec3<double>& focusPos = viewState.getFocusPos();
            const cglib::vec3<double>& cameraPos = viewState.getCameraPos();

            if (_surfaceVertices.size() != _backgroundCoords.size() * 3) {
                _backgroundCoords.resize(_surfaceVertices.size() * 3);
            }
            for (std::size_t i = 0; i < _surfaceVertices.size(); i++) {
                _backgroundCoords[i * 3 + 0] = static_cast<float>(_surfaceVertices[i](0) * coordScale - cameraPos(0));
                _backgroundCoords[i * 3 + 1] = static_cast<float>(_surfaceVertices[i](1) * coordScale - cameraPos(1));
                _backgroundCoords[i * 3 + 2] = static_cast<float>(_surfaceVertices[i](2) * coordScale - cameraPos(2));
            }

            // Transform texture coordinates
            float backgroundScale = static_cast<float>(Const::WORLD_SIZE / viewState.getCosHalfFOVXY());
            int intTwoPowZoom = (int) std::pow(2.0f, (int) viewState.getZoom());
            float scale = (float) (intTwoPowZoom * 0.5f / Const::HALF_WORLD_SIZE);
            double translateOriginX = (focusPos(0) != 0 || focusPos(1) != 0 ? std::atan2(focusPos(1), focusPos(0)) / Const::PI + 1.0 : 0);
            double translateOriginY = std::asin(std::max(-1.0, std::min(1.0, focusPos(2) / cglib::length(focusPos)))) / Const::PI + 0.5;
            double translateX = translateOriginX * scale - 0.5 * scale * backgroundScale;
            double translateY = translateOriginY * scale + 0.5 * scale * backgroundScale;
            translateX -= std::floor(translateX);
            translateY -= std::floor(translateY);

            if (_surfaceTexCoords.size() != _backgroundTexCoords.size() * 2) {
                _backgroundTexCoords.resize(_surfaceTexCoords.size() * 2);
            }
            for (std::size_t i = 0; i < _surfaceTexCoords.size(); i++) {
                _backgroundTexCoords[i * 2 + 0] = static_cast<float>(_surfaceTexCoords[i](0) * scale * backgroundScale + translateX);
                _backgroundTexCoords[i * 2 + 1] = static_cast<float>(_surfaceTexCoords[i](1) * scale * backgroundScale - translateY);
            }

            // Lighting
            cglib::vec3<float> lightDir = viewState.getFocusPosNormal();
            glUniform3fv(_u_lightDir, 1, lightDir.data());
            
            // Draw
            glEnableVertexAttribArray(_a_normal);
            glVertexAttribPointer(_a_coord, 3, GL_FLOAT, GL_FALSE, 0, _backgroundCoords.data());
            glVertexAttribPointer(_a_normal, 3, GL_FLOAT, GL_FALSE, 0, _surfaceNormals.data());
            glVertexAttribPointer(_a_texCoord, 2, GL_FLOAT, GL_FALSE, 0, _backgroundTexCoords.data());
            glDrawElements(GL_TRIANGLES, _surfaceIndices.size(), GL_UNSIGNED_SHORT, _surfaceIndices.data());
            glDisableVertexAttribArray(_a_normal);
            return;
        }

        float backgroundScale = static_cast<float>(viewState.getFar() * 2 / viewState.getCosHalfFOVXY());
        const cglib::vec3<double>& cameraPos = viewState.getCameraPos();

        // Scale background coordinates
        std::size_t vertexCount = sizeof(BACKGROUND_COORDS) / sizeof(float) / 3;
        if (_backgroundCoords.size() != vertexCount * 3) {
            _backgroundCoords.resize(vertexCount * 3);
        }
        for (std::size_t i = 0; i < vertexCount * 3; i += 3) {
            _backgroundCoords[i + 0] = BACKGROUND_COORDS[i + 0] * backgroundScale;
            _backgroundCoords[i + 1] = BACKGROUND_COORDS[i + 1] * backgroundScale;
            _backgroundCoords[i + 2] = static_cast<float>(-cameraPos(2));
        }

        // Transform texture coordinates
        int intTwoPowZoom = (int) std::pow(2.0f, (int) viewState.getZoom());
        float scale = (float) (intTwoPowZoom * 0.5f / Const::HALF_WORLD_SIZE);
        double translateX = cameraPos(0) * scale;
        double translateY = cameraPos(1) * scale;
        translateX -= std::floor(translateX);
        translateY -= std::floor(translateY);
        if (_backgroundTexCoords.size() != vertexCount * 2) {
            _backgroundTexCoords.resize(vertexCount * 2);
        }
        for (std::size_t i = 0; i < vertexCount * 2; i += 2) {
            _backgroundTexCoords[i + 0] = static_cast<float>((BACKGROUND_TEX_COORDS[i + 0] - 0.5f) * scale * backgroundScale + translateX);
            _backgroundTexCoords[i + 1] = static_cast<float>((BACKGROUND_TEX_COORDS[i + 1] - 0.5f) * scale * backgroundScale + translateY);
        }

        // Draw
        glVertexAttribPointer(_a_coord, 3, GL_FLOAT, GL_FALSE, 0, _backgroundCoords.data());
        glVertexAttribPointer(_a_texCoord, 2, GL_FLOAT, GL_FALSE, 0, _backgroundTexCoords.data());
        glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount);
    }
    
    void BackgroundRenderer::drawSky(const ViewState& viewState) {
        if (!_skyTex) {
            return;
        }

        // Texture
        glBindTexture(GL_TEXTURE_2D, _skyTex->getTexId());

        // Spherical mode?
        if (_options.getRenderProjectionMode() != RenderProjectionMode::RENDER_PROJECTION_MODE_PLANAR) {
            _skyVertices.clear();
            _skyTexCoords.clear();
            _skyIndices.clear();
            BuildSphereSky(_skyVertices, _skyTexCoords, _skyIndices, viewState.getCameraPos() * (1.0 / Const::WORLD_SIZE * Const::PI), viewState.getUpVec(), SKY_RELATIVE_HEIGHT, SKY_TESSELATION_LEVELS);

            // Transform coordinates
            float coordScale = Const::WORLD_SIZE / Const::PI;
            const cglib::vec3<double>& focusPos = viewState.getFocusPos();
            const cglib::vec3<double>& cameraPos = viewState.getCameraPos();

            if (_skyCoords.size() != _skyVertices.size() * 3) {
                _skyCoords.resize(_skyVertices.size() * 3);
            }
            for (std::size_t i = 0; i < _skyVertices.size(); i++) {
                _skyCoords[i * 3 + 0] = static_cast<float>(_skyVertices[i](0) * coordScale - cameraPos(0));
                _skyCoords[i * 3 + 1] = static_cast<float>(_skyVertices[i](1) * coordScale - cameraPos(1));
                _skyCoords[i * 3 + 2] = static_cast<float>(_skyVertices[i](2) * coordScale - cameraPos(2));
            }

            // Draw
            glVertexAttribPointer(_a_coord, 3, GL_FLOAT, GL_FALSE, 0, _skyCoords.data());
            glVertexAttribPointer(_a_texCoord, 2, GL_FLOAT, GL_FALSE, 0, _skyTexCoords.data());
            glDrawElements(GL_TRIANGLES, _skyIndices.size(), GL_UNSIGNED_SHORT, _skyIndices.data());
            return;
        }

        // Scale sky coordinates
        float skyScale = viewState.getFar() * SKY_SCALE_MULTIPLIER;
        const cglib::mat4x4<float>& mvpMat = viewState.getRTEModelviewProjectionMat();
        glUniformMatrix4fv(_u_mvpMat, 1, GL_FALSE, mvpMat.data());

        std::size_t vertexCount = sizeof(SKY_COORDS) / sizeof(float) / 3;
        if (_skyCoords.size() != vertexCount * 3) {
            _skyCoords.resize(vertexCount * 3);
        }
        for (std::size_t i = 0; i < vertexCount; i++) {
            _skyCoords[i * 3 + 0] = SKY_COORDS[i * 3 + 0] * skyScale;
            _skyCoords[i * 3 + 1] = SKY_COORDS[i * 3 + 1] * skyScale;
            _skyCoords[i * 3 + 2] = (SKY_COORDS[i * 3 + 2] + 0.5f) * 0.5f * skyScale;
        }

        // Draw
        glVertexAttribPointer(_a_coord, 3, GL_FLOAT, GL_FALSE, 0, _skyCoords.data());
        glVertexAttribPointer(_a_texCoord, 2, GL_FLOAT, GL_FALSE, 0, SKY_TEX_COORDS);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount);
    }

    void BackgroundRenderer::BuildSphereSky(std::vector<cglib::vec3<double> >& vertices, std::vector<cglib::vec2<float> >& texCoords, std::vector<unsigned short>& indices, const cglib::vec3<double>& cameraPos, const cglib::vec3<double>& upVec, double height, int tesselate) {
        int vertexCount = (tesselate + 1) * 2;
        int indexCount = 6 * tesselate;
        vertices.reserve(vertexCount);
        texCoords.reserve(vertexCount);
        indices.reserve(indexCount);

        cglib::vec3<double> axis1 = cglib::unit(cglib::vector_product(cglib::vector_product(cameraPos, upVec), cameraPos));
        cglib::vec3<double> axis2 = cglib::unit(cglib::vector_product(cameraPos, axis1));
        cglib::vec3<double> origin = cameraPos * (1.0 / cglib::norm(cameraPos));
        double r = std::sqrt(std::max(0.0, 1.0 - cglib::norm(origin)));

        for (int i = 0; i <= tesselate; i++) {
            double u = 2.0 * Const::PI * (static_cast<double>(i < tesselate ? i : 0) / tesselate - 0.5);
            double x = std::cos(u);
            double y = std::sin(u);
            vertices.emplace_back((axis1 * x + axis2 * y) * (r * 0.95) + origin);
            vertices.emplace_back((axis1 * x + axis2 * y) * (r + height) + origin);

            float s = static_cast<float>(i) / tesselate;
            texCoords.emplace_back(s, 0.0f);
            texCoords.emplace_back(s, 1.0f);
        }

        for (int i = 0; i < tesselate; i++) {
            int j = i + 1;
            indices.push_back(static_cast<unsigned short>(i * 2 + 0));
            indices.push_back(static_cast<unsigned short>(i * 2 + 1));
            indices.push_back(static_cast<unsigned short>(j * 2 + 1));
            indices.push_back(static_cast<unsigned short>(i * 2 + 0));
            indices.push_back(static_cast<unsigned short>(j * 2 + 1));
            indices.push_back(static_cast<unsigned short>(j * 2 + 0));
        }
    }

    void BackgroundRenderer::BuildSphereSurface(std::vector<cglib::vec3<double> >& vertices, std::vector<cglib::vec3<float> >& normals, std::vector<cglib::vec2<float> >& texCoords, std::vector<unsigned short>& indices, int tesselateU, int tesselateV) {
        int vertexCount = (tesselateU + 1) * (tesselateV + 1);
        int indexCount = 6 * tesselateU * tesselateV;
        vertices.reserve(vertexCount);
        normals.reserve(vertexCount);
        texCoords.reserve(vertexCount);
        indices.reserve(indexCount);

        // Note: we use simple longitude/latitude tesselation scheme. Recursive tetrahedra-based surface would contain fewer vertexCount but produces texture artifacts near poles
        for (int j = 0; j <= tesselateV; j++) {
            float t = 1.0f - 1.0f * j / tesselateV;
            double v = Const::PI * (static_cast<double>(j) / tesselateV - 0.5);
            for (int i = 0; i <= tesselateU; i++) {
                float s = 2.0f * i / tesselateU;
                double u = 2.0 * Const::PI * (static_cast<double>(i < tesselateU ? i : 0) / tesselateU - 0.5);

                double x = std::cos(u) * std::cos(v);
                double y = std::sin(u) * std::cos(v);
                double z = std::sin(v);

                vertices.emplace_back(x, y, z);
                normals.emplace_back(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
                texCoords.emplace_back(s, t);
            }
        }

        for (int j = 0; j < tesselateV; j++) {
            for (int i = 0; i < tesselateU; i++) {
                int i00 = (i + 0) + (j + 0) * (tesselateU + 1);
                int i01 = (i + 0) + (j + 1) * (tesselateU + 1);
                int i10 = (i + 1) + (j + 0) * (tesselateU + 1);
                int i11 = (i + 1) + (j + 1) * (tesselateU + 1);

                indices.push_back(static_cast<unsigned short>(i00));
                indices.push_back(static_cast<unsigned short>(i10));
                indices.push_back(static_cast<unsigned short>(i01));
                indices.push_back(static_cast<unsigned short>(i10));
                indices.push_back(static_cast<unsigned short>(i11));
                indices.push_back(static_cast<unsigned short>(i01));
            }
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
        -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f
    };

    const float BackgroundRenderer::SKY_TEX_COORDS[] = {
        // North
        0.0f, 0.0f, 0.25f, 0.0f, 0.0f, 1.0f, 0.25f, 1.0f,
        // East
        0.25f, 0.0f, 0.5f, 0.0f, 0.25f, 1.0f, 0.5f, 1.0f,
        // South
        0.5f, 0.0f, 0.75f, 0.0f, 0.5f, 1.0f, 0.75f, 1.0f,
        // West
        0.75f, 0.0f, 1.0f, 0.0f, 0.75f, 1.0f, 1.0f, 1.0f
    };

    const float BackgroundRenderer::SKY_SCALE_MULTIPLIER = 2.0f / std::sqrt(3.0f);

    const float BackgroundRenderer::SKY_RELATIVE_HEIGHT = 0.1f;

    const float BackgroundRenderer::BACKGROUND_COORDS[] = {
        -0.5f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f
    };

    const float BackgroundRenderer::BACKGROUND_TEX_COORDS[] = {
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f
    };

}
