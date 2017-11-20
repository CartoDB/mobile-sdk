#include "GLResourceManager.h"
#include "GLTexture.h"
#include "GLSubmesh.h"

#include <vector>

namespace carto { namespace nml {

    GLuint GLResourceManager::createProgram(const std::string& vertexShader, const std::string& fragmentShader, const std::set<std::string>& defs) {
        std::pair<std::pair<std::string, std::string>, std::set<std::string>> program{ { vertexShader, fragmentShader }, defs };
        auto it = _programMap.find(program);
        if (it != _programMap.end()) {
            return it->second;
        }

        GLuint vertexShaderId = 0, fragmentShaderId = 0, programId = 0;
        try {
            fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
            std::string fragmentShaderSourceStr = createShader(fragmentShader, defs);
            const char* fragmentShaderSource = fragmentShaderSourceStr.c_str();
            glShaderSource(fragmentShaderId, 1, const_cast<const char**>(&fragmentShaderSource), NULL);
            glCompileShader(fragmentShaderId);
            GLint isShaderCompiled = 0;
            glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &isShaderCompiled);
            if (!isShaderCompiled) {
                GLint infoLogLength = 0;
                glGetShaderiv(fragmentShaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
                std::vector<char> infoLog(infoLogLength + 1);
                GLsizei charactersWritten = 0;
                glGetShaderInfoLog(fragmentShaderId, infoLogLength, &charactersWritten, infoLog.data());
                throw std::runtime_error(std::string(infoLog.begin(), infoLog.begin() + charactersWritten));
            }

            vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
            std::string vertexShaderSourceStr = createShader(vertexShader, defs);
            const char* vertexShaderSource = vertexShaderSourceStr.c_str();
            glShaderSource(vertexShaderId, 1, const_cast<const char**>(&vertexShaderSource), NULL);
            glCompileShader(vertexShaderId);
            glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &isShaderCompiled);
            if (!isShaderCompiled) {
                GLint infoLogLength = 0;
                glGetShaderiv(vertexShaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
                std::vector<char> infoLog(infoLogLength + 1);
                GLsizei charactersWritten = 0;
                glGetShaderInfoLog(vertexShaderId, infoLogLength, &charactersWritten, infoLog.data());
                throw std::runtime_error(std::string(infoLog.begin(), infoLog.begin() + charactersWritten));
            }

            programId = glCreateProgram();
            glAttachShader(programId, fragmentShaderId);
            glAttachShader(programId, vertexShaderId);
            glLinkProgram(programId);
            GLint isLinked = 0;
            glGetProgramiv(programId, GL_LINK_STATUS, &isLinked);
            if (!isLinked) {
                GLint infoLogLength = 0;
                glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
                std::vector<char> infoLog(infoLogLength + 1);
                GLsizei charactersWritten = 0;
                glGetProgramInfoLog(programId, infoLogLength, &charactersWritten, infoLog.data());
                throw std::runtime_error(std::string(infoLog.begin(), infoLog.begin() + charactersWritten));
            }

            glDeleteShader(vertexShaderId);
            glDeleteShader(fragmentShaderId);
        }
        catch (...) {
            if (vertexShaderId != 0) {
                glDeleteShader(vertexShaderId);
            }
            if (fragmentShaderId != 0) {
                glDeleteShader(fragmentShaderId);
            }
            if (programId != 0) {
                glDeleteProgram(programId);
            }
            throw;
        }

        _programMap[program] = programId;
        return programId;
    }

    GLuint GLResourceManager::allocateTexture(const std::shared_ptr<GLTexture>& texture) {
        GLuint texId = 0;
        glGenTextures(1, &texId);

        _textureMap[texture].push_back(texId);
        return texId;
    }

    GLuint GLResourceManager::allocateBuffer(const std::shared_ptr<GLSubmesh>& submesh) {
        GLuint bufId = 0;
        glGenBuffers(1, &bufId);

        _bufferMap[submesh].push_back(bufId);
        return bufId;
    }

    void GLResourceManager::deleteUnused() {
        for (auto it = _textureMap.begin(); it != _textureMap.end(); ) {
            if (it->first.expired()) {
                const std::vector<GLuint>& texIds = it->second;
                glDeleteTextures(texIds.size(), texIds.data());
                it = _textureMap.erase(it);
            } else {
                it++;
            }
        }

        for (auto it = _bufferMap.begin(); it != _bufferMap.end(); ) {
            if (it->first.expired()) {
                const std::vector<GLuint>& bufIds = it->second;
                glDeleteBuffers(bufIds.size(), bufIds.data());
                it = _bufferMap.erase(it);
            } else {
                it++;
            }
        }
    }

    void GLResourceManager::deleteAll() {
        for (auto it = _programMap.begin(); it != _programMap.end(); it++) {
            GLuint programId = it->second;
            glDeleteProgram(programId);
        }
        _programMap.clear();

        for (auto it = _textureMap.begin(); it != _textureMap.end(); it++) {
            const std::vector<GLuint>& texIds = it->second;
            glDeleteTextures(texIds.size(), texIds.data());
        }
        _textureMap.clear();

        for (auto it = _bufferMap.begin(); it != _bufferMap.end(); it++) {
            const std::vector<GLuint>& bufIds = it->second;
            glDeleteBuffers(bufIds.size(), bufIds.data());
        }
        _bufferMap.clear();
    }

    void GLResourceManager::resetAll() {
        _programMap.clear();
        _textureMap.clear();
        _bufferMap.clear();
    }

    std::string GLResourceManager::createShader(const std::string& shader, const std::set<std::string>& defs) {
        std::string glslDefs;
        for (auto it2 = defs.begin(); it2 != defs.end(); it2++) {
            glslDefs += "#define " + *it2 + "\n";
        }

        return glslDefs + shader;
    }

} }
