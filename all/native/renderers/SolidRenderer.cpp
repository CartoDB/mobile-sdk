#include "SolidRenderer.h"
#include "components/Options.h"
#include "graphics/Bitmap.h"
#include "graphics/ViewState.h"
#include "renderers/MapRenderer.h"
#include "renderers/utils/GLResourceManager.h"
#include "renderers/utils/Shader.h"
#include "renderers/utils/Texture.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <cmath>

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
        _a_normal(0),
        _a_texCoord(0),
        _u_mvpMat(0),
        _u_tex(0),
        _u_lightDir(0),
        _u_color(0),
        _mutex()
    {
    }
    
    SolidRenderer::~SolidRenderer() {
    }
    
    void SolidRenderer::setComponents(const std::weak_ptr<Options>& options, const std::weak_ptr<MapRenderer>& mapRenderer) {
        std::lock_guard<std::mutex> lock(_mutex);

        _options = options;
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

        // Lighting
        cglib::vec3<float> lightDir = viewState.getFocusPosNormal();
        glUniform3fv(_u_lightDir, 1, lightDir.data());
        float alpha = _color.getA() / 255.0f;
        glUniform4f(_u_color, _color.getR() * alpha / 255.0f, _color.getG() * alpha / 255.0f, _color.getB() * alpha / 255.0f, alpha);

        // Coords, texCoords, colors
        glEnableVertexAttribArray(_a_coord);
        glEnableVertexAttribArray(_a_normal);
        glEnableVertexAttribArray(_a_texCoord);

        const cglib::mat4x4<float>& mvpMat = viewState.getRTESkyProjectionMat();
        glUniformMatrix4fv(_u_mvpMat, 1, GL_FALSE, mvpMat.data());

        // Draw
        drawBackground(viewState);

        // Disable bound arrays
        glDisableVertexAttribArray(_a_coord);
        glDisableVertexAttribArray(_a_normal);
        glDisableVertexAttribArray(_a_texCoord);
    
        GLContext::CheckGLError("SolidRenderer::onDrawFrame");
    }
    
    bool SolidRenderer::initializeRenderer() {
        if (_shader && _shader->isValid() && _bitmapTex && _bitmapTex->isValid()) {
            return true;
        }

        if (auto mapRenderer = _mapRenderer.lock()) {
            // Shader and textures must be reloaded
            _shader = mapRenderer->getGLResourceManager()->create<Shader>("solid", SOLID_VERTEX_SHADER, SOLID_FRAGMENT_SHADER);
        
            // Get shader variables locations
            _u_mvpMat = _shader->getUniformLoc("u_mvpMat");
            _u_lightDir = _shader->getUniformLoc("u_lightDir");
            _u_tex = _shader->getUniformLoc("u_tex");
            _u_color = _shader->getUniformLoc("u_color");
            _a_coord = _shader->getAttribLoc("a_coord");
            _a_normal = _shader->getAttribLoc("a_normal");
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
    
    void SolidRenderer::drawBackground(const ViewState& viewState) {
        std::shared_ptr<Options> options = _options.lock();
        if (!options) {
            return;
        }

        double twoPowZoom = std::pow(2.0, viewState.getZoom());
        const cglib::vec3<double>& cameraPos = viewState.getCameraPos();

        // Spherical mode?
        if (options->getRenderProjectionMode() == RenderProjectionMode::RENDER_PROJECTION_MODE_SPHERICAL) {
            // Build sphere surface on first call
            _backgroundCoords.clear();
            _backgroundNormals.clear();
            _backgroundTexCoords.clear();
            _backgroundIndices.clear();
            BuildSphereSurface(_backgroundCoords, _backgroundNormals, _backgroundTexCoords, _backgroundIndices, cameraPos, SPHERE_TESSELATION_LEVELS_U, SPHERE_TESSELATION_LEVELS_V);

            // Calculate coordinate transformation parameters
            float coordScale = static_cast<float>(Const::WORLD_SIZE / Const::PI);
            float backgroundScale = static_cast<float>(Const::WORLD_SIZE / viewState.getCosHalfFOVXY());
            float scale = static_cast<float>(twoPowZoom / Const::WORLD_SIZE);

            // Build vertex array
            std::size_t vertexCount = _backgroundCoords.size();
            if (vertexCount != _backgroundVertices.size() * 8) {
                _backgroundVertices.resize(vertexCount * 8);
            }
            for (std::size_t i = 0; i < vertexCount; i++) {
                _backgroundVertices[i * 8 + 0] = static_cast<float>(_backgroundCoords[i](0) * coordScale - cameraPos(0));
                _backgroundVertices[i * 8 + 1] = static_cast<float>(_backgroundCoords[i](1) * coordScale - cameraPos(1));
                _backgroundVertices[i * 8 + 2] = static_cast<float>(_backgroundCoords[i](2) * coordScale - cameraPos(2));

                _backgroundVertices[i * 8 + 3] = _backgroundNormals[i](0);
                _backgroundVertices[i * 8 + 4] = _backgroundNormals[i](1);
                _backgroundVertices[i * 8 + 5] = _backgroundNormals[i](2);

                _backgroundVertices[i * 8 + 6] = static_cast<float>(_backgroundTexCoords[i](0) * scale * backgroundScale);
                _backgroundVertices[i * 8 + 7] = static_cast<float>(_backgroundTexCoords[i](1) * scale * backgroundScale);
            }

            // Lighting
            cglib::vec3<float> lightDir = viewState.getFocusPosNormal();
            glUniform3fv(_u_lightDir, 1, lightDir.data());
            
            // Draw
            glVertexAttribPointer(_a_coord, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), _backgroundVertices.data() + 0);
            glVertexAttribPointer(_a_normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), _backgroundVertices.data() + 3);
            glVertexAttribPointer(_a_texCoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), _backgroundVertices.data() + 6);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_backgroundIndices.size()), GL_UNSIGNED_SHORT, _backgroundIndices.data());
        } else if (options->getRenderProjectionMode() == RenderProjectionMode::RENDER_PROJECTION_MODE_PLANAR) {
            // Calculate coordinate transformation parameters
            float backgroundScale = static_cast<float>(viewState.getFar() * 2 / viewState.getCosHalfFOVXY());
            float scale = static_cast<float>(twoPowZoom / Const::WORLD_SIZE);

            // Build vertex array
            std::size_t vertexCount = sizeof(PLANE_COORDS) / sizeof(float) / 3;
            if (_backgroundVertices.size() != vertexCount * 8) {
                _backgroundVertices.resize(vertexCount * 8);
            }
            for (std::size_t i = 0; i < vertexCount; i++) {
                _backgroundVertices[i * 8 + 0] = PLANE_COORDS[i * 3 + 0] * backgroundScale;
                _backgroundVertices[i * 8 + 1] = PLANE_COORDS[i * 3 + 1] * backgroundScale;
                _backgroundVertices[i * 8 + 2] = static_cast<float>(-cameraPos(2));

                _backgroundVertices[i * 8 + 3] = 0.0f;
                _backgroundVertices[i * 8 + 4] = 0.0f;
                _backgroundVertices[i * 8 + 5] = 1.0f;

                _backgroundVertices[i * 8 + 6] = static_cast<float>((PLANE_TEX_COORDS[i * 2 + 0] - 0.5f) * scale * backgroundScale);
                _backgroundVertices[i * 8 + 7] = static_cast<float>((PLANE_TEX_COORDS[i * 2 + 1] - 0.5f) * scale * backgroundScale);
            }

            // Draw
            glVertexAttribPointer(_a_coord, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), _backgroundVertices.data() + 0);
            glVertexAttribPointer(_a_normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), _backgroundVertices.data() + 3);
            glVertexAttribPointer(_a_texCoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), _backgroundVertices.data() + 6);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, static_cast<GLsizei>(vertexCount));
        }
    }
    
    void SolidRenderer::BuildSphereSurface(std::vector<cglib::vec3<double> >& coords, std::vector<cglib::vec3<float> >& normals, std::vector<cglib::vec2<float> >& texCoords, std::vector<unsigned short>& indices, const cglib::vec3<double>& cameraPos, int tesselateU, int tesselateV) {
        int vertexCount = (tesselateU + 1) * (tesselateV + 1);
        int indexCount = 6 * tesselateU * tesselateV;
        coords.reserve(vertexCount);
        normals.reserve(vertexCount);
        texCoords.reserve(vertexCount);
        indices.reserve(indexCount);

        cglib::vec3<double> origin(1.0, 0.0, 0.0);
        cglib::vec3<double> axis = cglib::vector_product(origin, cglib::unit(cameraPos));
        double dot = cglib::dot_product(origin, cglib::unit(cameraPos));
        cglib::mat3x3<double> transform = cglib::rotate3_matrix(axis, std::acos(std::max(-1.0, std::min(1.0, dot))));

        // Note: we use simple longitude/latitude tesselation scheme. Recursive tetrahedra-based surface would contain fewer vertexCount but produces texture artifacts near poles
        for (int j = 0; j <= tesselateV; j++) {
            double v = Const::PI * (static_cast<double>(j) / tesselateV - 0.5);
            float t = 0.5f * std::log((1.0f + std::sin(v)) / (1.0f - std::sin(v))) / static_cast<float>(Const::PI);
            for (int i = 0; i <= tesselateU; i++) {
                double u = 2.0 * Const::PI * (static_cast<double>(i < tesselateU ? i : 0) / tesselateU - 0.5);
                float s = 2.0f * i / tesselateU;

                double x = std::cos(u) * std::cos(v);
                double y = std::sin(u) * std::cos(v);
                double z = std::sin(v);
                cglib::vec3<double> pos = cglib::transform(cglib::vec3<double>(x, y, z), transform);

                coords.push_back(pos);
                normals.emplace_back(static_cast<float>(pos(0)), static_cast<float>(pos(1)), static_cast<float>(pos(2)));
                texCoords.emplace_back(s - 1.0f, t);
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

    const unsigned char SolidRenderer::DEFAULT_BITMAP[] = {
        255, 255, 255, 255
    };

    const float SolidRenderer::PLANE_COORDS[] = {
        -0.5f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f
    };

    const float SolidRenderer::PLANE_TEX_COORDS[] = {
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f
    };

    const std::string SolidRenderer::SOLID_VERTEX_SHADER = R"GLSL(
        #version 100
        attribute vec3 a_coord;
        attribute vec3 a_normal;
        attribute vec2 a_texCoord;
        varying vec4 v_color;
        varying vec2 v_texCoord;
        uniform vec4 u_color;
        uniform vec3 u_lightDir;
        uniform mat4 u_mvpMat;
        void main() {
            float lighting = max(0.0, dot(a_normal, u_lightDir)) * 0.5 + 0.5;
            v_color = u_color * vec4(lighting, lighting, lighting, 1.0);
            v_texCoord = a_texCoord;
            gl_Position = u_mvpMat * vec4(a_coord, 1.0);
        }
    )GLSL";

    const std::string SolidRenderer::SOLID_FRAGMENT_SHADER = R"GLSL(
        #version 100
        precision mediump float;
        varying lowp vec4 v_color;
        varying mediump vec2 v_texCoord;
        uniform sampler2D u_tex;
        void main() {
            gl_FragColor = texture2D(u_tex, v_texCoord) * v_color;
        }
    )GLSL";

}
