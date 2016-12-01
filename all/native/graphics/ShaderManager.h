/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_SHADERMANAGER_H_
#define _CARTO_SHADERMANAGER_H_

#include "graphics/Shader.h"

#include <memory>
#include <mutex>
#include <thread>
#include <string>
#include <unordered_map>
#include <vector>

namespace carto {
    class ShaderSource;
        
    class ShaderManager : public std::enable_shared_from_this<ShaderManager> {
    public:
        ShaderManager();
        virtual ~ShaderManager();
    
        std::thread::id getGLThreadId() const;
        void setGLThreadId(std::thread::id id);

        std::shared_ptr<Shader> createShader(const ShaderSource& source);

        void processShaders();
    
    private:
        void deleteShader(Shader* texture);

        std::thread::id _glThreadId;
        std::unordered_map<const ShaderSource*, std::weak_ptr<Shader> > _shaderMap;
        std::vector<std::weak_ptr<Shader> > _createQueue;
        std::vector<GLuint> _deleteProgIdQueue;
        std::vector<GLuint> _deleteShaderIdQueue;
        mutable std::mutex _mutex;
    };
    
}

#endif
