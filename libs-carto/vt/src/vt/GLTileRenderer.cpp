#include "GLTileRenderer.h"
#include "Color.h"
#include "BitmapManager.h"

#include <cassert>
#include <algorithm>

namespace {
    static const std::string backgroundVsh = R"GLSL(
        attribute vec2 aVertexPosition;
        uniform mat4 uMVPMatrix;
        #ifdef PATTERN
        varying vec2 vUV;
        #endif

        void main(void) {
        #ifdef PATTERN
            vUV = aVertexPosition;
        #endif
            gl_Position = uMVPMatrix * vec4(aVertexPosition, 0.0, 1.0);
        }
    )GLSL";

    static const std::string backgroundFsh = R"GLSL(
        precision mediump float;
        #ifdef PATTERN
        uniform sampler2D uPattern;
        #endif
        uniform lowp vec4 uColor;
        uniform lowp float uOpacity;
        #ifdef PATTERN
        varying vec2 vUV;
        #endif

        void main(void) {
        #ifdef PATTERN
            vec4 patternColor = texture2D(uPattern, vUV);
            gl_FragColor = (uColor * (1.0 - patternColor.a) + patternColor) * uOpacity;
        #else
            gl_FragColor = uColor * uOpacity;
        #endif
        }
    )GLSL";

    static const std::string bitmapVsh = R"GLSL(
        attribute vec2 aVertexPosition;
        uniform mat4 uMVPMatrix;
        uniform vec2 uUVScale;
        uniform vec2 uUVOffset;
        varying vec2 vUV;

        void main(void) {
            vec2 uv = uUVScale * aVertexPosition + uUVOffset;
            vUV = vec2(uv.x, 1.0 - uv.y);
            gl_Position = uMVPMatrix * vec4(aVertexPosition, 0.0, 1.0);
        }
    )GLSL";

    static const std::string bitmapFsh = R"GLSL(
        precision mediump float;
        uniform sampler2D uBitmap;
        uniform lowp float uOpacity;
        varying vec2 vUV;

        void main(void) {
            gl_FragColor = texture2D(uBitmap, vUV) * uOpacity;
        }
    )GLSL";

    static const std::string blendVsh = R"GLSL(
        attribute vec2 aVertexPosition;
        uniform mat4 uMVPMatrix;

        void main(void) {
            gl_Position = uMVPMatrix * vec4(aVertexPosition, 0.0, 1.0);
        }
    )GLSL";

    static const std::string blendFsh = R"GLSL(
        precision mediump float;
        uniform sampler2D uTexture;
        uniform lowp vec4 uColor;
        uniform mediump vec2 uInvScreenSize;

        void main(void) {
            vec4 textureColor = texture2D(uTexture, gl_FragCoord.xy * uInvScreenSize);
            gl_FragColor = textureColor * uColor;
        }
    )GLSL";

    static const std::string labelVsh = R"GLSL(
        attribute vec3 aVertexPosition;
        attribute vec2 aVertexUV;
        attribute vec4 aVertexColor;
        uniform mat4 uMVPMatrix;
        uniform vec2 uUVScale;
        varying lowp vec4 vColor;
        varying vec2 vUV;

        void main(void) {
            vColor = aVertexColor;
            vUV = uUVScale * aVertexUV;
            gl_Position = uMVPMatrix * vec4(aVertexPosition, 1.0);
        }
    )GLSL";

    static const std::string labelFsh = R"GLSL(
        precision mediump float;
        uniform sampler2D uBitmap;
        varying lowp vec4 vColor;
        varying vec2 vUV;

        void main(void) {
            gl_FragColor = texture2D(uBitmap, vUV) * vColor;
        }
    )GLSL";

    static const std::string pointVsh = R"GLSL(
        attribute vec2 aVertexPosition;
        attribute vec2 aVertexBinormal;
        #ifdef PATTERN
        attribute vec2 aVertexUV;
        #endif
        attribute vec4 aVertexAttribs;
        #ifdef PATTERN
        uniform float uUVScale;
        #endif
        uniform float uBinormalScale;
        uniform vec3 uXAxis;
        uniform vec3 uYAxis;
        #ifdef TRANSFORM
        uniform mat3 uTransformMatrix;
        #endif
        uniform mat4 uMVPMatrix;
        uniform vec4 uColorTable[16];
        varying lowp vec4 vColor;
        #ifdef PATTERN
        varying vec2 vUV;
        #endif

        void main(void) {
            int styleIndex = int(aVertexAttribs[0]);
            vec2 xy = aVertexBinormal * uBinormalScale;
        #ifdef TRANSFORM
            xy = vec2(uTransformMatrix * vec3(xy, 1.0));
        #endif
            vec3 pos = vec3(aVertexPosition, 0.0) + xy[0] * uXAxis + xy[1] * uYAxis;
            vColor = uColorTable[styleIndex];
        #ifdef PATTERN
            vUV = uUVScale * aVertexUV;
        #endif
            gl_Position = uMVPMatrix * vec4(pos, 1.0);
        }
    )GLSL";

    static const std::string pointFsh = R"GLSL(
        precision mediump float;
        #ifdef PATTERN
        uniform sampler2D uPattern;
        #endif
        varying lowp vec4 vColor;
        #ifdef PATTERN
        varying vec2 vUV;
        #endif
        varying float vWidth;

        void main(void) {
        #ifdef PATTERN
            gl_FragColor = texture2D(uPattern, vUV) * vColor;
        #else
            gl_FragColor = vColor;
        #endif
        }
    )GLSL";

    static const std::string lineVsh = R"GLSL(
        attribute vec2 aVertexPosition;
        attribute vec2 aVertexBinormal;
        #ifdef PATTERN
        attribute vec2 aVertexUV;
        #endif
        attribute vec4 aVertexAttribs;
        #ifdef PATTERN
        uniform float uUVScale;
        uniform float uZoomScale;
        #endif
        uniform float uBinormalScale;
        uniform float uHalfResolution;
        uniform float uGamma;
        #ifdef TRANSFORM
        uniform mat3 uTransformMatrix;
        #endif
        uniform mat4 uMVPMatrix;
        uniform vec4 uColorTable[16];
        uniform float uWidthTable[16];
        varying lowp vec4 vColor;
        #ifdef PATTERN
        varying vec2 vUV;
        #endif
        varying vec2 vDist;
        varying float vWidth;

        void main(void) {
            int styleIndex = int(aVertexAttribs[0]);
            float width = uWidthTable[styleIndex] * uHalfResolution;
            float roundedWidth = width + 1.0;
        #ifdef TRANSFORM
            vec3 pos = vec3(vec2(uTransformMatrix * vec3(aVertexPosition, 1.0)) + uBinormalScale * roundedWidth * aVertexBinormal, 0.0);
        #else
            vec3 pos = vec3(aVertexPosition + uBinormalScale * roundedWidth * aVertexBinormal, 0.0);
        #endif
            vColor = uColorTable[styleIndex];
        #ifdef PATTERN
            vUV = vec2(uZoomScale * uUVScale * aVertexUV.x, uUVScale * aVertexUV.y);
        #endif
            vDist = vec2(aVertexAttribs[1], aVertexAttribs[2]) * (roundedWidth * uGamma);
            vWidth = (width - 1.0) * uGamma;
            gl_Position = uMVPMatrix * vec4(pos, 1.0);
        }
    )GLSL";

    static const std::string lineFsh = R"GLSL(
        precision mediump float;
        #ifdef PATTERN
        uniform sampler2D uPattern;
        #endif
        varying lowp vec4 vColor;
        #ifdef PATTERN
        varying vec2 vUV;
        #endif
        varying vec2 vDist;
        varying float vWidth;

        void main(void) {
            float dist = length(vDist) - vWidth;
            lowp float a = clamp(1.0 - dist, 0.0, 1.0);
        #ifdef PATTERN
            gl_FragColor = texture2D(uPattern, vUV) * vColor * a;
        #else
            gl_FragColor = vColor * a;
        #endif
        }
    )GLSL";

    static const std::string polygonVsh = R"GLSL(
        attribute vec2 aVertexPosition;
        #ifdef PATTERN
        attribute vec2 aVertexUV;
        #endif
        attribute vec4 aVertexAttribs;
        #ifdef PATTERN
        uniform float uUVScale;
        uniform float uZoomScale;
        #endif
        #ifdef TRANSFORM
        uniform mat3 uTransformMatrix;
        #endif
        uniform mat4 uMVPMatrix;
        uniform vec4 uColorTable[16];
        varying lowp vec4 vColor;
        #ifdef PATTERN
        varying vec2 vUV;
        #endif

        void main(void) {
            int styleIndex = int(aVertexAttribs[0]);
        #ifdef TRANSFORM
            vec3 pos = vec3(vec2(uTransformMatrix * vec3(aVertexPosition, 1.0)), 0.0);
        #else
            vec3 pos = vec3(aVertexPosition, 0.0);
        #endif
            vColor = uColorTable[styleIndex];
        #ifdef PATTERN
            vUV = uZoomScale * uUVScale * aVertexUV;
        #endif
            gl_Position = uMVPMatrix * vec4(pos, 1.0);
        }
    )GLSL";

    static const std::string polygonFsh = R"GLSL(
        precision mediump float;
        #ifdef PATTERN
        uniform sampler2D uPattern;
        #endif
        varying lowp vec4 vColor;
        #ifdef PATTERN
        varying vec2 vUV;
        #endif

        void main(void) {
        #ifdef PATTERN
            gl_FragColor = texture2D(uPattern, vUV) * vColor;
        #else
            gl_FragColor = vColor;
        #endif
        }
    )GLSL";

    static const std::string polygon3DVsh = R"GLSL(
        attribute vec2 aVertexPosition;
        attribute vec2 aVertexBinormal;
        attribute float aVertexHeight;
        attribute vec4 aVertexAttribs;
        #ifdef TRANSFORM
        uniform mat3 uTransformMatrix;
        #endif
        uniform mat4 uMVPMatrix;
        uniform mat3 uTileMatrix;
        uniform float uVertexScale;
        uniform float uHeightScale;
        uniform vec3 uLightDir;
        uniform vec4 uColorTable[16];
        #ifdef GL_FRAGMENT_PRECISION_HIGH
        varying highp vec2 vTilePos;
        #else
        varying mediump vec2 vTilePos;
        #endif
        varying lowp vec4 vColor;
        varying mediump float vHeight;

        void main(void) {
            int styleIndex = int(aVertexAttribs[0]);
        #ifdef TRANSFORM
            vec3 pos = vec3(vec2(uTransformMatrix * vec3(aVertexPosition, 1.0)), aVertexHeight * uHeightScale);
        #else
            vec3 pos = vec3(aVertexPosition, aVertexHeight * uHeightScale);
        #endif
            vec4 color = uColorTable[styleIndex];
            if (aVertexAttribs[1] != 0.0) {
                vec3 binormal = vec3(aVertexBinormal, 0.0);
                color = vec4(color.rgb * (abs(dot(uLightDir, binormal)) * 0.5 + 0.5), color.a);
            }
            vTilePos = (uTileMatrix * vec3(vec2(pos), 1.0)).xy;
            vColor = color;
            vHeight = aVertexAttribs[2];
            gl_Position = uMVPMatrix * vec4(pos, 1.0);
        }
    )GLSL";

    static const std::string polygon3DFsh = R"GLSL(
        precision mediump float;
        #ifdef GL_FRAGMENT_PRECISION_HIGH
        varying highp vec2 vTilePos;
        #else
        varying mediump vec2 vTilePos;
        #endif
        varying lowp vec4 vColor;
        varying mediump float vHeight;

        void main(void) {
            if (min(vTilePos.x, vTilePos.y) < -0.01 || max(vTilePos.x, vTilePos.y) > 1.01) {
                discard;
            }
            gl_FragColor = vec4(vColor.rgb * (sqrt(vHeight) * 0.75 + 0.25), vColor.a);
        }
    )GLSL";
}

namespace carto { namespace vt {
    GLTileRenderer::GLTileRenderer(std::shared_ptr<std::mutex> mutex, std::shared_ptr<GLExtensions> glExtensions, float scale, bool useFBO, bool useDepth, bool useStencil) :
        _lightDir(0.5f, 0.5f, -0.707f), _projectionMatrix(cglib::mat4x4<double>::identity()), _cameraMatrix(cglib::mat4x4<double>::identity()), _cameraProjMatrix(cglib::mat4x4<double>::identity()), _labelMatrix(cglib::mat4x4<double>::identity()), _viewState(cglib::mat4x4<double>::identity(), cglib::mat4x4<double>::identity(), 0, 1, scale), _scale(scale), _useFBO(useFBO), _useDepth(useDepth), _useStencil(useStencil), _glExtensions(std::move(glExtensions)), _mutex(std::move(mutex))
    {
        _blendNodes = std::make_shared<std::vector<std::shared_ptr<BlendNode>>>();
        _bitmapLabelMap[0] = _bitmapLabelMap[1] = std::make_shared<BitmapLabelMap>();
    }
    
    void GLTileRenderer::setViewState(const cglib::mat4x4<double>& projectionMatrix, const cglib::mat4x4<double>& cameraMatrix, float zoom, float aspectRatio, float resolution) {
        std::lock_guard<std::mutex> lock(*_mutex);
        
        _projectionMatrix = projectionMatrix;
        _cameraMatrix = cameraMatrix;
        _cameraProjMatrix = projectionMatrix * cameraMatrix;
        _zoom = zoom;
        _halfResolution = resolution * 0.5f;
        _frustum = cglib::gl_projection_frustum(_cameraProjMatrix);
        _labelMatrix = calculateLocalViewMatrix(cameraMatrix);
        _viewState = ViewState(projectionMatrix, cameraMatrix, zoom, aspectRatio, _scale);
    }
    
    void GLTileRenderer::setLightDir(const cglib::vec3<float>& lightDir) {
        std::lock_guard<std::mutex> lock(*_mutex);
        
        _lightDir = lightDir;
    }

    void GLTileRenderer::setSubTileBlending(bool blend) {
        std::lock_guard<std::mutex> lock(*_mutex);

        _subTileBlending = blend;
    }
    
    void GLTileRenderer::setInteractionMode(bool enabled) {
        std::lock_guard<std::mutex> lock(*_mutex);

        _interactionEnabled = enabled;
    }

    void GLTileRenderer::setFBOClearColor(const Color& clearColor) {
        std::lock_guard<std::mutex> lock(*_mutex);
        
        _fboClearColor = clearColor;
    }
    
    void GLTileRenderer::setBackgroundColor(const Color& backgroundColor) {
        std::lock_guard<std::mutex> lock(*_mutex);
        
        _backgroundColor = backgroundColor;
    }
    
    void GLTileRenderer::setBackgroundPattern(std::shared_ptr<const BitmapPattern> pattern) {
        std::lock_guard<std::mutex> lock(*_mutex);
        
        _backgroundPattern = std::move(pattern);
    }
    
    void GLTileRenderer::setVisibleTiles(const std::map<TileId, std::shared_ptr<const Tile>>& tiles, bool blend) {
        using TilePair = std::pair<TileId, std::shared_ptr<const Tile>>;

        // Clear the 'visible' label list for now (used only for culling)
        {
            std::lock_guard<std::mutex> lock(*_mutex);
            _labels.clear();
        }

        // Build visible tile list for labels. Some tiles may be outside the frustum, ignore labels of such tiles as label rendering is quite expensive
        cglib::frustum3<double> frustum;
        {
            std::lock_guard<std::mutex> lock(*_mutex);
            frustum = _frustum;
        }
        std::vector<TilePair> orderedTiles;
        for (const TilePair& tilePair : tiles) {
            if (tilePair.second) {
                if (frustum.inside(calculateTileBBox(tilePair.first))) {
                    orderedTiles.push_back(tilePair);
                }
            }
        }

        // Order tiles by tile zoom level. This will fix flickering when multiple tiles from different zoom levels redefine same label
        std::sort(orderedTiles.begin(), orderedTiles.end(), [](const TilePair& tilePair1, const TilePair& tilePair2) {
            return tilePair2.second->getTileId() < tilePair1.second->getTileId();
        });

        // All other operations must be synchronized
        std::lock_guard<std::mutex> lock(*_mutex);
        
        // Create label list, merge geometries
        std::unordered_map<std::pair<int, long long>, std::shared_ptr<TileLabel>, LabelHash> newLabelMap;
        for (auto tileIt = orderedTiles.begin(); tileIt != orderedTiles.end(); tileIt++) {
            cglib::mat4x4<double> tileMatrix = calculateTileMatrix(tileIt->second->getTileId());
            for (const std::shared_ptr<TileLayer>& layer : tileIt->second->getLayers()) {
                for (const std::shared_ptr<TileLabel>& label : layer->getLabels()) {
                    std::pair<int, long long> labelId(layer->getLayerIndex(), label->getGlobalId());
                    label->transformGeometry(tileMatrix);
                    auto newLabelIt = newLabelMap.find(labelId);
                    if (newLabelIt != newLabelMap.end()) {
                        newLabelIt->second->mergeGeometries(*label);
                        continue;
                    }
                    newLabelMap[labelId] = label;
                }
            }
        }

        // Release old labels
        for (auto labelIt = _labelMap.begin(); labelIt != _labelMap.end();) {
            if (labelIt->second->getOpacity() <= 0) {
                labelIt = _labelMap.erase(labelIt);
            }
            else {
                labelIt->second->setActive(false);
                labelIt++;
            }
        }
        
        // Copy existing label placements
        for (auto newLabelIt = newLabelMap.begin(); newLabelIt != newLabelMap.end(); newLabelIt++) {
            auto oldLabelIt = _labelMap.find(newLabelIt->first);
            if (oldLabelIt != _labelMap.end()) {
                newLabelIt->second->setVisible(oldLabelIt->second->isVisible());
                newLabelIt->second->setOpacity(oldLabelIt->second->getOpacity());
                newLabelIt->second->snapPlacement(*oldLabelIt->second);
            }
            else {
                newLabelIt->second->setVisible(false);
                newLabelIt->second->setOpacity(0);
            }
            newLabelIt->second->setActive(true);
            _labelMap[newLabelIt->first] = newLabelIt->second;
        }
        
        // Build final label list, group labels by font bitmaps
        std::vector<std::shared_ptr<TileLabel>> labels;
        labels.reserve(_labelMap.size());
        std::array<std::shared_ptr<BitmapLabelMap>, 2> bitmapLabelMap;
        bitmapLabelMap[0] = std::make_shared<BitmapLabelMap>();
        bitmapLabelMap[1] = std::make_shared<BitmapLabelMap>();
        for (auto labelIt = _labelMap.begin(); labelIt != _labelMap.end(); labelIt++) {
            const std::shared_ptr<TileLabel>& label = labelIt->second;
            int pass = (label->getOrientation() == LabelOrientation::BILLBOARD_3D ? 1 : 0);
            (*bitmapLabelMap[pass])[label->getFont()->getGlyphMap()->getBitmapPattern()->bitmap].push_back(label);
            labels.push_back(label);
        }
        _labels = std::move(labels);
        _bitmapLabelMap = std::move(bitmapLabelMap);

        // Sort labels by priority. This is a hack implementation and is needed only for labels that 'are allowed' to overlap each other.
        // The current implementation only does proper ordering within single bitmap atlas, but usually this is enough
        for (int pass = 0; pass < 2; pass++) {
            for (auto it = _bitmapLabelMap[pass]->begin(); it != _bitmapLabelMap[pass]->end(); it++) {
                std::sort(it->second.begin(), it->second.end(), [](const std::shared_ptr<TileLabel>& label1, const std::shared_ptr<TileLabel>& label2) {
                    return label1->getPriority() < label2->getPriority();
                });
            }
        }
        
        // Build blend nodes for tiles
        auto blendNodes = std::make_shared<std::vector<std::shared_ptr<BlendNode>>>();
        blendNodes->reserve(tiles.size());
        for (const TilePair& tilePair : tiles) {
            auto blendNode = std::make_shared<BlendNode>(tilePair.first, tilePair.second, blend ? 0.0f : 1.0f);
            for (std::shared_ptr<BlendNode>& oldBlendNode : *_blendNodes) {
                if (blendNode->tileId == oldBlendNode->tileId && blendNode->tile == oldBlendNode->tile) {
                    blendNode = oldBlendNode;
                    break;
                }
                if (blend && blendNode->tileId.intersects(oldBlendNode->tileId)) {
                    if (_subTileBlending) {
                        blendNode->childNodes.push_back(oldBlendNode);
                        oldBlendNode->blend = calculateBlendNodeOpacity(*oldBlendNode, 1.0f); // this is an optimization, to reduce extensive blending subtrees
                        oldBlendNode->childNodes.clear();
                    }
                    else {
                        blendNode->blend = std::max(blendNode->blend, oldBlendNode->blend);
                    }
                }
            }
            blendNodes->push_back(std::move(blendNode));
        }
        _blendNodes = std::move(blendNodes);
    }
    
    std::vector<std::shared_ptr<TileLabel>> GLTileRenderer::getVisibleLabels() const {
        std::lock_guard<std::mutex> lock(*_mutex);
        
        return _labels;
    }
    
    void GLTileRenderer::initializeRenderer() {
        const std::map<std::string, std::pair<std::string, std::string>> shaderMap = {
            { "background", { backgroundVsh, backgroundFsh } },
            { "bitmap", { bitmapVsh, bitmapFsh } },
            { "blend", { blendVsh, blendFsh } },
            { "label", { labelVsh, labelFsh } },
            { "point", { pointVsh, pointFsh } },
            { "line", { lineVsh, lineFsh } },
            { "polygon", { polygonVsh, polygonFsh } },
            { "polygon3d", { polygon3DVsh, polygon3DFsh } }
        };

        std::lock_guard<std::mutex> lock(*_mutex);

        // Register shaders
        for (auto it = shaderMap.begin(); it != shaderMap.end(); it++) {
            _shaderManager.registerShaders(it->first, it->second.first, it->second.second);
        }
        
        // Create shader contexts
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                std::set<std::string> defs;
                if (i != 0) {
                    defs.insert("PATTERN");
                }
                if (j != 0) {
                    defs.insert("TRANSFORM");
                }
                _patternTransformContext[i][j] = std::make_shared<std::set<std::string>>(defs);
            }
        }
    }
    
    void GLTileRenderer::resetRenderer() {
        std::lock_guard<std::mutex> lock(*_mutex);
        
        // Drop all caches with texture references/FBO/VBOs
        _compiledBitmapMap.clear();
        _compiledTileBitmapMap.clear();
        _compiledTileGeometryMap.clear();
        _layerFBOs.clear();
        _screenFBO = ScreenFBO();
        _overlayFBO = ScreenFBO();

        // Reset shader programs
        _shaderManager.resetPrograms();
    }
        
    void GLTileRenderer::deinitializeRenderer() {
        std::lock_guard<std::mutex> lock(*_mutex);
        
        // Release compiled bitmaps (textures)
        for (auto it = _compiledBitmapMap.begin(); it != _compiledBitmapMap.end(); it++) {
            deleteTexture(it->second.texture);
        }
        _compiledBitmapMap.clear();

        // Release compiler tile bitmaps (textures)
        for (auto it = _compiledTileBitmapMap.begin(); it != _compiledTileBitmapMap.end(); it++) {
            deleteTexture(it->second.texture);
        }
        _compiledTileBitmapMap.clear();

        // Release compiled geometry (VBOs)
        for (auto it = _compiledTileGeometryMap.begin(); it != _compiledTileGeometryMap.end(); it++) {
            deleteVertexArray(it->second.geometryVAO);
            deleteBuffer(it->second.vertexGeometryVBO);
            deleteBuffer(it->second.indicesVBO);
        }
        _compiledTileGeometryMap.clear();
        
        // Release FBOs
        for (auto it = _layerFBOs.begin(); it != _layerFBOs.end(); it++) {
            deleteLayerFBO(*it);
        }
        _layerFBOs.clear();
        
        // Release screen and overlay FBOs
        deleteScreenFBO(_screenFBO);
        deleteScreenFBO(_overlayFBO);

        // Release shader programs
        _shaderManager.deletePrograms();
        
        _blendNodes.reset();
        _renderBlendNodes.reset();
        _bitmapLabelMap[0].reset();
        _bitmapLabelMap[1].reset();
        _renderBitmapLabelMap[0].reset();
        _renderBitmapLabelMap[1].reset();
        _labels.clear();
        _labelMap.clear();
    }
    
    void GLTileRenderer::startFrame(float dt) {
        std::lock_guard<std::mutex> lock(*_mutex);
        
        // Update geometry blending state
        _renderBlendNodes = _blendNodes;
        for (std::shared_ptr<BlendNode>& blendNode : *_renderBlendNodes) {
            updateBlendNode(*blendNode, dt);
        }
        
        // Update labels
        _renderBitmapLabelMap = _bitmapLabelMap;
        for (int pass = 0; pass < 2; pass++) {
            for (const std::pair<std::shared_ptr<const Bitmap>, std::vector<std::shared_ptr<TileLabel>>>& bitmapLabels : *_renderBitmapLabelMap[pass]) {
                updateLabels(bitmapLabels.second, dt);
            }
        }
        
        // Load viewport dimensions, update dependent values
        GLint viewport[4] = { 0, 0, 0, 0 };
        glGetIntegerv(GL_VIEWPORT, viewport);
        if (viewport[2] != _screenWidth || viewport[3] != _screenHeight) {
            _screenWidth = viewport[2];
            _screenHeight = viewport[3];

            // Release layer FBOs
            for (auto it = _layerFBOs.begin(); it != _layerFBOs.end(); it++) {
                deleteLayerFBO(*it);
            }
            _layerFBOs.clear();

            // Release screen/overlay FBOs
            deleteScreenFBO(_screenFBO);
            deleteScreenFBO(_overlayFBO);
        }
    }
    
    bool GLTileRenderer::renderGeometry2D() {
        std::lock_guard<std::mutex> lock(*_mutex);
        
        // Bind screen FBO and clear it (if FBO enabled)
        GLint currentFBO = 0;
        if (_useFBO) {
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
            
            if (_screenFBO.fbo == 0) {
                _screenFBO = createScreenFBO(_useDepth, _useStencil);
            }
            
            glBindFramebuffer(GL_FRAMEBUFFER, _screenFBO.fbo);
            glClearColor(_fboClearColor[0], _fboClearColor[1], _fboClearColor[2], _fboClearColor[3]);
            glClearStencil(0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }

        // Update GL state
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        if (_useStencil) {
            glEnable(GL_STENCIL_TEST);
            glStencilMask(255);
        }
        else {
            glDisable(GL_STENCIL_TEST);
        }
        glDisable(GL_CULL_FACE);

        // 2D geometry pass
        bool update = renderBlendNodes2D(*_renderBlendNodes);
        
        // Restore GL state
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDisable(GL_STENCIL_TEST);
        glStencilMask(255);
        glEnable(GL_CULL_FACE);
        
        // Blend screen FBO, if FBO rendering enabled
        if (_useFBO) {
            if (_glExtensions->GL_EXT_discard_framebuffer_supported() && !_screenFBO.depthStencilAttachments.empty()) {
                _glExtensions->glDiscardFramebufferEXT(GL_FRAMEBUFFER, static_cast<unsigned int>(_screenFBO.depthStencilAttachments.size()), _screenFBO.depthStencilAttachments.data());
            }
            
            glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);
            blendScreenTexture(1.0f, _screenFBO.colorTexture);
        }

        return update;
    }
    
    bool GLTileRenderer::renderGeometry3D() {
        std::lock_guard<std::mutex> lock(*_mutex);
        
        // Update GL state
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDisable(GL_STENCIL_TEST);
        glStencilMask(0);
        glDisable(GL_CULL_FACE);

        // 3D polygon pass
        bool update = renderBlendNodes3D(*_renderBlendNodes);
        
        // Restore GL state
        glEnable(GL_BLEND);
        glStencilMask(255);
        glEnable(GL_CULL_FACE);
        
        return update;
    }
    
    bool GLTileRenderer::renderLabels(bool labels2D, bool labels3D) {
        std::lock_guard<std::mutex> lock(*_mutex);
        
        // Update GL state
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glDisable(GL_STENCIL_TEST);
        glStencilMask(0);
        glDisable(GL_CULL_FACE);

        // Label pass
        bool update = false;
        for (int pass = 0; pass < 2; pass++) {
            if ((pass == 0 && labels2D) || (pass == 1 && labels3D)) {
                for (const std::pair<std::shared_ptr<const Bitmap>, std::vector<std::shared_ptr<TileLabel>>>& bitmapLabels : *_renderBitmapLabelMap[pass]) {
                    update = renderLabels(bitmapLabels.first, bitmapLabels.second) || update;
                }
            }
        }
        
        // Restore GL state
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glStencilMask(255);
        glEnable(GL_CULL_FACE);
        
        return update;
    }
    
    void GLTileRenderer::endFrame() {
        std::lock_guard<std::mutex> lock(*_mutex);
        
        // Release unused textures
        for (auto it = _compiledBitmapMap.begin(); it != _compiledBitmapMap.end();) {
            if (it->first.unique()) {
                deleteTexture(it->second.texture);
                it = _compiledBitmapMap.erase(it);
            }
            else {
                it++;
            }
        }
        
        // Release unused tile textures
        for (auto it = _compiledTileBitmapMap.begin(); it != _compiledTileBitmapMap.end();) {
            if (it->first.unique()) {
                deleteTexture(it->second.texture);
                it = _compiledTileBitmapMap.erase(it);
            }
            else {
                it++;
            }
        }

        // Release unused VBOs
        for (auto it = _compiledTileGeometryMap.begin(); it != _compiledTileGeometryMap.end();) {
            if (it->first.unique()) {
                deleteVertexArray(it->second.geometryVAO);
                deleteBuffer(it->second.vertexGeometryVBO);
                deleteBuffer(it->second.indicesVBO);
                it = _compiledTileGeometryMap.erase(it);
            }
            else {
                it++;
            }
        }
    }

    bool GLTileRenderer::findGeometryIntersections(const cglib::ray3<double>& ray, std::vector<std::tuple<TileId, double, long long>>& results, float radius, bool geom2D, bool geom3D) const {
        std::lock_guard<std::mutex> lock(*_mutex);

        // Calculate intersection with z=0 plane
        double t = 0;
        if (!cglib::intersect_plane(cglib::vec4<double>(0, 0, 1, 0), ray, &t)) {
            return false;
        }

        // First find the intersecting tile. NOTE: we ignore building height information
        std::size_t initialResults = results.size();
        for (const std::shared_ptr<BlendNode>& blendNode : *_blendNodes) {
            std::multimap<int, RenderNode> renderNodeMap;
            if (!buildRenderNodes(*blendNode, 1.0f, renderNodeMap)) {
                continue;
            }

            // Check that the hit point is inside the blend node tile (used for clipping)
            cglib::mat4x4<double> tileMatrix = calculateTileMatrix(blendNode->tileId);
            cglib::vec3<double> posLocal = cglib::transform_point(ray(t), cglib::inverse(tileMatrix));
            double radiusLocal = radius / cglib::length(cglib::proj_o(cglib::row_vector(tileMatrix, 0)));
            if (!(posLocal(0) >= -radiusLocal && posLocal(0) <= 1 + radiusLocal && posLocal(1) >= -radiusLocal && posLocal(1) <= 1 + radiusLocal)) {
                continue;
            }

            for (auto it = renderNodeMap.begin(); it != renderNodeMap.end(); it++) {
                const RenderNode& renderNode = it->second;

                // Check that the hit point is inside the render node tile (actual tile we are performing intersection test)
                tileMatrix = calculateTileMatrix(renderNode.tileId);
                posLocal = cglib::transform_point(ray(t), cglib::inverse(tileMatrix));
                radiusLocal = radius / cglib::length(cglib::proj_o(cglib::row_vector(tileMatrix, 0)));
                if (!(posLocal(0) >= -radiusLocal && posLocal(0) <= 1 + radiusLocal && posLocal(1) >= -radiusLocal && posLocal(1) <= 1 + radiusLocal)) {
                    continue;
                }

                // Test all geometry batches for intersections
                for (const std::shared_ptr<TileGeometry>& geometry : renderNode.layer->getGeometries()) {
                    bool polygon3D = geometry->getType() == TileGeometry::Type::POLYGON3D;
                    if ((!polygon3D && geom2D) || (polygon3D && geom3D)) {
                        cglib::ray3<float> rayLocal = cglib::ray3<float>::convert(cglib::transform_ray(ray, cglib::inverse(tileMatrix)));

                        std::vector<std::pair<float, long long>> resultsLocal;
                        findTileGeometryIntersections(renderNode.tileId, geometry, rayLocal, static_cast<float>(radiusLocal), resultsLocal);

                        for (std::pair<float, long long> resultLocal : resultsLocal) {
                            float tLocal = resultLocal.first;
                            long long id = resultLocal.second;
                            cglib::vec3<double> pos = cglib::transform_point(cglib::vec3<double>(rayLocal(tLocal)(0), rayLocal(tLocal)(1), 0), tileMatrix);
                            results.emplace_back(renderNode.tileId, cglib::dot_product(pos - ray.origin, ray.direction) / cglib::dot_product(ray.direction, ray.direction), id);
                        }
                    }
                }
            }
        }

        return results.size() > initialResults;
    }
    
    bool GLTileRenderer::findLabelIntersections(const cglib::ray3<double>& ray, std::vector<std::tuple<TileId, double, long long>>& results, float radius, bool labels2D, bool labels3D) const {
        std::lock_guard<std::mutex> lock(*_mutex);

        // Test for label intersections. The ordering may be mixed compared to actual rendering order, but this is non-issue if the labels are non-overlapping.
        std::size_t initialResults = results.size();
        for (int pass = 0; pass < 2; pass++) {
            if ((pass == 0 && labels2D) || (pass == 1 && labels3D)) {
                for (const std::pair<std::shared_ptr<const Bitmap>, std::vector<std::shared_ptr<TileLabel>>>& bitmapLabels : *_renderBitmapLabelMap[pass]) {
                    for (const std::shared_ptr<TileLabel>& label : bitmapLabels.second) {
                        if (!label->isActive() || !label->isVisible() || label->getOpacity() <= 0) {
                            continue;
                        }

                        double result = 0;
                        if (findLabelIntersection(label, ray, radius, result)) {
                            results.emplace_back(label->getTileId(), result, label->getLocalId());
                        }
                    }
                }
            }
        }

        return results.size() > initialResults;
    }
    
    cglib::mat4x4<double> GLTileRenderer::calculateLocalViewMatrix(const cglib::mat4x4<double>& cameraMatrix) {
        cglib::mat4x4<double> mv = cameraMatrix;
        mv(0, 3) = mv(1, 3) = mv(2, 3) = 0;
        return mv;
    }
    
    cglib::mat4x4<double> GLTileRenderer::calculateTileMatrix(const TileId& tileId, float coordScale) const {
        double z = 1.0 / (1 << tileId.zoom);
        cglib::mat4x4<double> m = cglib::mat4x4<double>::zero();
        m(0, 0) = z * _scale * coordScale;
        m(1, 1) = -z * _scale * coordScale;
        m(2, 2) = _scale * coordScale;
        m(0, 3) = tileId.x * z * _scale - 0.5 * _scale;
        m(1, 3) = ((1 << tileId.zoom) - tileId.y) * z * _scale - 0.5 * _scale;
        m(2, 3) = 0;
        m(3, 3) = 1;
        return m;
    }
    
    cglib::mat4x4<float> GLTileRenderer::calculateTileMVPMatrix(const TileId& tileId, float coordScale) const {
        cglib::mat4x4<float> mvp = cglib::mat4x4<float>::convert(_cameraProjMatrix * calculateTileMatrix(tileId, coordScale));
        
        // Update mvp so that mvp(x+1,y)*[0,0,0,1]^T = mvp(x,y)*[1/coordScale,0,0,1]^T and mvp(x,y+1)*[0,0,0,1]^T = mvp(x,y)*[0, 1/coordScale,0,1]^T
        cglib::vec4<float> p00 = cglib::vec4<float>::convert(cglib::transform(calculateTileOrigin(tileId), _cameraProjMatrix));
        cglib::vec4<float> p10 = cglib::vec4<float>::convert(cglib::transform(calculateTileOrigin(TileId(tileId.zoom, tileId.x + 1, tileId.y + 0)), _cameraProjMatrix));
        cglib::vec4<float> p01 = cglib::vec4<float>::convert(cglib::transform(calculateTileOrigin(TileId(tileId.zoom, tileId.x + 0, tileId.y + 1)), _cameraProjMatrix));
        for (int i = 0; i < 4; i++) {
            mvp(i, 0) = (p10(i) - p00(i)) * coordScale;
            mvp(i, 1) = (p01(i) - p00(i)) * coordScale;
        }
        return mvp;
    }
    
    cglib::mat3x3<double> GLTileRenderer::calculateTileMatrix2D(const TileId& tileId, float coordScale) const {
        cglib::mat4x4<double> m0 = calculateTileMatrix(tileId, coordScale);
        cglib::mat3x3<double> m1;
        m1(0, 0) = m0(0, 0);
        m1(0, 1) = m0(0, 1);
        m1(0, 2) = m0(0, 3);
        m1(1, 0) = m0(1, 0);
        m1(1, 1) = m0(1, 1);
        m1(1, 2) = m0(1, 3);
        m1(2, 0) = m0(3, 0);
        m1(2, 1) = m0(3, 1);
        m1(2, 2) = m0(3, 3);
        return m1;
    }
    
    cglib::vec4<double> GLTileRenderer::calculateTileOrigin(const TileId& tileId) const {
        double z = 1.0 / (1 << tileId.zoom);
        cglib::vec4<double> p;
        p(0) = tileId.x * z * _scale - 0.5 * _scale;
        p(1) = ((1 << tileId.zoom) - tileId.y) * z * _scale - 0.5 * _scale;
        p(2) = 0;
        p(3) = 1;
        return p;
    }
    
    cglib::bbox3<double> GLTileRenderer::calculateTileBBox(const TileId& tileId) const {
        return cglib::transform_bbox(cglib::bbox3<double>(cglib::vec3<double>(0, 0, 0), cglib::vec3<double>(1, 1, 0)), calculateTileMatrix(tileId));
    }

    float GLTileRenderer::calculateBlendNodeOpacity(const BlendNode& blendNode, float blend) const {
        float opacity = blend * blendNode.blend;
        for (const std::shared_ptr<BlendNode>& childBlendNode : blendNode.childNodes) {
            opacity += calculateBlendNodeOpacity(*childBlendNode, blend * (1 - blendNode.blend));
        }
        return std::min(opacity, 1.0f);
    }

    void GLTileRenderer::updateBlendNode(BlendNode& blendNode, float dBlend) const {
        if (!_frustum.inside(calculateTileBBox(blendNode.tileId))) {
            blendNode.blend = 1.0f;
            return;
        }
        
        blendNode.blend += dBlend;
        if (blendNode.blend >= 1.0f) {
            blendNode.blend = 1.0f;
            blendNode.childNodes.clear();
        }
        
        for (std::shared_ptr<BlendNode>& childBlendNode : blendNode.childNodes) {
            updateBlendNode(*childBlendNode, dBlend);
        }
    }
    
    bool GLTileRenderer::buildRenderNodes(const BlendNode& blendNode, float blend, std::multimap<int, RenderNode>& renderNodeMap) const {
        if (!_frustum.inside(calculateTileBBox(blendNode.tileId))) {
            return false;
        }
        
        bool exists = false;
        if (blendNode.tile) {
            // Use original source tile id for render node, but apply tile shift from target tile
            TileId rootTileId = blendNode.tileId;
            while (rootTileId.zoom > 0) {
                rootTileId = rootTileId.getParent();
            }
            TileId tileId = blendNode.tile->getTileId();
            tileId.x += rootTileId.x * (1 << tileId.zoom);
            tileId.y += rootTileId.y * (1 << tileId.zoom);
            
            // Add render nodes for each layer
            for (const std::shared_ptr<TileLayer>& layer : blendNode.tile->getLayers()) {
                // Special case for raster layers - ignore global blend factor
                RenderNode renderNode(tileId, layer, (layer->getGeometries().empty() ? blendNode.blend : blend * blendNode.blend));
                addRenderNode(renderNode, renderNodeMap);
            }
            exists = true;
        }
        
        for (const std::shared_ptr<BlendNode>& childBlendNode : blendNode.childNodes) {
            if (buildRenderNodes(*childBlendNode, blend * (1 - blendNode.blend), renderNodeMap)) {
                exists = true;
            }
        }
        return exists;
    }
    
    void GLTileRenderer::addRenderNode(RenderNode renderNode, std::multimap<int, RenderNode>& renderNodeMap) const {
        const std::shared_ptr<const TileLayer>& layer = renderNode.layer;
        auto range = renderNodeMap.equal_range(layer->getLayerIndex());
        for (auto it = range.first; it != range.second; ) {
            RenderNode& baseRenderNode = (it++)->second;
            if (!renderNode.tileId.intersects(baseRenderNode.tileId)) {
                continue;
            }

            // Check if the layer should be combined with base layer
            bool combine = false;
            if (!layer->getGeometries().empty() && !baseRenderNode.layer->getGeometries().empty()) {
                TileGeometry::Type type = layer->getGeometries().front()->getType();
                TileGeometry::Type baseType = baseRenderNode.layer->getGeometries().front()->getType();
                combine = (type == baseType);
            }
            
            // Combine layers, if possible
            if (combine) {
                if (baseRenderNode.tileId.zoom > renderNode.tileId.zoom) {
                    renderNode.blend = std::max(renderNode.blend, std::min(renderNode.initialBlend + baseRenderNode.blend, 1.0f));
                    it = renderNodeMap.erase(--it);
                }
                else {
                    baseRenderNode.blend = std::max(baseRenderNode.blend, std::min(baseRenderNode.initialBlend + renderNode.blend, 1.0f));
                    return;
                }
            }
        }
        
        // New/non-intersecting layer. Add it to render node map.
        auto it = renderNodeMap.lower_bound(layer->getLayerIndex());
        renderNodeMap.insert(it, { layer->getLayerIndex(), renderNode });
    }
    
    void GLTileRenderer::updateLabels(const std::vector<std::shared_ptr<TileLabel>>& labels, float dOpacity) const {
        for (const std::shared_ptr<TileLabel>& label : labels) {
            if (!label->isValid()) {
                continue;
            }
            
            float sign = (label->isVisible() && label->isActive() ? 1.0f : -1.0f);
            float step = (label->getOpacity() <= 0.0f || label->getOpacity() >= 1.0f ? 0.01f : dOpacity); // important if dOpacity is highly variable - if fully hidden/visible, take small first step
            label->setOpacity(std::max(0.0f, std::min(1.0f, label->getOpacity() + sign * step)));
        }
    }
    
    void GLTileRenderer::setupPointCoordinateSystem(PointOrientation orientation, const TileId& tileId, float vertexScale, cglib::vec3<float>& xAxis, cglib::vec3<float>& yAxis) const {
        switch (orientation) {
        case PointOrientation::BILLBOARD_2D:
            xAxis = _viewState.orientation[0];
            yAxis = cglib::vector_product(cglib::vec3<float>(0, 0, 1), xAxis);
            break;
        case PointOrientation::BILLBOARD_3D:
            xAxis = _viewState.orientation[0];
            yAxis = _viewState.orientation[1];
            break;
        case PointOrientation::POINT:
            xAxis = cglib::vec3<float>(1, 0, 0);
            yAxis = cglib::vec3<float>(0, 1, 0);
            break;
        }
        cglib::mat4x4<float> invTileMatrix = cglib::mat4x4<float>::convert(cglib::inverse(calculateTileMatrix(tileId, 1.0f / vertexScale)));
        xAxis = cglib::transform_vector(xAxis * _viewState.scale, invTileMatrix);
        yAxis = cglib::transform_vector(yAxis * _viewState.scale, invTileMatrix);
    }

    void GLTileRenderer::findTileGeometryIntersections(const TileId& tileId, const std::shared_ptr<TileGeometry>& geometry, const cglib::ray3<float>& ray, float radius, std::vector<std::pair<float, long long>>& results) const {
        cglib::vec3<float> xAxis, yAxis;
        setupPointCoordinateSystem(geometry->getStyleParameters().pointOrientation, tileId, geometry->getGeometryLayoutParameters().vertexScale, xAxis, yAxis);

        for (std::size_t i = 0; i + 2 < geometry->getIndices().size(); i += 3) {
            std::size_t index0 = geometry->getIndices()[i + 0];
            std::size_t index1 = geometry->getIndices()[i + 1];
            std::size_t index2 = geometry->getIndices()[i + 2];

            cglib::vec3<float> p0 = decodeVertex(geometry, index0);
            cglib::vec3<float> p1 = decodeVertex(geometry, index1);
            cglib::vec3<float> p2 = decodeVertex(geometry, index2);

            if (geometry->getType() == TileGeometry::Type::POINT) {
                p0 += extendOffset(decodePointOffset(geometry, index0, xAxis, yAxis), radius);
                p1 += extendOffset(decodePointOffset(geometry, index1, xAxis, yAxis), radius);
                p2 += extendOffset(decodePointOffset(geometry, index2, xAxis, yAxis), radius);
            }
            else if (geometry->getType() == TileGeometry::Type::LINE) {
                p0 += extendOffset(decodeLineBinormal(geometry, index0), radius);
                p1 += extendOffset(decodeLineBinormal(geometry, index1), radius);
                p2 += extendOffset(decodeLineBinormal(geometry, index2), radius);
            }
            else if (geometry->getType() == TileGeometry::Type::POLYGON) {
                cglib::vec3<float> center = (p0 + p1 + p2) * (1.0f / 3.0f);
                p0 = center + extendOffset(p0 - center, radius);
                p1 = center + extendOffset(p1 - center, radius);
                p2 = center + extendOffset(p2 - center, radius);
            }
            else if (geometry->getType() == TileGeometry::Type::POLYGON3D) {
                p0 += decodePolygon3DOffset(geometry, index0);
                p1 += decodePolygon3DOffset(geometry, index1);
                p2 += decodePolygon3DOffset(geometry, index2);
            }

            float t = 0;
            if (cglib::intersect_triangle(p0, p1, p2, ray, &t)) {
                std::size_t counter = i;
                for (std::size_t j = 0; j < geometry->getIds().size(); j++) {
                    if (counter < geometry->getIds()[j].first) {
                        results.emplace_back(t, geometry->getIds()[j].second);
                        break;
                    }
                    counter -= geometry->getIds()[j].first;
                }
            }
        }
    }

    bool GLTileRenderer::findLabelIntersection(const std::shared_ptr<TileLabel>& label, const cglib::ray3<double>& ray, float radius, double& result) const {
        std::array<cglib::vec3<float>, 4> envelope;
        if (!label->calculateEnvelope(_viewState, envelope)) {
            return false;
        }

        cglib::vec3<float> center = (envelope[0] + envelope[1] + envelope[2] + envelope[3]) * 0.25f;

        std::array<cglib::vec3<double>, 4> p;
        for (int i = 0; i < 4; i++) {
            cglib::vec3<float> dp = center + extendOffset(envelope[i] - center, radius);
            p[i] = _viewState.origin + cglib::vec3<double>::convert(dp);
        }

        return cglib::intersect_triangle(p[0], p[1], p[2], ray, &result) ||
               cglib::intersect_triangle(p[0], p[2], p[3], ray, &result);
    }

    cglib::vec3<float> GLTileRenderer::extendOffset(const cglib::vec3<float>& offset, float radius) {
        float len = cglib::length(offset);
        if (len == 0) {
            return offset;
        }
        return offset * (1.0f + radius / len);
    }

    cglib::vec3<float> GLTileRenderer::decodeVertex(const std::shared_ptr<TileGeometry>& geometry, std::size_t index) const {
        const TileGeometry::GeometryLayoutParameters& geometryLayoutParams = geometry->getGeometryLayoutParameters();
        std::size_t vertexOffset = index * geometryLayoutParams.vertexSize + geometryLayoutParams.vertexOffset;
        const short* vertexPtr = reinterpret_cast<const short*>(&geometry->getVertexGeometry()[vertexOffset]);
        return cglib::vec3<float>(vertexPtr[0], vertexPtr[1], 0) * (1.0f / geometryLayoutParams.vertexScale);
    }

    cglib::vec3<float> GLTileRenderer::decodePointOffset(const std::shared_ptr<TileGeometry>& geometry, std::size_t index, const cglib::vec3<float>& xAxis, const cglib::vec3<float>& yAxis) const {
        const TileGeometry::GeometryLayoutParameters& geometryLayoutParams = geometry->getGeometryLayoutParameters();
        std::size_t binormalOffset = index * geometryLayoutParams.vertexSize + geometryLayoutParams.binormalOffset;
        const short* binormalPtr = reinterpret_cast<const short*>(&geometry->getVertexGeometry()[binormalOffset]);
        cglib::vec2<float> xy = cglib::vec2<float>(binormalPtr[0], binormalPtr[1]) * (geometry->getGeometryScale() / geometry->getTileSize() / geometryLayoutParams.binormalScale);
        return xAxis * xy(0) + yAxis * xy(1);
    }

    cglib::vec3<float> GLTileRenderer::decodeLineBinormal(const std::shared_ptr<TileGeometry>& geometry, std::size_t index) const {
        const TileGeometry::GeometryLayoutParameters& geometryLayoutParams = geometry->getGeometryLayoutParameters();
        std::size_t binormalOffset = index * geometryLayoutParams.vertexSize + geometryLayoutParams.binormalOffset;
        const short* binormalPtr = reinterpret_cast<const short*>(&geometry->getVertexGeometry()[binormalOffset]);
        std::size_t attribOffset = index * geometryLayoutParams.vertexSize + geometryLayoutParams.attribsOffset;
        const char* attribPtr = reinterpret_cast<const char*>(&geometry->getVertexGeometry()[attribOffset]);
        float width = 0.5f * (*geometry->getStyleParameters().widthTable[attribPtr[0]])(_viewState) * geometry->getGeometryScale() / geometry->getTileSize();
        return cglib::vec3<float>(binormalPtr[0], binormalPtr[1], 0) * (width / geometryLayoutParams.binormalScale);
    }

    cglib::vec3<float> GLTileRenderer::decodePolygon3DOffset(const std::shared_ptr<TileGeometry>& geometry, std::size_t index) const {
        const TileGeometry::GeometryLayoutParameters& geometryLayoutParams = geometry->getGeometryLayoutParameters();
        std::size_t heightOffset = index * geometryLayoutParams.vertexSize + geometryLayoutParams.heightOffset;
        const float* heightPtr = reinterpret_cast<const float*>(&geometry->getVertexGeometry()[heightOffset]);
        return cglib::vec3<float>(0, 0, *heightPtr);
    }

    bool GLTileRenderer::renderBlendNodes2D(const std::vector<std::shared_ptr<BlendNode>>& blendNodes) {
        GLint stencilBits = 0;
        if (_useStencil) {
            glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
        }
        
        int stencilNum = (1 << stencilBits) - 1; // forces initial stencil clear
        TileId activeTileMaskId(-1, 0, 0); // invalid mask
        bool update = false;
        for (const std::shared_ptr<BlendNode>& blendNode : blendNodes) {
            std::multimap<int, RenderNode> renderNodeMap;
            if (!buildRenderNodes(*blendNode, 1.0f, renderNodeMap)) {
                continue;
            }
            
            std::unordered_map<int, std::size_t> layerFBOMap;
            if (stencilBits > 0) {
                glStencilFunc(GL_ALWAYS, stencilNum, 255);
            }
            float backgroundOpacity = calculateBlendNodeOpacity(*blendNode, 1.0f);
            renderTileBackground(blendNode->tileId, backgroundOpacity);
            update = backgroundOpacity < 1.0f || update;
            if (stencilBits > 0) {
                glStencilFunc(GL_EQUAL, stencilNum, 255);
            }
            
            for (auto it = renderNodeMap.begin(); it != renderNodeMap.end(); it++) {
                const RenderNode& renderNode = it->second;
                
                float blendOpacity = 1.0f, geometryOpacity = 1.0f;
                if (renderNode.layer->getOpacity()) {
                    float opacity = (*renderNode.layer->getOpacity())(_viewState);
                    if (renderNode.layer->getCompOp()) { // a 'useful' hack - we use real layer opacity only if comp-op is explicitly defined; otherwise we translate it into element opacity, which is in many cases close enough
                        blendOpacity = opacity;
                    } else {
                        geometryOpacity = opacity;
                    }
                }

                GLint currentFBO = 0;
                bool blendGeometry = false;

                auto setupFrameBuffer = [&]() {
                    if (renderNode.layer->getCompOp() && !blendGeometry) {
                        blendGeometry = true;

                        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);

                        auto it = layerFBOMap.find(renderNode.layer->getLayerIndex());
                        if (it == layerFBOMap.end()) {
                            std::size_t fboIndex = layerFBOMap.size();
                            if (fboIndex >= _layerFBOs.size()) {
                                _layerFBOs.push_back(createLayerFBO(stencilBits > 0));
                            }
                            it = layerFBOMap.emplace(renderNode.layer->getLayerIndex(), fboIndex).first;
                        }
                        glBindFramebuffer(GL_FRAMEBUFFER, _layerFBOs[it->second].fbo);
                        glClearColor(0, 0, 0, 0);
                        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

                        activeTileMaskId = TileId(-1, -1, -1); // force stencil mask update
                    }
                };

                if (renderNode.layer->getCompOp()) {
                    if (isEmptyBlendRequired(renderNode.layer->getCompOp().get())) {
                        setupFrameBuffer();
                    }
                }

                for (const std::shared_ptr<TileBitmap>& bitmap : renderNode.layer->getBitmaps()) {
                    setupFrameBuffer();
                    
                    setBlendState(CompOp::SRC_OVER);
                    renderTileBitmap(renderNode.tileId, blendNode->tileId, renderNode.blend, geometryOpacity, bitmap);
                }
                
                for (const std::shared_ptr<TileGeometry>& geometry : renderNode.layer->getGeometries()) {
                    if (geometry->getType() == TileGeometry::Type::POLYGON3D) {
                        continue;
                    }

                    setupFrameBuffer();

                    TileId tileMaskId = renderNode.tileId.zoom > blendNode->tileId.zoom ? renderNode.tileId : blendNode->tileId;
                    if (activeTileMaskId != tileMaskId && stencilBits > 0) {
                        if (++stencilNum == (1 << stencilBits)) { // do initial clear, or clear after each N (usually 256) updates
                            glClearStencil(0);
                            glClear(GL_STENCIL_BUFFER_BIT);
                            stencilNum = 1;
                        }

                        glStencilFunc(GL_ALWAYS, stencilNum, 255);
                        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
                        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

                        renderTileMask(tileMaskId);

                        glStencilFunc(GL_EQUAL, stencilNum, 255);
                        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
                        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

                        activeTileMaskId = tileMaskId;
                    }

                    setBlendState(geometry->getStyleParameters().compOp);
                    renderTileGeometry(renderNode.tileId, blendNode->tileId, renderNode.blend, geometryOpacity, geometry);
                }
                update = renderNode.initialBlend < 1.0f || update;

                if (blendGeometry) {
                    glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);
                    if (stencilBits > 0) {
                        glStencilFunc(GL_ALWAYS, stencilNum, 255);
                    }

                    setBlendState(renderNode.layer->getCompOp().get());
                    blendTileTexture(renderNode.tileId, blendOpacity, _layerFBOs[layerFBOMap[renderNode.layer->getLayerIndex()]].colorTexture);
                }
            }
        }
        
        return update;
    }
    
    bool GLTileRenderer::renderBlendNodes3D(const std::vector<std::shared_ptr<BlendNode>>& blendNodes) {
        bool update = false;
        GLint currentFBO = 0;
        bool blendGeometry = false;
        for (const std::shared_ptr<BlendNode>& blendNode : blendNodes) {
            std::multimap<int, RenderNode> renderNodeMap;
            if (!buildRenderNodes(*blendNode, 1.0f, renderNodeMap)) {
                continue;
            }
            
            for (auto it = renderNodeMap.begin(); it != renderNodeMap.end(); it++) {
                const RenderNode& renderNode = it->second;

                float opacity = 1.0f;
                if (renderNode.layer->getCompOp() && renderNode.layer->getOpacity()) {
                    opacity = (*renderNode.layer->getOpacity())(_viewState);
                }

                for (const std::shared_ptr<TileGeometry>& geometry : renderNode.layer->getGeometries()) {
                    if (geometry->getType() != TileGeometry::Type::POLYGON3D) {
                        continue;
                    }
                    
                    // Bind screen FBO lazily, only when needed
                    if (!blendGeometry) {
                        blendGeometry = true;
                        
                        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);

                        if (_overlayFBO.fbo == 0) {
                            _overlayFBO = createScreenFBO(true, false);
                        }

                        glBindFramebuffer(GL_FRAMEBUFFER, _overlayFBO.fbo);
                        glClearColor(0, 0, 0, 0);
                        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    }
                    
                    renderTileGeometry(renderNode.tileId, blendNode->tileId, renderNode.blend, opacity, geometry);
                }
                update = renderNode.initialBlend < 1.0f || update;
            }
        }
        
        // If any 3D geometry, blend rendered screen FBO
        if (blendGeometry) {
            glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);

            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

            blendScreenTexture(1.0f, _overlayFBO.colorTexture);

            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
        }

        return update;
    }
    
    bool GLTileRenderer::renderLabels(const std::shared_ptr<const Bitmap>& bitmap, const std::vector<std::shared_ptr<TileLabel>>& labels) {
        bool update = false;
        
        for (const std::shared_ptr<TileLabel>& label : labels) {
            if (!label->isValid()) {
                continue;
            }
            if (label->getOpacity() <= 0.0f) {
                continue;
            }
            
            std::size_t verticesSize = _labelVertices.size();
            label->calculateVertexData(_viewState, _labelVertices, _labelTexCoords, _labelIndices);
            Color color = label->getColor() * label->getOpacity();
            _labelColors.fill(color.rgba(), _labelVertices.size() - verticesSize);

            if (_labelVertices.size() >= 32768) { // flush the batch if largest vertex index is getting 'close' to 64k limit
                renderLabelBatch(bitmap);
            }
            
            update = label->getOpacity() < 1.0f || update;
        }

        renderLabelBatch(bitmap);

        return update;
    }
    
    void GLTileRenderer::blendScreenTexture(float opacity, GLuint texture) {
        static const GLfloat screenVertices[8] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };
        
        if (opacity <= 0) {
            return;
        }

        GLuint shaderProgram = _shaderManager.createProgram("blend", _patternTransformContext[0][0]);
        glUseProgram(shaderProgram);
        checkGLError();
        
        glVertexAttribPointer(glGetAttribLocation(shaderProgram, "aVertexPosition"), 2, GL_FLOAT, GL_FALSE, 0, &screenVertices[0]);
        glEnableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexPosition"));
        
        cglib::mat4x4<float> mvpMatrix = cglib::mat4x4<float>::identity();
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uMVPMatrix"), 1, GL_FALSE, mvpMatrix.data());
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);
        Color color(opacity, opacity, opacity, opacity);
        glUniform4fv(glGetUniformLocation(shaderProgram, "uColor"), 1, color.rgba().data());
        glUniform2f(glGetUniformLocation(shaderProgram, "uInvScreenSize"), 1.0f / _screenWidth, 1.0f / _screenHeight);
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        glBindTexture(GL_TEXTURE_2D, 0);
        
        glDisableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexPosition"));
    }

    void GLTileRenderer::blendTileTexture(const TileId& tileId, float opacity, GLuint texture) {
        static const GLfloat tileVertices[8] = { 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };

        if (opacity <= 0) {
            return;
        }
        
        GLuint shaderProgram = _shaderManager.createProgram("blend", _patternTransformContext[0][0]);
        glUseProgram(shaderProgram);
        checkGLError();

        glVertexAttribPointer(glGetAttribLocation(shaderProgram, "aVertexPosition"), 2, GL_FLOAT, GL_FALSE, 0, &tileVertices[0]);
        glEnableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexPosition"));

        cglib::mat4x4<float> mvpMatrix = calculateTileMVPMatrix(tileId);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uMVPMatrix"), 1, GL_FALSE, mvpMatrix.data());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);
        Color color(opacity, opacity, opacity, opacity);
        glUniform4fv(glGetUniformLocation(shaderProgram, "uColor"), 1, color.rgba().data());
        glUniform2f(glGetUniformLocation(shaderProgram, "uInvScreenSize"), 1.0f / _screenWidth, 1.0f / _screenHeight);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        glBindTexture(GL_TEXTURE_2D, 0);

        glDisableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexPosition"));
    }
    
    void GLTileRenderer::renderTileMask(const TileId& tileId) {
        static const GLfloat tileVertices[8] = { 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };

        GLuint shaderProgram = _shaderManager.createProgram("background", _patternTransformContext[0][0]);
        glUseProgram(shaderProgram);
        checkGLError();
        
        glVertexAttribPointer(glGetAttribLocation(shaderProgram, "aVertexPosition"), 2, GL_FLOAT, GL_FALSE, 0, &tileVertices[0]);
        glEnableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexPosition"));
        
        cglib::mat4x4<float> mvpMatrix = calculateTileMVPMatrix(tileId);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uMVPMatrix"), 1, GL_FALSE, mvpMatrix.data());
        
        Color color(0, 0, 0, 0);
        glUniform4fv(glGetUniformLocation(shaderProgram, "uColor"), 1, color.rgba().data());
        glUniform1f(glGetUniformLocation(shaderProgram, "uOpacity"), 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        glDisableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexPosition"));
    }
    
    void GLTileRenderer::renderTileBackground(const TileId& tileId, float opacity) {
        static const GLfloat tileVertices[8] = { 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };

        if (opacity <= 0) {
            return;
        }
        if (!_backgroundPattern && !_backgroundColor.value()) {
            return;
        }

        GLuint shaderProgram = _shaderManager.createProgram("background", _patternTransformContext[_backgroundPattern ? 1 : 0][0]);
        glUseProgram(shaderProgram);
        checkGLError();
        
        glVertexAttribPointer(glGetAttribLocation(shaderProgram, "aVertexPosition"), 2, GL_FLOAT, GL_FALSE, 0, &tileVertices[0]);
        glEnableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexPosition"));
        
        cglib::mat4x4<float> mvpMatrix = calculateTileMVPMatrix(tileId);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uMVPMatrix"), 1, GL_FALSE, mvpMatrix.data());
        
        if (_backgroundPattern) {
            CompiledBitmap compiledBitmap;
            auto it = _compiledBitmapMap.find(_backgroundPattern->bitmap);
            if (it == _compiledBitmapMap.end()) {
                std::shared_ptr<const Bitmap> patternBitmap = BitmapManager::scaleToPOT(_backgroundPattern->bitmap);
                compiledBitmap.texture = createTexture();
                glBindTexture(GL_TEXTURE_2D, compiledBitmap.texture);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, patternBitmap->width, patternBitmap->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, patternBitmap->data.data());
                glGenerateMipmap(GL_TEXTURE_2D);
                
                _compiledBitmapMap[_backgroundPattern->bitmap] = compiledBitmap;
            }
            else {
                compiledBitmap = it->second;
            }
            
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, compiledBitmap.texture);
            glUniform1i(glGetUniformLocation(shaderProgram, "uPattern"), 0);
        }

        glUniform4fv(glGetUniformLocation(shaderProgram, "uColor"), 1, _backgroundColor.rgba().data());
        glUniform1f(glGetUniformLocation(shaderProgram, "uOpacity"), opacity);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        
        glDisableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexPosition"));
    }

    void GLTileRenderer::renderTileBitmap(const TileId& tileId, const TileId& targetTileId, float blend, float opacity, const std::shared_ptr<TileBitmap>& bitmap) {
        static const GLfloat tileVertices[8] = { 1.0f, 0.0f, 1.0f, 1.0f,0.0f, 0.0f, 0.0f, 1.0f };

        if (blend * opacity <= 0) {
            return;
        }

        GLuint shaderProgram = _shaderManager.createProgram("bitmap", _patternTransformContext[0][0]);
        glUseProgram(shaderProgram);
        checkGLError();

        glVertexAttribPointer(glGetAttribLocation(shaderProgram, "aVertexPosition"), 2, GL_FLOAT, GL_FALSE, 0, &tileVertices[0]);
        glEnableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexPosition"));

        cglib::mat4x4<float> mvpMatrix = calculateTileMVPMatrix(targetTileId.zoom > tileId.zoom ? targetTileId : tileId);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uMVPMatrix"), 1, GL_FALSE, mvpMatrix.data());

        CompiledBitmap compiledTileBitmap;
        auto it = _compiledTileBitmapMap.find(bitmap);
        if (it == _compiledTileBitmapMap.end()) {
            // Use a different strategy is the bitmap is not of POT dimensions, simply do not create the mipmaps
            bool pow2Size = (bitmap->getWidth() & (bitmap->getWidth() - 1)) == 0 && (bitmap->getHeight() & (bitmap->getHeight() - 1)) == 0;

            compiledTileBitmap.texture = createTexture();
            glBindTexture(GL_TEXTURE_2D, compiledTileBitmap.texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, pow2Size ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            GLenum format = GL_NONE;
            switch (bitmap->getFormat()) {
            case TileBitmap::Format::GRAYSCALE:
                format = GL_LUMINANCE;
                break;
            case TileBitmap::Format::RGB:
                format = GL_RGB;
                break;
            case TileBitmap::Format::RGBA:
                format = GL_RGBA;
                break;
            }
            glTexImage2D(GL_TEXTURE_2D, 0, format, bitmap->getWidth(), bitmap->getHeight(), 0, format, GL_UNSIGNED_BYTE, bitmap->getData().data());
            if (pow2Size) {
                glGenerateMipmap(GL_TEXTURE_2D);
            }

            _compiledTileBitmapMap[bitmap] = compiledTileBitmap;
        }
        else {
            compiledTileBitmap = it->second;
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, compiledTileBitmap.texture);
        glUniform1i(glGetUniformLocation(shaderProgram, "uPattern"), 0);

        int deltaMask = (1 << (std::max(tileId.zoom, targetTileId.zoom) - tileId.zoom)) - 1;
        float s = 1.0f / (1 << (std::max(tileId.zoom, targetTileId.zoom) - tileId.zoom));
        float x = (targetTileId.x & deltaMask) * s;
        float y = (targetTileId.y & deltaMask) * s;
        glUniform2fv(glGetUniformLocation(shaderProgram, "uUVScale"), 1, cglib::vec2<float>(s, s).data());
        glUniform2fv(glGetUniformLocation(shaderProgram, "uUVOffset"), 1, cglib::vec2<float>(x, y).data());

        glUniform1f(glGetUniformLocation(shaderProgram, "uOpacity"), blend * opacity);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glDisableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexPosition"));
    }

    void GLTileRenderer::renderTileGeometry(const TileId& tileId, const TileId& targetTileId, float blend, float opacity, const std::shared_ptr<TileGeometry>& geometry) {
        const TileGeometry::StyleParameters& styleParams = geometry->getStyleParameters();
        const TileGeometry::GeometryLayoutParameters& geometryLayoutParams = geometry->getGeometryLayoutParameters();
        
        if (blend * opacity <= 0) {
            return;
        }
        
        GLuint shaderProgram = 0;
        switch (geometry->getType()) {
            case TileGeometry::Type::POINT:
                shaderProgram = _shaderManager.createProgram("point", _patternTransformContext[styleParams.pattern ? 1 : 0][styleParams.transform ? 1 : 0]);
                break;
            case TileGeometry::Type::LINE:
                shaderProgram = _shaderManager.createProgram("line", _patternTransformContext[styleParams.pattern ? 1 : 0][styleParams.transform ? 1 : 0]);
                break;
            case TileGeometry::Type::POLYGON:
                shaderProgram = _shaderManager.createProgram("polygon", _patternTransformContext[styleParams.pattern ? 1 : 0][styleParams.transform ? 1 : 0]);
                break;
            case TileGeometry::Type::POLYGON3D:
                shaderProgram = _shaderManager.createProgram("polygon3d", _patternTransformContext[0][styleParams.transform ? 1 : 0]);
                break;
            default:
                return;
        }
        glUseProgram(shaderProgram);
        checkGLError();
        
        CompiledGeometry compiledGeometry;
        auto itGeom = _compiledTileGeometryMap.find(geometry);
        if (itGeom == _compiledTileGeometryMap.end()) {
            if (_glExtensions->GL_OES_vertex_array_object_supported()) {
                compiledGeometry.geometryVAO = createVertexArray();
            }
            
            compiledGeometry.vertexGeometryVBO = createBuffer();
            glBindBuffer(GL_ARRAY_BUFFER, compiledGeometry.vertexGeometryVBO);
            glBufferData(GL_ARRAY_BUFFER, geometry->getVertexGeometry().size() * sizeof(unsigned char), geometry->getVertexGeometry().data(), GL_STATIC_DRAW);
            
            compiledGeometry.indicesVBO = createBuffer();
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, compiledGeometry.indicesVBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry->getIndices().size() * sizeof(unsigned short), geometry->getIndices().data(), GL_STATIC_DRAW);
            
            if (!_interactionEnabled) {
                geometry->releaseVertexArrays(); // if interaction is enabled, we must keep the vertex arrays. Otherwise optimize for lower memory usage
            }
            
            _compiledTileGeometryMap[geometry] = compiledGeometry;
        }
        else {
            compiledGeometry = itGeom->second;
        }
        
        if (styleParams.pattern) {
            glUniform1f(glGetUniformLocation(shaderProgram, "uUVScale"), 1.0f / (geometryLayoutParams.texCoordScale * styleParams.pattern->widthScale));
            glUniform1f(glGetUniformLocation(shaderProgram, "uZoomScale"), std::pow(2.0f, static_cast<int>(_zoom) - tileId.zoom));
            
            CompiledBitmap compiledBitmap;
            auto itBitmap = _compiledBitmapMap.find(styleParams.pattern->bitmap);
            if (itBitmap == _compiledBitmapMap.end()) {
                bool genMipmaps = geometry->getType() != TileGeometry::Type::LINE;
                std::shared_ptr<const Bitmap> patternBitmap = BitmapManager::scaleToPOT(styleParams.pattern->bitmap);
                compiledBitmap.texture = createTexture();
                glBindTexture(GL_TEXTURE_2D, compiledBitmap.texture);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, genMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, patternBitmap->width, patternBitmap->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, patternBitmap->data.data());
                if (genMipmaps) {
                    glGenerateMipmap(GL_TEXTURE_2D);
                }
                
                _compiledBitmapMap[styleParams.pattern->bitmap] = compiledBitmap;
            }
            else {
                compiledBitmap = itBitmap->second;
            }
            
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, compiledBitmap.texture);
            glUniform1i(glGetUniformLocation(shaderProgram, "uPattern"), 0);
        }
        
        cglib::mat4x4<float> mvpMatrix = calculateTileMVPMatrix(tileId, 1.0f / geometryLayoutParams.vertexScale);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uMVPMatrix"), 1, GL_FALSE, mvpMatrix.data());
        
        if (styleParams.transform) {
            float transformScale = geometryLayoutParams.vertexScale * std::pow(2.0f, tileId.zoom - _zoom) / geometry->getTileSize();
            cglib::mat3x3<float> transformMatrix = styleParams.transform.get();
            transformMatrix(0, 2) *= transformScale;
            transformMatrix(1, 2) *= transformScale;
            glUniformMatrix3fv(glGetUniformLocation(shaderProgram, "uTransformMatrix"), 1, GL_FALSE, transformMatrix.data());
        }

        std::array<cglib::vec4<float>, TileGeometry::StyleParameters::MAX_PARAMETERS> colors;
        for (int i = 0; i < styleParams.parameterCount; i++) {
            Color color = (*styleParams.colorTable[i])(_viewState) * (blend * opacity * (*styleParams.opacityTable[i])(_viewState));
            colors[i] = color.rgba();
        }
        
        if (geometry->getType() == TileGeometry::Type::POINT) {
            cglib::vec3<float> xAxis, yAxis;
            setupPointCoordinateSystem(styleParams.pointOrientation, tileId, geometryLayoutParams.vertexScale, xAxis, yAxis);
            
            glUniform1f(glGetUniformLocation(shaderProgram, "uBinormalScale"), geometry->getGeometryScale() / geometry->getTileSize() / geometryLayoutParams.binormalScale);
            glUniform3fv(glGetUniformLocation(shaderProgram, "uXAxis"), 1, xAxis.data());
            glUniform3fv(glGetUniformLocation(shaderProgram, "uYAxis"), 1, yAxis.data());
        }
        else if (geometry->getType() == TileGeometry::Type::LINE) {
            float gamma = 0.5f;
            std::array<float, TileGeometry::StyleParameters::MAX_PARAMETERS> widths;
            for (int i = 0; i < styleParams.parameterCount; i++) {
                float width = 0.5f * (*styleParams.widthTable[i])(_viewState) * geometry->getGeometryScale() / geometry->getTileSize();
                float pixelWidth = 2.0f * _halfResolution * width;
                if (pixelWidth < 1.0f) {
                    colors[i] = colors[i] * pixelWidth; // should do gamma correction here, but simple implementation gives closer results to Mapnik
                    width = 1.0f / (2.0f * _halfResolution);
                }
                widths[i] = width;
            }
            
            glUniform1f(glGetUniformLocation(shaderProgram, "uBinormalScale"), geometryLayoutParams.vertexScale / (_halfResolution * geometryLayoutParams.binormalScale * std::pow(2.0f, _zoom - tileId.zoom)));
            glUniform1fv(glGetUniformLocation(shaderProgram, "uWidthTable"), styleParams.parameterCount, widths.data());
            glUniform1f(glGetUniformLocation(shaderProgram, "uHalfResolution"), _halfResolution);
            glUniform1f(glGetUniformLocation(shaderProgram, "uGamma"), gamma);
        }
        else if (geometry->getType() == TileGeometry::Type::POLYGON3D) {
            glUniform1f(glGetUniformLocation(shaderProgram, "uVertexScale"), 1.0f / geometryLayoutParams.vertexScale);
            glUniform1f(glGetUniformLocation(shaderProgram, "uHeightScale"), blend * geometryLayoutParams.vertexScale);
            cglib::vec3<float> lightDir = _lightDir * (1.0f / geometryLayoutParams.binormalScale);
            glUniform3fv(glGetUniformLocation(shaderProgram, "uLightDir"), 1, lightDir.data());
            cglib::mat3x3<float> tileMatrix = cglib::mat3x3<float>::convert(cglib::inverse(calculateTileMatrix2D(targetTileId)) * calculateTileMatrix2D(tileId, 1.0f / geometryLayoutParams.vertexScale));
            glUniformMatrix3fv(glGetUniformLocation(shaderProgram, "uTileMatrix"), 1, GL_FALSE, tileMatrix.data());
        }
        
        glUniform4fv(glGetUniformLocation(shaderProgram, "uColorTable"), styleParams.parameterCount, colors[0].data());
        
        if (_glExtensions->GL_OES_vertex_array_object_supported()) {
            _glExtensions->glBindVertexArrayOES(compiledGeometry.geometryVAO);
        }
        if (!(_glExtensions->GL_OES_vertex_array_object_supported() && itGeom != _compiledTileGeometryMap.end())) {
            glBindBuffer(GL_ARRAY_BUFFER, compiledGeometry.vertexGeometryVBO);
            glVertexAttribPointer(glGetAttribLocation(shaderProgram, "aVertexPosition"), 2, GL_SHORT, GL_FALSE, geometryLayoutParams.vertexSize, reinterpret_cast<const GLvoid*>(geometryLayoutParams.vertexOffset));
            glEnableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexPosition"));
            glVertexAttribPointer(glGetAttribLocation(shaderProgram, "aVertexAttribs"), 4, GL_BYTE, GL_FALSE, geometryLayoutParams.vertexSize, reinterpret_cast<const GLvoid*>(geometryLayoutParams.attribsOffset));
            glEnableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexAttribs"));
            
            if (geometryLayoutParams.texCoordOffset >= 0) {
                glVertexAttribPointer(glGetAttribLocation(shaderProgram, "aVertexUV"), 2, GL_SHORT, GL_FALSE, geometryLayoutParams.vertexSize, reinterpret_cast<const GLvoid*>(geometryLayoutParams.texCoordOffset));
                glEnableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexUV"));
            }
            
            if (geometryLayoutParams.binormalOffset >= 0) {
                glVertexAttribPointer(glGetAttribLocation(shaderProgram, "aVertexBinormal"), 2, GL_SHORT, GL_FALSE, geometryLayoutParams.vertexSize, reinterpret_cast<const GLvoid*>(geometryLayoutParams.binormalOffset));
                glEnableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexBinormal"));
            }
            
            if (geometryLayoutParams.heightOffset >= 0) {
                glVertexAttribPointer(glGetAttribLocation(shaderProgram, "aVertexHeight"), 1, GL_FLOAT, GL_FALSE, geometryLayoutParams.vertexSize, reinterpret_cast<const GLvoid*>(geometryLayoutParams.heightOffset));
                glEnableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexHeight"));
            }
            
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, compiledGeometry.indicesVBO);
        }
        
        glDrawElements(GL_TRIANGLES, geometry->getIndicesCount(), GL_UNSIGNED_SHORT, 0);
        
        if (_glExtensions->GL_OES_vertex_array_object_supported()) {
            _glExtensions->glBindVertexArrayOES(0);
        }
        else {
            glDisableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexAttribs"));
            
            if (geometryLayoutParams.heightOffset >= 0) {
                glDisableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexHeight"));
            }
            
            if (geometryLayoutParams.binormalOffset >= 0) {
                glDisableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexBinormal"));
            }
            
            if (geometryLayoutParams.texCoordOffset >= 0) {
                glDisableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexUV"));
            }
            
            glDisableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexPosition"));
        }
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void GLTileRenderer::renderLabelBatch(const std::shared_ptr<const Bitmap>& bitmap) {
        if (_labelIndices.empty()) {
            return;
        }

        GLuint shaderProgram = _shaderManager.createProgram("label", _patternTransformContext[0][0]);
        glUseProgram(shaderProgram);
        checkGLError();

        cglib::mat4x4<float> mvpMatrix = cglib::mat4x4<float>::convert(_projectionMatrix * _labelMatrix);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uMVPMatrix"), 1, GL_FALSE, mvpMatrix.data());

        glVertexAttribPointer(glGetAttribLocation(shaderProgram, "aVertexPosition"), 3, GL_FLOAT, GL_FALSE, 0, _labelVertices.data());
        glEnableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexPosition"));

        glVertexAttribPointer(glGetAttribLocation(shaderProgram, "aVertexUV"), 2, GL_FLOAT, GL_FALSE, 0, _labelTexCoords.data());
        glEnableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexUV"));

        glVertexAttribPointer(glGetAttribLocation(shaderProgram, "aVertexColor"), 4, GL_FLOAT, GL_FALSE, 0, _labelColors.data());
        glEnableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexColor"));

        CompiledBitmap compiledBitmap;
        auto it = _compiledBitmapMap.find(bitmap);
        if (it == _compiledBitmapMap.end()) {
            compiledBitmap.texture = createTexture();
            glBindTexture(GL_TEXTURE_2D, compiledBitmap.texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            if (_glExtensions->GL_EXT_texture_filter_anisotropic_supported()) {
                GLint maxAnisotropy = 0;
                glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
                if (maxAnisotropy > 1) {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(4, maxAnisotropy));
                }
            }
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap->width, bitmap->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap->data.data());
            glGenerateMipmap(GL_TEXTURE_2D);

            _compiledBitmapMap[bitmap] = compiledBitmap;
        }
        else {
            compiledBitmap = it->second;
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, compiledBitmap.texture);
        glUniform1i(glGetUniformLocation(shaderProgram, "uBitmap"), 0);
        glUniform2f(glGetUniformLocation(shaderProgram, "uUVScale"), 1.0f / bitmap->width, 1.0f / bitmap->height);

        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(_labelIndices.size()), GL_UNSIGNED_SHORT, _labelIndices.data());

        glDisableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexColor"));

        glDisableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexUV"));

        glDisableVertexAttribArray(glGetAttribLocation(shaderProgram, "aVertexPosition"));

        _labelVertices.clear();
        _labelTexCoords.clear();
        _labelColors.clear();
        _labelIndices.clear();
    }

    void GLTileRenderer::setBlendState(CompOp compOp) {
        struct GLBlendState {
            GLenum blendEquation;
            GLenum blendFuncSrc;
            GLenum blendFuncDst;
        };
        static const std::map<CompOp, GLBlendState> compOpBlendStates = {
            { CompOp::SRC,      { GL_FUNC_ADD, GL_ONE, GL_ZERO } },
            { CompOp::SRC_OVER, { GL_FUNC_ADD, GL_ONE, GL_ONE_MINUS_SRC_ALPHA } },
            { CompOp::SRC_IN,   { GL_FUNC_ADD, GL_DST_ALPHA, GL_ZERO } },
            { CompOp::SRC_ATOP, { GL_FUNC_ADD, GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA } },
            { CompOp::DST,      { GL_FUNC_ADD, GL_ZERO, GL_ONE } },
            { CompOp::DST_OVER, { GL_FUNC_ADD, GL_ONE_MINUS_DST_ALPHA, GL_ONE } },
            { CompOp::DST_IN,   { GL_FUNC_ADD, GL_ZERO, GL_SRC_ALPHA } },
            { CompOp::DST_ATOP, { GL_FUNC_ADD, GL_ONE_MINUS_DST_ALPHA, GL_SRC_ALPHA } },
            { CompOp::ZERO,     { GL_FUNC_ADD, GL_ZERO, GL_ZERO } },
            { CompOp::PLUS,     { GL_FUNC_ADD, GL_ONE, GL_ONE } },
            { CompOp::MINUS,    { GL_FUNC_REVERSE_SUBTRACT, GL_ONE, GL_ONE } },
            { CompOp::MULTIPLY, { GL_FUNC_ADD, GL_DST_COLOR, GL_ZERO } },
            { CompOp::DARKEN,   { GL_MIN_EXT,  GL_ONE, GL_ONE } },
            { CompOp::LIGHTEN,  { GL_MAX_EXT,  GL_ONE, GL_ONE } },
        };
        auto it = compOpBlendStates.find(compOp);
        if (it != compOpBlendStates.end()) {
            glBlendEquation(it->second.blendEquation);
            glBlendFunc(it->second.blendFuncSrc, it->second.blendFuncDst);
        }
    }

    bool GLTileRenderer::isEmptyBlendRequired(CompOp compOp) const {
        switch (compOp) {
        case CompOp::SRC:
        case CompOp::SRC_OVER:
        case CompOp::DST_OVER:
        case CompOp::DST_ATOP:
        case CompOp::PLUS:
        case CompOp::MINUS:
        case CompOp::LIGHTEN:
            return false;
        default:
            return true;
        }
    }
    
    void GLTileRenderer::checkGLError() {
        for (GLenum error = glGetError(); error != GL_NONE; error = glGetError()) {
            assert(error != GL_NONE);
        }
    }
    
    GLuint GLTileRenderer::createBuffer() {
        GLuint buffer = 0;
        glGenBuffers(1, &buffer);
        return buffer;
    }
    
    void GLTileRenderer::deleteBuffer(GLuint& buffer) {
        if (buffer != 0) {
            glDeleteBuffers(1, &buffer);
            buffer = 0;
        }
    }
    
    GLuint GLTileRenderer::createVertexArray() {
        GLuint vertexArray = 0;
        _glExtensions->glGenVertexArraysOES(1, &vertexArray);
        return vertexArray;
    }
    
    void GLTileRenderer::deleteVertexArray(GLuint& vertexArray) {
        if (vertexArray != 0) {
            _glExtensions->glDeleteVertexArraysOES(1, &vertexArray);
            vertexArray = 0;
        }
    }
    
    GLuint GLTileRenderer::createTexture() {
        GLuint texture = 0;
        glGenTextures(1, &texture);
        return texture;
    }
    
    void GLTileRenderer::deleteTexture(GLuint& texture) {
        if (texture != 0) {
            glDeleteTextures(1, &texture);
            texture = 0;
        }
    }

    GLTileRenderer::LayerFBO GLTileRenderer::createLayerFBO(bool useStencil) {
        LayerFBO layerFBO;

        glGenFramebuffers(1, &layerFBO.fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, layerFBO.fbo);

        if (useStencil) {
            glGenRenderbuffers(1, &layerFBO.stencilRB);
            glBindRenderbuffer(GL_RENDERBUFFER, layerFBO.stencilRB);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, _screenWidth, _screenHeight);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, layerFBO.stencilRB);
        }

        layerFBO.colorTexture = createTexture();
        glBindTexture(GL_TEXTURE_2D, layerFBO.colorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _screenWidth, _screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, layerFBO.colorTexture, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            checkGLError();
        }
        return layerFBO;
    }

    void GLTileRenderer::deleteLayerFBO(LayerFBO& layerFBO) {
        if (layerFBO.fbo != 0) {
            glDeleteFramebuffers(1, &layerFBO.fbo);
            layerFBO.fbo = 0;
        }
        if (layerFBO.stencilRB != 0) {
            glDeleteRenderbuffers(1, &layerFBO.stencilRB);
            layerFBO.stencilRB = 0;
        }
        deleteTexture(layerFBO.colorTexture);
    }

    GLTileRenderer::ScreenFBO GLTileRenderer::createScreenFBO(bool useDepth, bool useStencil) {
        ScreenFBO screenFBO;

        glGenFramebuffers(1, &screenFBO.fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, screenFBO.fbo);

        if (useDepth || useStencil) {
            glGenRenderbuffers(1, &screenFBO.depthStencilRB);
            glBindRenderbuffer(GL_RENDERBUFFER, screenFBO.depthStencilRB);
            if (useDepth && useStencil && _glExtensions->GL_OES_packed_depth_stencil_supported()) {
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, _screenWidth, _screenHeight);
                screenFBO.depthStencilAttachments.push_back(GL_DEPTH_ATTACHMENT);
                screenFBO.depthStencilAttachments.push_back(GL_STENCIL_ATTACHMENT);
            }
            else if (useDepth) {
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, _screenWidth, _screenHeight);
                screenFBO.depthStencilAttachments.push_back(GL_DEPTH_ATTACHMENT);
            }
            else if (useStencil) {
                glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, _screenWidth, _screenHeight);
                screenFBO.depthStencilAttachments.push_back(GL_STENCIL_ATTACHMENT);
            }
            for (GLenum attachment : screenFBO.depthStencilAttachments) {
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, screenFBO.depthStencilRB);
            }
        }

        screenFBO.colorTexture = createTexture();
        glBindTexture(GL_TEXTURE_2D, screenFBO.colorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _screenWidth, _screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenFBO.colorTexture, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            checkGLError();
        }
        return screenFBO;
    }

    void GLTileRenderer::deleteScreenFBO(ScreenFBO& screenFBO) {
        if (screenFBO.fbo != 0) {
            glDeleteFramebuffers(1, &screenFBO.fbo);
            screenFBO.fbo = 0;
        }
        if (screenFBO.depthStencilRB != 0) {
            glDeleteRenderbuffers(1, &screenFBO.depthStencilRB);
            screenFBO.depthStencilRB = 0;
        }
        deleteTexture(screenFBO.colorTexture);
    }
} }
