#include "ShaderManager.h"

#include <vector>

namespace carto { namespace nmlgl {

    GLuint ShaderManager::createProgram(const std::string& vertexShader, const std::string& fragmentShader, const std::set<std::string>& defs) {
        std::pair<std::pair<std::string, std::string>, std::set<std::string>> program = std::make_pair(std::make_pair(vertexShader, fragmentShader), defs);
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

    void ShaderManager::resetPrograms() {
        _programMap.clear();
    }

    void ShaderManager::deletePrograms() {
        for (auto it = _programMap.begin(); it != _programMap.end(); it++) {
            GLuint program = it->second;
            glDeleteProgram(program);
        }
        _programMap.clear();
    }

    std::string ShaderManager::createShader(const std::string& shader, const std::set<std::string>& defs) {
        std::string glslDefs;
        for (auto it2 = defs.begin(); it2 != defs.end(); it2++) {
            glslDefs += "#define " + *it2 + "\n";
        }

        return glslDefs + shader;
    }

} }
