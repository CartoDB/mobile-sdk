/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NML_BASETYPES_H_
#define _CARTO_NML_BASETYPES_H_

#ifdef __APPLE__
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#else
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#include <cglib/bbox.h>
#include <cglib/frustum3.h>
#include <cglib/mat.h>
#include <cglib/vec.h>

namespace carto { namespace nmlgl {

    struct Ray {
        cglib::vec3<double> origin;
        cglib::vec3<double> dir;
        
        Ray(const cglib::vec3<double>& origin, const cglib::vec3<double>& dir) : origin(origin), dir(dir) { }
    };
    
    struct RayIntersection {
        unsigned int vertexId;
        cglib::vec3<double> pos;
        cglib::vec3<double> normal;
    
        RayIntersection(unsigned int vertexId, const cglib::vec3<double>& pos, const cglib::vec3<double>& normal) : vertexId(vertexId), pos(pos), normal(normal) { }
    };

    struct RenderState {
        cglib::mat4x4<float> projMatrix;
        cglib::mat4x4<float> mvMatrix;
        cglib::mat4x4<float> invTransMVMatrix;
        cglib::vec4<float> ambientLightColor;
        cglib::vec4<float> mainLightColor;
        cglib::vec3<float> mainLightDir;

        RenderState(const cglib::mat4x4<float>& projMatrix, const cglib::mat4x4<float>& mvMatrix, const cglib::vec4<float>& ambientLightColor, const cglib::vec4<float>& mainLightColor, const cglib::vec3<float>& mainLightDir) : projMatrix(projMatrix), mvMatrix(mvMatrix), invTransMVMatrix(cglib::transpose(cglib::inverse(mvMatrix))), ambientLightColor(ambientLightColor), mainLightColor(mainLightColor), mainLightDir(mainLightDir) { }
    };
   
} }

#endif
