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
        struct Source {
            Source(const std::string& name, const std::string& vertSource, const std::string& fragSource) :
                _name(name),
                _vertSource(vertSource),
                _fragSource(fragSource)
            {
            }
        
            std::string _name;
            std::string _vertSource;
            std::string _fragSource;
        };

        virtual ~Shader();

        GLuint getProgId() const;
        GLuint getUniformLoc(const std::string& varName) const;
        GLuint getAttribLoc(const std::string& varName) const;
        
    protected:
        friend GLResourceManager;
        
        Shader(const std::shared_ptr<GLResourceManager>& manager, const Source& source);

        virtual void create() const;
        virtual void destroy() const;

    private:
        void registerVars(GLuint progId) const;

        static GLuint LoadProg(const std::string& name, GLuint vertShaderId, GLuint fragShaderId);
        static GLuint LoadShader(const std::string& name, const std::string& source, GLenum shaderType);

        Source _source;
        
        mutable GLuint _progId;
        mutable GLuint _vertShaderId;
        mutable GLuint _fragShaderId;
        
        mutable std::unordered_map<std::string, GLuint> _uniformMap;
        mutable std::unordered_map<std::string, GLuint> _attribMap;
    };

}

#endif
