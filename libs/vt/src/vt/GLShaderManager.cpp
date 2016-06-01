#include "GLShaderManager.h"

#include <vector>

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
        #ifdef PATTERN
        attribute vec2 aVertexUV;
        #endif
        attribute vec4 aVertexAttribs;
        #ifdef PATTERN
        uniform float uUVScale;
        #endif
        uniform float uBinormalScale;
        uniform float uHalfResolution;
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

        void main(void) {
            int styleIndex = int(aVertexAttribs[0]);
            float width = uWidthTable[styleIndex] * uHalfResolution;
            vec2 binormal = vec2(aVertexAttribs[1], aVertexAttribs[2]);
        #ifdef TRANSFORM
            vec3 pos = vec3(vec2(uTransformMatrix * vec3(aVertexPosition, 1.0)) + uBinormalScale * width * binormal, 0.0);
        #else
            vec3 pos = vec3(aVertexPosition + uBinormalScale * width * binormal, 0.0);
        #endif
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
    GLuint GLShaderManager::createProgram(const std::string& programName, const ShaderContext& context) {
        static const std::map<std::string, const std::string*> vertexShaderMap = {
            { "background", &backgroundVsh },
            { "bitmap", &bitmapVsh },
            { "blend", &blendVsh },
            { "label", &labelVsh },
            { "point", &pointVsh },
            { "line", &lineVsh },
            { "polygon", &polygonVsh },
            { "polygon3d", &polygon3DVsh },
        };

        static const std::map<std::string, const std::string*> fragmentShaderMap = {
            { "background", &backgroundFsh },
            { "bitmap", &bitmapFsh },
            { "blend", &blendFsh },
            { "label", &labelFsh },
            { "point", &pointFsh },
            { "line", &lineFsh },
            { "polygon", &polygonFsh },
            { "polygon3d", &polygon3DFsh },
        };

        auto it = _programMap.find(std::make_pair(programName, context));
        if (it != _programMap.end()) {
            return it->second;
        }

        GLuint vertexShader = 0, fragmentShader = 0, program = 0;
        try {
            fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            std::string fragmentShaderSourceStr = createShader(programName, fragmentShaderMap, *context);
            const char* fragmentShaderSource = fragmentShaderSourceStr.c_str();
            glShaderSource(fragmentShader, 1, const_cast<const char**>(&fragmentShaderSource), NULL);
            glCompileShader(fragmentShader);
            GLint isShaderCompiled = 0;
            glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isShaderCompiled);
            if (!isShaderCompiled) {
                GLint infoLogLength = 0;
                glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLogLength);
                std::vector<char> infoLog(infoLogLength + 1);
                GLsizei charactersWritten = 0;
                glGetShaderInfoLog(fragmentShader, infoLogLength, &charactersWritten, infoLog.data());
                throw std::runtime_error(std::string(infoLog.begin(), infoLog.begin() + charactersWritten));
            }

            vertexShader = glCreateShader(GL_VERTEX_SHADER);
            std::string vertexShaderSourceStr = createShader(programName, vertexShaderMap, *context);
            const char* vertexShaderSource = vertexShaderSourceStr.c_str();
            glShaderSource(vertexShader, 1, const_cast<const char**>(&vertexShaderSource), NULL);
            glCompileShader(vertexShader);
            glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isShaderCompiled);
            if (!isShaderCompiled) {
                GLint infoLogLength = 0;
                glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLogLength);
                std::vector<char> infoLog(infoLogLength + 1);
                GLsizei charactersWritten = 0;
                glGetShaderInfoLog(vertexShader, infoLogLength, &charactersWritten, infoLog.data());
                throw std::runtime_error(std::string(infoLog.begin(), infoLog.begin() + charactersWritten));
            }

            program = glCreateProgram();
            glAttachShader(program, fragmentShader);
            glAttachShader(program, vertexShader);
            glLinkProgram(program);
            GLint isLinked = 0;
            glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
            if (!isLinked) {
                GLint infoLogLength = 0;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
                std::vector<char> infoLog(infoLogLength + 1);
                GLsizei charactersWritten = 0;
                glGetProgramInfoLog(program, infoLogLength, &charactersWritten, infoLog.data());
                throw std::runtime_error(std::string(infoLog.begin(), infoLog.begin() + charactersWritten));
            }

            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
        }
        catch (...) {
            if (vertexShader != 0) {
                glDeleteShader(vertexShader);
            }
            if (fragmentShader != 0) {
                glDeleteShader(fragmentShader);
            }
            if (program != 0) {
                glDeleteProgram(program);
            }
            throw;
        }

        _programMap[std::make_pair(programName, context)] = program;
        return program;
    }

    void GLShaderManager::resetPrograms() {
        _programMap.clear();
    }

    void GLShaderManager::deletePrograms() {
        for (auto it = _programMap.begin(); it != _programMap.end(); it++) {
            GLuint program = it->second;
            glDeleteProgram(program);
        }
        _programMap.clear();
    }

    std::string GLShaderManager::createShader(const std::string& name, const std::map<std::string, const std::string*>& shaderMap, const std::set<std::string>& defs) {
        auto it = shaderMap.find(name);
        if (it == shaderMap.end()) {
            throw std::runtime_error("Missing shader " + name);
        }
        const std::string& glslShader = *it->second;

        std::string glslDefs;
        for (auto it2 = defs.begin(); it2 != defs.end(); it2++) {
            glslDefs += "#define " + *it2 + "\n";
        }

        return glslDefs + glslShader;
    }
} }
