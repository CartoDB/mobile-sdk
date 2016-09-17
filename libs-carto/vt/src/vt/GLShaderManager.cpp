#include "GLShaderManager.h"

#include <vector>

namespace carto { namespace vt {
    void GLShaderManager::registerShaders(const std::string& programName, const std::string& vsh, const std::string& fsh) {
        _vertexShaderMap[programName] = vsh;
        _fragmentShaderMap[programName] = fsh;
    }

    GLuint GLShaderManager::createProgram(const std::string& programName, const ShaderContext& context) {
        auto it = _programMap.find({ programName, context });
        if (it != _programMap.end()) {
            return it->second;
        }

        GLuint vertexShader = 0, fragmentShader = 0, program = 0;
        try {
            fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            std::string fragmentShaderSourceStr = createShader(programName, _fragmentShaderMap, *context);
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
            std::string vertexShaderSourceStr = createShader(programName, _vertexShaderMap, *context);
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

        _programMap[{ programName, context }] = program;
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

    std::string GLShaderManager::createShader(const std::string& name, const std::map<std::string, std::string>& shaderMap, const std::set<std::string>& defs) {
        auto it = shaderMap.find(name);
        if (it == shaderMap.end()) {
            throw std::runtime_error("Missing shader " + name);
        }
        const std::string& glslShader = it->second;

        std::string glslDefs;
        for (auto it2 = defs.begin(); it2 != defs.end(); it2++) {
            glslDefs += "#define " + *it2 + "\n";
        }

        return glslDefs + glslShader;
    }
} }
