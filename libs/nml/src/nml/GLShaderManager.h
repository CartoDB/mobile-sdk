/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NML_GLSHADERMANAGER_H_
#define _CARTO_NML_GLSHADERMANAGER_H_

#include "GLBase.h"

#include <string>
#include <map>
#include <set>

namespace carto { namespace nml {

    class GLShaderManager {
    public:
        GLuint createProgram(const std::string& vertexShader, const std::string& fragmentShader, const std::set<std::string>& defs);

        void resetPrograms();
        void deletePrograms();

    private:
        static std::string createShader(const std::string& shader, const std::set<std::string>& defs);
        
        std::map<std::pair<std::pair<std::string, std::string>, std::set<std::string>>, GLuint> _programMap;
    };
} }

#endif
