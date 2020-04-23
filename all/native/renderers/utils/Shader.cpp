#include "Shader.h"
#include "renderers/utils/GLResourceManager.h"
#include "utils/Log.h"

#include <vector>

namespace carto {

    Shader::~Shader() {
    }

    GLuint Shader::getProgId() const {
        create();
        return _progId;
    }
    
    GLuint Shader::getUniformLoc(const std::string& uniformName) const {
        auto it = _uniformMap.find(uniformName);
        if (it == _uniformMap.end()) {
            Log::Errorf("Shader::getUniformLoc: Uniform '%s' not found in shader '%s'", uniformName.c_str(), _name.c_str());
            return 0;
        }
        return it->second;
    }
    
    GLuint Shader::getAttribLoc(const std::string& attribName) const {
        auto it = _attribMap.find(attribName);
        if (it == _attribMap.end()) {
            Log::Errorf("Shader::getAttribLoc: Attribute '%s' not found in shader '%s'", attribName.c_str(), _name.c_str());
            return 0;
        }
        return it->second;
    }
    
    Shader::Shader(const std::weak_ptr<GLResourceManager>& manager, const std::string& name, const std::string& vertSource, const std::string& fragSource) :
        GLResource(manager),
        _name(name),
        _vertSource(vertSource),
        _fragSource(fragSource),
        _progId(0),
        _vertShaderId(0),
        _fragShaderId(0),
        _uniformMap(),
        _attribMap()
    {
    }

    void Shader::create() const {
        if (_progId == 0) {
            _vertShaderId = LoadShader(_name, _vertSource, GL_VERTEX_SHADER);
            _fragShaderId = LoadShader(_name, _fragSource, GL_FRAGMENT_SHADER);
            _progId = LoadProg(_name, _vertShaderId, _fragShaderId);

            registerVars(_progId);
        }
    }

    void Shader::destroy() const {
        if (_vertShaderId) {
            glDeleteShader(_vertShaderId);
            _vertShaderId = 0;
        }
        
        if (_fragShaderId) {
            glDeleteShader(_fragShaderId);
            _fragShaderId = 0;
        }
        
        if (_progId) {
            glDeleteProgram(_progId);
            _progId = 0;
        }
        
        _uniformMap.clear();
        _attribMap.clear();
        
        GLContext::CheckGLError("Shader::destroy");
    }
    
    void Shader::registerVars(GLuint progId) const {
        enum { VAR_NAME_BUF_SIZE = 256 };
        char varNameBuf[VAR_NAME_BUF_SIZE];
        GLint count = 0;
        
        // Assign a location for every uniform variable, save them to map
        glGetProgramiv(progId, GL_ACTIVE_UNIFORMS, &count);
        for (GLuint tsj = 0; tsj < (GLuint) count; tsj++) {
            GLsizei actualLength = 0;
            GLint size = 0;
            GLenum type = 0;
            glGetActiveUniform(progId, tsj, VAR_NAME_BUF_SIZE, &actualLength, &size, &type, varNameBuf);
            std::string varName(varNameBuf, actualLength);
            GLuint loc = glGetUniformLocation(progId, varName.c_str());
            _uniformMap[varName] = loc;
        }
        
        // Assign a location for every attribute variable, save them to map
        glGetProgramiv(progId, GL_ACTIVE_ATTRIBUTES, &count);
        for (GLuint tsj = 0; tsj < (GLuint) count; tsj++) {
            GLsizei actualLength = 0;
            GLint size = 0;
            GLenum type = 0;
            glGetActiveAttrib(progId, tsj, VAR_NAME_BUF_SIZE, &actualLength, &size, &type, varNameBuf);
            std::string varName(varNameBuf, actualLength);
            GLuint loc = glGetAttribLocation(progId, varName.c_str());
            _attribMap[varName] = loc;
        }
        
        GLContext::CheckGLError("Shader::registerVars");
    }

    GLuint Shader::LoadProg(const std::string& name, GLuint vertShaderId, GLuint fragShaderId) {
        GLuint progId = glCreateProgram();
        if (progId == 0) {
            Log::Errorf("Shader::LoadProg: Failed to create shader program in '%s' shader", name.c_str());
            return 0;
        }

        glAttachShader(progId, vertShaderId);
        glAttachShader(progId, fragShaderId);
        glLinkProgram(progId);
        GLint linked = GL_FALSE;
        glGetProgramiv(progId, GL_LINK_STATUS, &linked);
        if (linked == GL_FALSE) {
            GLint infoLen = 0;
            glGetShaderiv(progId, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen > 0) {
                std::vector<char> infoBuf(infoLen);
                glGetProgramInfoLog(progId, infoLen, NULL, infoBuf.data());
                Log::Errorf("Shader::LoadProg: Failed to link shader program in '%s' shader \n Error: %s ", name.c_str(), infoBuf.data());
            }
            glDeleteProgram(progId);
            progId = 0;
        }

        GLContext::CheckGLError("Shader::LoadProg");

        return progId;
    }

    GLuint Shader::LoadShader(const std::string& name, const std::string& source, GLenum shaderType) {
        GLuint shaderId = glCreateShader(shaderType);
        if (shaderId == 0) {
            Log::Errorf("Shader::LoadShader: Failed to create shader type %i in '%s' shader", shaderType, name.c_str());
            return 0;
        }

        const char* sourceBuf = source.data();
        glShaderSource(shaderId, 1, &sourceBuf, NULL);

        glCompileShader(shaderId);
        GLint compiled = GL_FALSE;
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compiled);
        if (compiled == GL_FALSE) {
            GLint infoLen = 0;
            glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen > 0) {
                std::vector<char> infoBuf(infoLen);
                glGetShaderInfoLog(shaderId, infoLen, NULL, infoBuf.data());
                Log::Errorf("Shader::LoadShader: Failed to compile shader type %i in '%s' shader \n Error: %s ", shaderType, name.c_str(), infoBuf.data());
            }
            glDeleteShader(shaderId);
            shaderId = 0;
        }

        GLContext::CheckGLError("Shader::LoadShader");

        return shaderId;
    }

}
