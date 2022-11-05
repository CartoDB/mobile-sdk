#include "LineDrawData.h"
#include "core/MapPos.h"
#include "geometry/LineGeometry.h"
#include "graphics/Bitmap.h"
#include "projections/Projection.h"
#include "projections/ProjectionSurface.h"
#include "renderers/utils/GLContext.h"
#include "styles/LineStyle.h"
#include "vectorelements/Line.h"
#include "vectorelements/Polygon.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <cmath>
#include <unordered_map>

namespace carto {

    LineDrawData::LineDrawData(const LineGeometry& geometry, const LineStyle& style, const Projection& projection, const std::shared_ptr<ProjectionSurface>& projectionSurface) :
        VectorElementDrawData(style.getColor(), projectionSurface),
        _bitmap(style.getBitmap()),
        _normalScale(style.getWidth() / 2),
        _clickScale(style.getClickWidth() == -1 ? std::max(1.0f, 1 + (IDEAL_CLICK_WIDTH - style.getWidth()) * CLICK_WIDTH_COEF / style.getWidth()) : style.getClickWidth()),
        _poses(),
        _coords(),
        _normals(),
        _texCoords(),
        _indices()
    {
        init(geometry.getPoses(), projection, style);
    }
    
    LineDrawData::LineDrawData(const std::vector<MapPos>& poses, const LineStyle& style, const Projection& projection, const std::shared_ptr<ProjectionSurface>& projectionSurface) :
        VectorElementDrawData(style.getColor(), projectionSurface),
        _bitmap(style.getBitmap()),
        _normalScale(style.getWidth() / 2),
        _clickScale(std::max(1.0f, 1 + (IDEAL_CLICK_WIDTH - style.getWidth()) * CLICK_WIDTH_COEF / style.getWidth())),
        _poses(),
        _coords(),
        _normals(),
        _texCoords(),
        _indices()
    {
        init(poses, projection, style);
    }
        
    LineDrawData::~LineDrawData() {
    }
    
    const std::shared_ptr<Bitmap> LineDrawData::getBitmap() const {
        return _bitmap;
    }

    float LineDrawData::getNormalScale() const {
        return _normalScale;
    }
    
    float LineDrawData::getClickScale() const {
        return _clickScale;
    }
    
    const std::vector<std::vector<cglib::vec3<double>*> >& LineDrawData::getCoords() const {
        return _coords;
    }
    
    const std::vector<std::vector<cglib::vec4<float> > >& LineDrawData::getNormals() const {
        return _normals;
    }
    
    const std::vector<std::vector<cglib::vec2<float> > >& LineDrawData::getTexCoords() const {
        return _texCoords;
    }
    
    const std::vector<std::vector<unsigned int> >& LineDrawData::getIndices() const {
        return _indices;
    }
    
    void LineDrawData::offsetHorizontally(double offset) {
        for (cglib::vec3<double>& pos : _poses) {
            pos(0) += offset;
        }
        setIsOffset(true);
    }
    
    void LineDrawData::init(const std::vector<MapPos>& poses, const Projection& projection, const LineStyle& style) {
        // Calculate real coordinates and tesselate the line
        std::vector<cglib::vec3<float> > posNormals;
        _poses.reserve(poses.size());
        posNormals.reserve(poses.size());
        std::vector<MapPos> internalPoses;
        for (std::size_t i = 1; i < poses.size(); i++) {
            internalPoses.clear();
            _projectionSurface->tesselateSegment(projection.toInternal(poses[i - 1]), projection.toInternal(poses[i]), internalPoses);
            for (const MapPos& internalPos : internalPoses) {
                cglib::vec3<double> pos = _projectionSurface->calculatePosition(internalPos);
                if (_poses.empty() || pos != _poses.back()) {
                    _poses.push_back(pos);
                    posNormals.push_back(cglib::vec3<float>::convert(_projectionSurface->calculateNormal(internalPos)));
                }
            }
        }

        if (_poses.size() < 2) {
            _coords.clear();
            _normals.clear();
            _texCoords.clear();
            _indices.clear();
            return;
        }
    
        // Detect looped line
        bool loopedLine = (_poses.front() == _poses.back()) && (_poses.size() > 2);

        // Detect if we must tesselate line joins
        bool tesselateLineJoin = (style.getLineJoinType() == LineJoinType::LINE_JOIN_TYPE_BEVEL || style.getLineJoinType() == LineJoinType::LINE_JOIN_TYPE_ROUND);
    
        // Calculate angles between lines and buffers sizes
        std::size_t coordCount = (_poses.size() - 1) * 4;
        std::size_t indexCount = (_poses.size() - 1) * 6;
        std::vector<float> deltaAngles(_poses.size() - 1);
        cglib::vec3<float> prevLineVec(0, 0, 0);
        if (tesselateLineJoin) {
            for (std::size_t i = 0; i < _poses.size(); i++) {
                if (!loopedLine && i + 1 >= _poses.size()) {
                    break;
                }
    
                const cglib::vec3<double>& pos = _poses[i];
                const cglib::vec3<double>& nextPos = (i + 1 < _poses.size()) ? _poses[i + 1] : _poses[1];
                if (nextPos == pos) {
                    continue;
                }
                cglib::vec3<float> nextLineVec = cglib::vec3<float>::convert(cglib::unit(nextPos - pos));

                double dot = cglib::dot_product(prevLineVec, nextLineVec);
                if (cglib::norm(prevLineVec) > 0) {
                    float deltaAngle = static_cast<float>(std::acos(std::max(-1.0, std::min(1.0, dot))) * Const::RAD_TO_DEG);
                    if (cglib::dot_product(posNormals[i], cglib::vector_product(prevLineVec, nextLineVec)) < 0) {
                        deltaAngle = -deltaAngle;
                    }
                    deltaAngles[i - 1] = deltaAngle;
    
                    int segments = 0;
                    if (style.getLineJoinType() == LineJoinType::LINE_JOIN_TYPE_BEVEL) {
                        segments = deltaAngle != 0 ? 1 : 0;
                    } else { //style.getLineJoinType() == LineJoinType::ROUND
                        segments = 1 + static_cast<int>(std::ceil(std::abs(deltaAngle) * style.getWidth() * LINE_JOIN_TESSELATION_FACTOR));
                    }
    
                    coordCount += segments;
                    indexCount += segments * 3;
                }
    
                prevLineVec = nextLineVec;
            }
        }
    
        // Endpoints need a bigger buffer
        if (!loopedLine && style.getLineEndType() == LineEndType::LINE_END_TYPE_ROUND) {
            int segments = static_cast<int>(180 * style.getWidth() * LINE_ENDPOINT_TESSELATION_FACTOR);
            coordCount += segments * 2;
            indexCount += segments * 3 * 2;
        }
    
        // Texture bounds
        float texCoordX = 1.0f;
        float texCoordY = 0.0f;
        float texCoordYScale = _bitmap->getWidth() / (style.getStretchFactor() * _bitmap->getHeight() * style.getWidth());
        bool useTexCoordY = _bitmap->getHeight() > 1;

        // Instead of calculating actual vertex positions calculate vertex origins and normals
        // Actual vertex positions are view dependent and will be calculated in the renderer
        std::vector<cglib::vec3<double>*> coords;
        std::vector<cglib::vec4<float> > normals;
        std::vector<cglib::vec2<float> > texCoords;
        std::vector<unsigned int> indices;
        coords.reserve(coordCount);
        normals.reserve(coordCount);
        texCoords.reserve(coordCount);
        indices.reserve(indexCount);

        // Calculate initial state for line string
        cglib::vec3<float> nextLine = cglib::vec3<float>::convert(_poses[1] - _poses[0]);
        cglib::vec3<float> nextPerpVec = cglib::unit(cglib::vector_product(posNormals[1], nextLine));

        cglib::vec3<float> nextNormalVec = nextPerpVec;
        bool resetNormalVec = true;
        if (style.getLineJoinType() == LineJoinType::LINE_JOIN_TYPE_MITER) {
            if (loopedLine) {
                cglib::vec3<float> prevLine = cglib::vec3<float>::convert(_poses[0] - _poses[_poses.size() - 2]);
                cglib::vec3<float> prevPerpVec = cglib::unit(cglib::vector_product(posNormals[0], prevLine));

                float dot = cglib::dot_product(prevPerpVec, nextPerpVec);
                if (dot >= LINE_JOIN_MIN_MITER_DOT) {
                    nextNormalVec = cglib::unit(prevPerpVec + nextPerpVec) * (1 / std::sqrt((1 + dot) / 2));
                    resetNormalVec = false;
                }
            }
        }

        // Loop over line segments
        cglib::vec3<float> firstPerpVec;
        cglib::vec3<float> lastPerpVec;
        unsigned int vertexIndex = 0;
        for (std::size_t i = 1; i < _poses.size(); i++) {
            std::size_t i1 = i + 1 < _poses.size() ? i + 1 : 1;
            
            cglib::vec3<double>& pos = _poses[i];
            cglib::vec3<double>& prevPos = _poses[i - 1];
            cglib::vec3<double>& nextPos = _poses[i1];

            // Calculate line body
            cglib::vec3<float> prevLine = cglib::vec3<float>::convert(pos - prevPos);
            cglib::vec3<float> prevPerpVec = cglib::unit(cglib::vector_product(posNormals[i], prevLine));

            // Trick to reuse already generated vertex data (only for mitered lines)
            if (!resetNormalVec && vertexIndex >= 2) {
                vertexIndex -= 2;
                coords.pop_back();
                coords.pop_back();
                texCoords.pop_back();
                texCoords.pop_back();
                normals.pop_back();
                normals.pop_back();
            }

            cglib::vec3<float> prevNormalVec = (resetNormalVec ? prevPerpVec : nextNormalVec);
            nextNormalVec = prevPerpVec;
            resetNormalVec = true;

            if (style.getLineJoinType() == LineJoinType::LINE_JOIN_TYPE_MITER) {
                if (i + 1 < _poses.size() || loopedLine) {
                    cglib::vec3<float> nextLine = cglib::vec3<float>::convert(nextPos - pos);
                    cglib::vec3<float> nextPerpVec = cglib::unit(cglib::vector_product(posNormals[i1], nextLine));

                    float dot = cglib::dot_product(prevPerpVec, nextPerpVec);
                    if (dot >= LINE_JOIN_MIN_MITER_DOT) {
                        nextNormalVec = cglib::unit(prevPerpVec + nextPerpVec) * (1 / std::sqrt((1 + dot) / 2));
                        resetNormalVec = false;
                    }
                }
            }

            if (i == 1) {
                firstPerpVec = prevPerpVec;
            }
            if (i == _poses.size() - 1) {
                lastPerpVec = prevPerpVec;
            }

            // Add line vertices, normals and indices
            coords.push_back(&prevPos);
            coords.push_back(&prevPos);
            coords.push_back(&pos);
            coords.push_back(&pos);
            
            if (useTexCoordY) {
                float texCoordYOffset = cglib::length(prevLine) * texCoordYScale;
                texCoords.push_back(cglib::vec2<float>(0, texCoordY));
                texCoords.push_back(cglib::vec2<float>(texCoordX, texCoordY));
                texCoords.push_back(cglib::vec2<float>(0, texCoordY + texCoordYOffset));
                texCoords.push_back(cglib::vec2<float>(texCoordX, texCoordY + texCoordYOffset));
                texCoordY += texCoordYOffset;
            } else {
                texCoords.push_back(cglib::vec2<float>(0, 0.5f));
                texCoords.push_back(cglib::vec2<float>(texCoordX, 0.5f));
                texCoords.push_back(cglib::vec2<float>(0, 0.5f));
                texCoords.push_back(cglib::vec2<float>(texCoordX, 0.5f));
            }

            normals.push_back(cglib::expand(prevNormalVec, 1.0f));
            normals.push_back(cglib::expand(prevNormalVec, -1.0f));
            normals.push_back(cglib::expand(nextNormalVec, 1.0f));
            normals.push_back(cglib::expand(nextNormalVec, -1.0f));

            indices.push_back(vertexIndex + 0);
            indices.push_back(vertexIndex + 1);
            indices.push_back(vertexIndex + 2);
            indices.push_back(vertexIndex + 1);
            indices.push_back(vertexIndex + 3);
            indices.push_back(vertexIndex + 2);
            
            vertexIndex += 4;
            
            // Calculate line joins, if necessary
            if (tesselateLineJoin && (i + 1 <  _poses.size() || loopedLine)) {
                float deltaAngle = deltaAngles[i - 1];
                
                int segments = 0;
                if (style.getLineJoinType() == LineJoinType::LINE_JOIN_TYPE_BEVEL) {
                    segments = deltaAngle != 0 ? 1 : 0;
                } else { // style.getLineJoinType() == LineJoinType::ROUND
                    segments = 1 + static_cast<int>(std::ceil(std::abs(deltaAngle) * style.getWidth() * LINE_JOIN_TESSELATION_FACTOR));
                }
                if (segments > 0) {
                    float segmentDeltaAngle = deltaAngle / segments;
                    cglib::mat3x3<float> rot3DMat = cglib::rotate3_matrix(posNormals[i], static_cast<float>(segmentDeltaAngle * Const::DEG_TO_RAD));
                    bool leftTurn = (deltaAngle <= 0);
                    cglib::vec3<float> rotVec = prevNormalVec;
                    
                    // Add the t vertex
                    coords.push_back(&pos);
                    normals.push_back(cglib::expand(rotVec, 0.0f));
                    texCoords.push_back(cglib::vec2<float>(0.5f, texCoordY));
                    
                    // Add vertices and normals, do not create double vertices anywhere
                    for (int j = 0; j < segments - 1; j++) {
                        rotVec = cglib::transform(rotVec, rot3DMat);
                        coords.push_back(&pos);
                        normals.push_back(cglib::expand(rotVec, leftTurn ? 1.0f : -1.0f));
                        texCoords.push_back(cglib::vec2<float>(leftTurn ? 0.0f : 1.0f, texCoordY));
                    }
                    
                    // Add indices, make use of existing and future line's vertices
                    if (deltaAngle <= 0) {
                        for (int j = 0; j < segments; j++) {
                            indices.push_back(vertexIndex);
                            if (j == segments - 1) {
                                indices.push_back((i == _poses.size() - 1) ? 0 : (vertexIndex + j + 1));
                            } else {
                                indices.push_back(vertexIndex + j + 1);
                            }
                            indices.push_back((j == 0) ? vertexIndex - 2 : (vertexIndex + j));
                        }
                    } else {
                        for (int j = 0; j < segments; j++) {
                            indices.push_back(vertexIndex);
                            indices.push_back((j == 0) ? vertexIndex - 1 : (vertexIndex + j));
                            if (j == segments - 1) {
                                indices.push_back((i == _poses.size() - 1) ? 1 : (vertexIndex + j + 2));
                            } else {
                                indices.push_back(vertexIndex + j + 1);
                            }
                        }
                    }
                    
                    vertexIndex += segments;
                }
            }
        }
        
        // Calculate line end points
        if (!loopedLine && style.getLineEndType() != LineEndType::LINE_END_TYPE_NONE) {
            int segments = 0;
            float segmentDeltaAngle = 0.0f;
            if (style.getLineEndType() == LineEndType::LINE_END_TYPE_SQUARE) {
                segments = 3;
                segmentDeltaAngle = 45.0f;
            } else { // style.getLineEndType() == LineEndType::SQUARE
                segments = static_cast<int>(180 * style.getWidth() * LINE_ENDPOINT_TESSELATION_FACTOR);
                segmentDeltaAngle = 180.0f / (segments - 1);
            }
            if (segments > 1) {
                cglib::mat2x2<float> rot2DMat = cglib::rotate2_matrix(static_cast<float>(segmentDeltaAngle * Const::DEG_TO_RAD));
                
                // Add the t vertex
                coords.push_back(&_poses[_poses.size() - 1]);
                normals.push_back(cglib::expand(lastPerpVec, 0.0f));
                texCoords.push_back(cglib::vec2<float>(0.5f, texCoordY));
                
                if (style.getLineEndType() == LineEndType::LINE_END_TYPE_ROUND) {
                    // Last end point, lastLine contains the last valid line segment
                    cglib::mat3x3<float> rot3DMat = cglib::rotate3_matrix(posNormals[_poses.size() - 1], static_cast<float>(segmentDeltaAngle * Const::DEG_TO_RAD));
                    cglib::vec3<float> rotVec = lastPerpVec;
                    cglib::vec2<float> uvRotVec(-1, 0);
                
                    // Vertices
                    for (int i = 0; i < segments - 1; i++) {
                        rotVec = cglib::transform(rotVec, rot3DMat);
                        uvRotVec = cglib::transform(uvRotVec, rot2DMat);
                        coords.push_back(&_poses[_poses.size() - 1]);
                        normals.push_back(cglib::expand(rotVec, -1.0f));
                        texCoords.push_back(cglib::vec2<float>(uvRotVec(0) * 0.5f + 0.5f, texCoordY));
                    }
                } else {
                    // Vertices
                    for (int s = -1; s <= 1; s += 2) {
                        cglib::mat3x3<float> rot3DMat = cglib::rotate3_matrix(posNormals[_poses.size() - 1], static_cast<float>(-s * segmentDeltaAngle * Const::DEG_TO_RAD));
                        cglib::vec3<float> normalVec = cglib::transform(lastPerpVec, rot3DMat) * std::sqrt(2.0f);
                        coords.push_back(&_poses[_poses.size() - 1]);
                        normals.push_back(cglib::expand(normalVec, static_cast<float>(s)));
                        texCoords.push_back(cglib::vec2<float>(s * 0.5f + 0.5f, texCoordY));
                    }
                }
                
                // Indices
                for (int j = 0; j < segments; j++) {
                    indices.push_back(vertexIndex);
                    indices.push_back((j == 0) ? vertexIndex - 1 : (vertexIndex + j));
                    indices.push_back((j == segments - 1) ? vertexIndex - 2 : (vertexIndex + j + 1));
                }
                vertexIndex += segments;
                
                // Add the t vertex for the other end point
                coords.push_back(&_poses[0]);
                normals.push_back(cglib::expand(firstPerpVec, 0.0f));
                texCoords.push_back(cglib::vec2<float>(0.5f, 0));
                
                if (style.getLineEndType() == LineEndType::LINE_END_TYPE_ROUND) {
                    // First end point, firstLine contains the first valid line segment
                    cglib::mat3x3<float> rot3DMat = cglib::rotate3_matrix(posNormals[0], static_cast<float>(segmentDeltaAngle * Const::DEG_TO_RAD));
                    cglib::vec3<float> rotVec = firstPerpVec;
                    cglib::vec2<float> uvRotVec(1, 0);
                
                    // Vertices
                    for (int i = 0; i < segments - 1; i++) {
                        rotVec = cglib::transform(rotVec, rot3DMat);
                        uvRotVec = cglib::transform(uvRotVec, rot2DMat);
                        coords.push_back(&_poses[0]);
                        normals.push_back(cglib::expand(rotVec, 1.0f));
                        texCoords.push_back(cglib::vec2<float>(uvRotVec(0) * 0.5f + 0.5f, 0));
                    }
                } else {
                    // Vertices
                    for (int s = 1; s >= -1; s -= 2) {
                        cglib::mat3x3<float> rot3DMat = cglib::rotate3_matrix(posNormals[0], static_cast<float>(s * segmentDeltaAngle * Const::DEG_TO_RAD));
                        cglib::vec3<float> normalVec = cglib::transform(firstPerpVec, rot3DMat) * std::sqrt(2.0f);
                        coords.push_back(&_poses[0]);
                        normals.push_back(cglib::expand(normalVec, static_cast<float>(s)));
                        texCoords.push_back(cglib::vec2<float>(s * 0.5f + 0.5f, 0));
                    }
                }
                
                // Indices
                for (int j = 0; j < segments; j++) {
                    indices.push_back(vertexIndex);
                    indices.push_back((j == 0) ? 0 : (vertexIndex + j));
                    indices.push_back((j == segments - 1) ? 1 : (vertexIndex + j + 1));
                }
                vertexIndex += segments;
            }
        }
        
        _coords.push_back(std::vector<cglib::vec3<double>*>());
        _normals.push_back(std::vector<cglib::vec4<float> >());
        _texCoords.push_back(std::vector<cglib::vec2<float> >());
        _indices.push_back(std::vector<unsigned int>());
        if (indices.size() <= GLContext::MAX_VERTEXBUFFER_SIZE) {
            _coords.back().swap(coords);
            _normals.back().swap(normals);
            _texCoords.back().swap(texCoords);
            _indices.back().swap(indices);
        } else {
            // Buffers too big, split into multiple buffers
            _coords.back().reserve(std::min(coords.size(), GLContext::MAX_VERTEXBUFFER_SIZE));
            _normals.back().reserve(std::min(normals.size(), GLContext::MAX_VERTEXBUFFER_SIZE));
            _texCoords.back().reserve(std::min(texCoords.size(), GLContext::MAX_VERTEXBUFFER_SIZE));
            _indices.back().reserve(std::min(indices.size(), GLContext::MAX_VERTEXBUFFER_SIZE));
            std::unordered_map<unsigned int, unsigned int> indexMap;
            indexMap.reserve(indices.size() * 2);
            for (std::size_t i = 0; i < indices.size(); i += 3) {
                
                // Check for possible GL buffer overflow
                if (_indices.back().size() + 3 > GLContext::MAX_VERTEXBUFFER_SIZE) {
                    // The buffer is full, create a new one
                    _coords.back().shrink_to_fit();
                    _coords.push_back(std::vector<cglib::vec3<double>*>());
                    _coords.back().reserve(std::min(coords.size(), GLContext::MAX_VERTEXBUFFER_SIZE));
                    _normals.back().shrink_to_fit();
                    _normals.push_back(std::vector<cglib::vec4<float> >());
                    _normals.back().reserve(std::min(normals.size(), GLContext::MAX_VERTEXBUFFER_SIZE));
                    _texCoords.back().shrink_to_fit();
                    _texCoords.push_back(std::vector<cglib::vec2<float> >());
                    _texCoords.back().reserve(std::min(texCoords.size(), GLContext::MAX_VERTEXBUFFER_SIZE));
                    _indices.back().shrink_to_fit();
                    _indices.push_back(std::vector<unsigned int>());
                    _indices.back().reserve(std::min(indices.size(), GLContext::MAX_VERTEXBUFFER_SIZE));
                    indexMap.clear();
                }
                
                for (int j = 0; j < 3; j++) {
                    unsigned int index = static_cast<unsigned int>(indices[i + j]);
                    auto it = indexMap.find(index);
                    if (it == indexMap.end()) {
                        unsigned int newIndex = static_cast<unsigned int>(_coords.back().size());
                        _coords.back().push_back(coords[index]);
                        _normals.back().push_back(normals[index]);
                        _texCoords.back().push_back(texCoords[index]);
                        _indices.back().push_back(newIndex);
                        indexMap[index] = newIndex;
                    } else {
                        _indices.back().push_back(it->second);
                    }
                }
            }
        }
        
        _coords.back().shrink_to_fit();
        _normals.back().shrink_to_fit();
        _texCoords.back().shrink_to_fit();
        _indices.back().shrink_to_fit();
    }
    
    const float LineDrawData::LINE_ENDPOINT_TESSELATION_FACTOR = 0.004f;
    const float LineDrawData::LINE_JOIN_TESSELATION_FACTOR = 0.002f;
    const float LineDrawData::LINE_JOIN_MIN_MITER_DOT = -0.8f;

    const int LineDrawData::IDEAL_CLICK_WIDTH = 64;

    const float LineDrawData::CLICK_WIDTH_COEF = 0.5f;
    
}
