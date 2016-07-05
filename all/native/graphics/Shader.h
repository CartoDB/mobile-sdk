/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_SHADER_H_
#define _CARTO_SHADER_H_

#include "graphics/shaders/ShaderSource.h"
#include "graphics/utils/GLContext.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace carto {
    class ShaderManager;

    class Shader {
    public:
        virtual ~Shader();

        GLuint getProgId() const;
        GLuint getUniformLoc(const std::string& varName) const;
        GLuint getAttribLoc(const std::string& varName) const;
        
    protected:
        friend class ShaderManager;
        
        Shader(const std::shared_ptr<ShaderManager>& shaderManager, const ShaderSource& source);

        void load() const;
        void unload() const;

    private:
        void registerVars(GLuint progId) const;

        GLuint loadProg(GLuint vertShaderId, GLuint fragShaderId) const;
        GLuint loadShader(const std::string& source, GLenum shaderType) const;

        ShaderSource _shaderSource;
        
        mutable GLuint _progId;
        mutable GLuint _vertShaderId;
        mutable GLuint _fragShaderId;
        
        mutable std::unordered_map<std::string, GLuint> _uniformMap;
        mutable std::unordered_map<std::string, GLuint> _attribMap;

        std::shared_ptr<ShaderManager> _shaderManager;
    };

}

#endif
