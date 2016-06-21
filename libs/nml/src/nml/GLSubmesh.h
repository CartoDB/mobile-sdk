/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_NML_GLSUBMESH_H_
#define _CARTO_NML_GLSUBMESH_H_

#include "GLBase.h"

#include <memory>
#include <string>
#include <vector>

namespace carto { namespace nml {
    class Submesh;
    class SubmeshOpList;
    class GLMesh;

    class GLSubmesh {
    public:
        GLSubmesh(const Submesh& submesh);
        GLSubmesh(const GLMesh& glMesh, const SubmeshOpList& submeshOpList);

        void create();
        void dispose();

        void draw(const RenderState& renderState);

        void calculateRayIntersections(const Ray& ray, std::vector<RayIntersection>& intersections) const;

        const std::string& getMaterialId() const;

        int getDrawCallCount() const;
        int getTotalGeometrySize() const;

    private:
        void uploadSubmesh();
        
        static bool findRayTriangleIntersection(const cglib::vec3<float>* points, const Ray& ray, cglib::vec3<float>& p, cglib::vec3<float>& n);

        static GLint convertType(int type);
        static void convertToFloatBuffer(const std::string& str, std::vector<float>& buf);
        static void convertToByteBuffer(const std::string& str, std::vector<unsigned char>& buf);

        int _refCount;
        GLint _glType;
        std::vector<int> _vertexCounts;
        std::string _materialId;

        std::vector<float> _positionBuffer;
        std::vector<float> _normalBuffer;
        std::vector<float> _uvBuffer;
        std::vector<unsigned char> _colorBuffer;
        std::vector<unsigned int> _vertexIdBuffer;

        GLuint _glPositionVBOId;
        GLuint _glNormalVBOId;
        GLuint _glUVVBOId;
        GLuint _glColorVBOId;
    };
} }

#endif
