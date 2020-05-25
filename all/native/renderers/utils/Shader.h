/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_SHADER_H_
#define _CARTO_SHADER_H_

#include "renderers/utils/GLResource.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace carto {

    class Shader : public GLResource {
    public:
        virtual ~Shader();

        GLuint getProgId() const;
        GLuint getUniformLoc(const std::string& varName) const;
        GLuint getAttribLoc(const std::string& varName) const;
        
    protected:
        friend GLResourceManager;
        
        Shader(const std::weak_ptr<GLResourceManager>& manager, const std::string& name, const std::string& vertSource, const std::string& fragSource);

        virtual void create();
        virtual void destroy();

    private:
        static GLuint LoadProg(const std::string& name, GLuint vertShaderId, GLuint fragShaderId);
        static GLuint LoadShader(const std::string& name, const std::string& source, GLenum shaderType);

        const std::string _name;
        const std::string _vertSource;
        const std::string _fragSource;
        
        GLuint _progId;
        GLuint _vertShaderId;
        GLuint _fragShaderId;
        
        std::unordered_map<std::string, GLuint> _uniformMap;
        std::unordered_map<std::string, GLuint> _attribMap;
    };

}

#endif
