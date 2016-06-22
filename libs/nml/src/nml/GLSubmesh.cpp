#include "GLSubmesh.h"
#include "GLMesh.h"
#include "Package.h"

#include <cassert>

namespace carto { namespace nml {

    GLSubmesh::GLSubmesh(const Submesh& submesh) :
        _refCount(0),
        _glType(-1),
        _vertexCounts(),
        _materialId(),
        _positionBuffer(),
        _normalBuffer(),
        _uvBuffer(),
        _colorBuffer(),
        _vertexIdBuffer(),
        _glPositionVBOId(0),
        _glNormalVBOId(0),
        _glUVVBOId(0),
        _glColorVBOId(0)
    {
        // Translate submesh type
        _glType = convertType(submesh.type());
    
        // Copy basic attributes
        _vertexCounts.clear();
        for (int i = 0; i < submesh.vertex_counts_size(); i++) {
            _vertexCounts.push_back(submesh.vertex_counts(i));
        }
        _materialId = submesh.material_id();
    
        // Create vertex buffers
        convertToFloatBuffer(submesh.positions(), _positionBuffer);
        convertToFloatBuffer(submesh.normals(), _normalBuffer);
        convertToFloatBuffer(submesh.uvs(), _uvBuffer);
        _uvBuffer.resize(_positionBuffer.size() / 3 * 2, 0.5f);
        convertToByteBuffer(submesh.colors(), _colorBuffer);
        _colorBuffer.resize(_positionBuffer.size() / 3 * 4, 255);
    
        // Generate vertex id buffer
        _vertexIdBuffer.clear();
        for (int i = 0; i < submesh.vertex_ids_size(); i++) {
            int count = static_cast<int>(submesh.vertex_ids(i) >> 32);
            unsigned int id = static_cast<unsigned int>(submesh.vertex_ids(i) & (unsigned int) -1);
            _vertexIdBuffer.insert(_vertexIdBuffer.end(), count, id);
        }
    }
    
    GLSubmesh::GLSubmesh(const GLMesh& glMesh, const SubmeshOpList& submeshOpList) :
        _refCount(0),
        _glType(-1),
        _vertexCounts(),
        _materialId(),
        _positionBuffer(),
        _normalBuffer(),
        _uvBuffer(),
        _colorBuffer(),
        _vertexIdBuffer(),
        _glPositionVBOId(0),
        _glNormalVBOId(0),
        _glUVVBOId(0),
        _glColorVBOId(0)
    {
        _glType = convertType(submeshOpList.type());
        _materialId = submeshOpList.material_id();
    
        int vertexCount = 0;
        for (int i = 0; i < submeshOpList.submesh_ops_size(); i++) {
            vertexCount += submeshOpList.submesh_ops(i).count();
        }
        _vertexCounts.assign(1, vertexCount);
    
        _positionBuffer.reserve(vertexCount * 3);
        _normalBuffer.reserve(vertexCount * 3);
        _colorBuffer.reserve(vertexCount * 4);
        _vertexIdBuffer.reserve(vertexCount);
        _uvBuffer.reserve(vertexCount * 2);
        for (int i = 0; i < submeshOpList.submesh_ops_size(); i++) {
            const SubmeshOp& submeshOp = submeshOpList.submesh_ops(i);
            const GLSubmesh& src = *glMesh.getSubmeshList()[submeshOp.submesh_idx()];
    
            int start = submeshOp.offset(), end = submeshOp.offset() + submeshOp.count();
            _positionBuffer.insert(_positionBuffer.end(), src._positionBuffer.begin() + start * 3, src._positionBuffer.begin() + end * 3);

            if (!src._normalBuffer.empty()) {
                _normalBuffer.insert(_normalBuffer.end(), src._normalBuffer.begin() + start * 3, src._normalBuffer.begin() + end * 3);
            }

            if (!src._colorBuffer.empty()) {
                _colorBuffer.insert(_colorBuffer.end(), src._colorBuffer.begin() + start * 4, src._colorBuffer.begin() + end * 4);
            }

            if (!src._vertexIdBuffer.empty()) {
                _vertexIdBuffer.insert(_vertexIdBuffer.end(), src._vertexIdBuffer.begin() + start, src._vertexIdBuffer.begin() + end);
            }

            if (!src._uvBuffer.empty()) {
                for (int idx = start; idx < end; idx++) {
                    float u = src._uvBuffer[idx * 2 + 0] * submeshOp.tex_u_scale() + submeshOp.tex_u_trans();
                    float v = src._uvBuffer[idx * 2 + 1] * submeshOp.tex_v_scale() + submeshOp.tex_v_trans();
                    _uvBuffer.push_back(u);
                    _uvBuffer.push_back(v);
                }
            }
        }
    }
    
    void GLSubmesh::create() {
        if (_refCount++ > 0) {
            return;
        }
    
        uploadSubmesh();
    }
    
    void GLSubmesh::dispose() {
        if (--_refCount > 0) {
            return;
        }
    
        if (_glPositionVBOId != 0) {
            glDeleteBuffers(1, &_glPositionVBOId);
        }
        if (_glNormalVBOId != 0) {
            glDeleteBuffers(1, &_glNormalVBOId);
        }
        if (_glUVVBOId != 0) {
            glDeleteBuffers(1, &_glUVVBOId);
        }
        if (_glColorVBOId != 0) {
            glDeleteBuffers(1, &_glColorVBOId);
        }
        
        _glPositionVBOId = 0;
        _glNormalVBOId = 0;
        _glUVVBOId = 0;
        _glColorVBOId = 0;
    }
    
    void GLSubmesh::draw(const RenderState& renderState) {
        if (_vertexCounts.empty()) {
            return;
        }
    
        if (_glPositionVBOId == 0) {
            uploadSubmesh();
        }

        // Enable vertex buffers
        GLint programId = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &programId);

        if (!_positionBuffer.empty()) {
            GLint positionLocation = glGetAttribLocation(programId, "aVertexPosition");
            glBindBuffer(GL_ARRAY_BUFFER, _glPositionVBOId);
            glEnableVertexAttribArray(positionLocation);
            glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
        } else {
            return;
        }

        GLint normalLocation = glGetAttribLocation(programId, "aVertexNormal");
        if (normalLocation != -1) {
            if (!_normalBuffer.empty()) {
                glBindBuffer(GL_ARRAY_BUFFER, _glNormalVBOId);
                glEnableVertexAttribArray(normalLocation);
                glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
            } else {
                glDisableVertexAttribArray(normalLocation);
                glVertexAttrib3f(normalLocation, 0, 0, 0);
            }
        }

        GLint uvLocation = glGetAttribLocation(programId, "aVertexUV");
        if (uvLocation != -1) {
            if (!_uvBuffer.empty()) {
                glBindBuffer(GL_ARRAY_BUFFER, _glUVVBOId);
                glEnableVertexAttribArray(uvLocation);
                glVertexAttribPointer(uvLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
            } else {
                glDisableVertexAttribArray(uvLocation);
                glVertexAttrib2f(uvLocation, 0, 0);
            }
        }

        GLint colorLocation = glGetAttribLocation(programId, "aVertexColor");
        if (colorLocation != -1) {
            if (!_colorBuffer.empty()) {
                glBindBuffer(GL_ARRAY_BUFFER, _glColorVBOId);
                glEnableVertexAttribArray(colorLocation);
                glVertexAttribPointer(colorLocation, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, 0);
            } else {
                glDisableVertexAttribArray(colorLocation);
                glVertexAttrib4f(colorLocation, 1, 1, 1, 1);
            }
        }
    
        // Draw primitives
        int idx = 0;
        for (std::size_t i = 0; i < _vertexCounts.size(); i++) {
            int count = _vertexCounts[i];
            glDrawArrays(_glType, idx, count);
            idx += count;
        }
    
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    void GLSubmesh::calculateRayIntersections(const cglib::ray3<double>& ray, std::vector<RayIntersection>& intersections) const {
        if (!(_glType == GL_TRIANGLES || _glType == GL_TRIANGLE_FAN || _glType == GL_TRIANGLE_STRIP)) {
            return;
        }
    
        // Do ray-triangle test for each individual triangles
        int idx = 0;
        for (std::size_t i = 0; i < _vertexCounts.size(); i++) {
            int count = _vertexCounts[i];
            for (int k = 2; k < count; ) {
                std::size_t i0, i1, i2;
                if (_glType == GL_TRIANGLE_FAN) {
                    i0 = idx;
                    i1 = idx + k - 1;
                    i2 = idx + k;
                    k++;
                } else if (_glType == GL_TRIANGLE_STRIP) {
                    i0 = (k & 1) == 0 ? idx + k - 2 : idx + k - 1;
                    i1 = (k & 1) == 0 ? idx + k - 1 : idx + k - 2;
                    i2 = idx + k;
                    k++;
                } else {
                    i0 = idx + k - 2;
                    i1 = idx + k - 1;
                    i2 = idx + k;
                    k += 3;
                }

                cglib::vec3<double> points[] = {
                    cglib::vec3<double>(_positionBuffer[i0 * 3 + 0], _positionBuffer[i0 * 3 + 1], _positionBuffer[i0 * 3 + 2]),
                    cglib::vec3<double>(_positionBuffer[i1 * 3 + 0], _positionBuffer[i1 * 3 + 1], _positionBuffer[i1 * 3 + 2]),
                    cglib::vec3<double>(_positionBuffer[i2 * 3 + 0], _positionBuffer[i2 * 3 + 1], _positionBuffer[i2 * 3 + 2])
                };
    
                double t = 0;
                if (cglib::intersect_triangle(points[0], points[1], points[2], ray, &t)) {
                    cglib::vec3<double> pos = ray(t);
                    cglib::vec3<double> normal = cglib::unit(cglib::vector_product(points[1] - points[0], points[2] - points[0]));
                    unsigned int id = 0;
                    if (i0 < _vertexIdBuffer.size()) {
                        id = _vertexIdBuffer[i0];
                    }
                    intersections.push_back(RayIntersection(id, pos, normal));
                }
            }
            idx += _vertexCounts[i];
        }
    }
    
    const std::string& GLSubmesh::getMaterialId() const {
        return _materialId;
    }
    
    int GLSubmesh::getDrawCallCount() const {
        return static_cast<int>(_vertexCounts.size());
    }
    
    int GLSubmesh::getTotalGeometrySize() const {
        std::size_t size = 0;
        size += _positionBuffer.size() * sizeof(float);
        size += _normalBuffer.size() * sizeof(float);
        size += _uvBuffer.size() * sizeof(float);
        size += _colorBuffer.size() * sizeof(unsigned char);
        size += _vertexIdBuffer.size() * sizeof(unsigned char);
        return static_cast<int>(size);
    }
    
    void GLSubmesh::uploadSubmesh() {
        if (!_positionBuffer.empty()) {
            glGenBuffers(1, &_glPositionVBOId);
            glBindBuffer(GL_ARRAY_BUFFER, _glPositionVBOId);
            glBufferData(GL_ARRAY_BUFFER, _positionBuffer.size() * sizeof(float), _positionBuffer.data(), GL_STATIC_DRAW);
        }

        if (!_normalBuffer.empty()) {
            glGenBuffers(1, &_glNormalVBOId);
            glBindBuffer(GL_ARRAY_BUFFER, _glNormalVBOId);
            glBufferData(GL_ARRAY_BUFFER, _normalBuffer.size() * sizeof(float), _normalBuffer.data(), GL_STATIC_DRAW);
        }
        
        if (!_uvBuffer.empty()) {
            glGenBuffers(1, &_glUVVBOId);
            glBindBuffer(GL_ARRAY_BUFFER, _glUVVBOId);
            glBufferData(GL_ARRAY_BUFFER, _uvBuffer.size() * sizeof(float), _uvBuffer.data(), GL_STATIC_DRAW);
        }
        
        if (!_colorBuffer.empty()) {
            glGenBuffers(1, &_glColorVBOId);
            glBindBuffer(GL_ARRAY_BUFFER, _glColorVBOId);
            glBufferData(GL_ARRAY_BUFFER, _colorBuffer.size() * sizeof(unsigned char), _colorBuffer.data(), GL_STATIC_DRAW);
        }
    
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    GLint GLSubmesh::convertType(int type) {
        GLint glType = -1;
        switch (type) {
        case Submesh::POINTS:
            glType = GL_POINTS;
            break;
        case Submesh::LINES:
            glType = GL_LINES;
            break;
        case Submesh::LINE_STRIPS:
            glType = GL_LINE_STRIP;
            break;
        case Submesh::TRIANGLES:
            glType = GL_TRIANGLES;
            break;
        case Submesh::TRIANGLE_STRIPS:
            glType = GL_TRIANGLE_STRIP;
            break;
        case Submesh::TRIANGLE_FANS:
            glType = GL_TRIANGLE_FAN;
            break;
        default:
            assert(false);
        }
        return glType;
    }
    
    void GLSubmesh::convertToFloatBuffer(const std::string& str, std::vector<float>& buf) {
        if (str.empty()) {
            buf.clear();
            return;
        }
    
        buf.assign(reinterpret_cast<const float*>(str.data()), reinterpret_cast<const float*>(str.data()) + str.size() / sizeof(float));

        // Detect if the host is big-endian - swap bytes in that case
        int num = 1;
        if (*(char*)&num != 1) {
            unsigned char* bytePtr = reinterpret_cast<unsigned char*>(buf.data());
            for (std::size_t i = 0; i < buf.size(); i++) {
                for (std::size_t j = 0; j < sizeof(float) - j - 1; j++) {
                    std::swap(bytePtr[i * sizeof(float) + j], bytePtr[i * sizeof(float) + sizeof(float) - j - 1]);
                }
            }
        }
    }
    
    void GLSubmesh::convertToByteBuffer(const std::string& str, std::vector<unsigned char>& buf) {
        if (str.empty()) {
            buf.clear();
            return;
        }
    
        buf.assign(reinterpret_cast<const unsigned char*>(str.data()), reinterpret_cast<const unsigned char *>(str.data()) + str.size() / sizeof(unsigned char));
    }
    
} }
