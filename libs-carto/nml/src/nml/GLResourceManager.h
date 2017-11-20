/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NML_GLRESOURCEMANAGER_H_
#define _CARTO_NML_GLRESOURCEMANAGER_H_

#include "GLBase.h"

#include <memory>
#include <string>
#include <map>
#include <set>
#include <vector>

namespace carto { namespace nml {
    class GLTexture;
    class GLSubmesh;

    class GLResourceManager final {
    public:
        GLResourceManager() = default;

        GLuint createProgram(const std::string& vertexShader, const std::string& fragmentShader, const std::set<std::string>& defs);

        GLuint allocateTexture(const std::shared_ptr<GLTexture>& texture);

        GLuint allocateVBO(const std::shared_ptr<GLSubmesh>& submesh);

        void deleteUnused();
        void deleteAll();
        void resetAll();

    private:
        static std::string createShader(const std::string& shader, const std::set<std::string>& defs);
        
        std::map<std::pair<std::pair<std::string, std::string>, std::set<std::string>>, GLuint> _programMap;
        std::multimap<std::weak_ptr<GLTexture>, GLuint, std::owner_less<std::weak_ptr<GLTexture>>> _textureMap;
        std::multimap<std::weak_ptr<GLSubmesh>, GLuint, std::owner_less<std::weak_ptr<GLSubmesh>>> _vboMap;
    };
} }

#endif
