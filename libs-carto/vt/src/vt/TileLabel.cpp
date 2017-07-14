#include "TileLabel.h"

#include <algorithm>
#include <map>

namespace carto { namespace vt {
    TileLabel::TileLabel(const TileId& tileId, long long localId, long long globalId, long long groupId, std::vector<Font::Glyph> glyphs, boost::optional<cglib::vec3<double>> position, std::vector<cglib::vec3<double>> vertices, std::shared_ptr<const LabelStyle> style) :
        _tileId(tileId), _localId(localId), _globalId(globalId), _groupId(groupId), _glyphs(std::move(glyphs)), _originalPosition(std::move(position)), _originalVertices(std::move(vertices)), _style(std::move(style))
    {
        _cachedVertices.reserve(_glyphs.size() * 4);
        _cachedTexCoords.reserve(_glyphs.size() * 4);
        _cachedAttribs.reserve(_glyphs.size() * 4);
        _cachedIndices.reserve(_glyphs.size() * 6);
        
        cglib::vec2<float> pen = cglib::vec2<float>(0, 0);
        for (const Font::Glyph& glyph : _glyphs) {
            if (glyph.codePoint == Font::CR_CODEPOINT) {
                pen = cglib::vec2<float>(0, 0);
            }
            else {
                _bbox.add(pen + glyph.offset);
                _bbox.add(pen + glyph.offset + glyph.size);
            }

            pen += glyph.advance;
        }
    }

    void TileLabel::transformGeometry(const cglib::mat4x4<double>& transform) {
        if (auto vertex = boost::get<Vertex>(&_originalPosition)) {
            _transformedPositions.resize(1);
            _transformedPositions.front() = cglib::transform_point(*vertex, transform);
        }
        else {
            _transformedPositions.clear();
        }
        
        if (!_originalVertices.empty()) {
            _transformedVerticesList.resize(1);
            _transformedVerticesList.front().clear();
            _transformedVerticesList.front().reserve(_originalVertices.size());
            for (const Vertex& vertex : _originalVertices) {
                _transformedVerticesList.front().push_back(cglib::transform_point(vertex, transform));
            }
        }
        else {
            _transformedVerticesList.clear();
        }
    }

    void TileLabel::mergeGeometries(TileLabel& label) {
        for (Vertex& labelVertex : label._transformedPositions) {
            if (std::find(_transformedPositions.begin(), _transformedPositions.end(), labelVertex) == _transformedPositions.end()) {
                _transformedPositions.push_back(labelVertex);
            }
        }

        for (Vertices& labelVertices : label._transformedVerticesList) {
            if (std::find(_transformedVerticesList.begin(), _transformedVerticesList.end(), labelVertices) == _transformedVerticesList.end()) {
                _transformedVerticesList.push_back(std::move(labelVertices));
            }
        }
    }
    
    void TileLabel::snapPlacement(const TileLabel& label) {
        _placement = label._placement;
        if (!_placement) {
            return;
        }

        if (!_transformedPositions.empty()) {
            _placement = _flippedPlacement = findSnappedPointPlacement(_placement->pos, _transformedPositions);
            if (_placement && !_transformedVerticesList.empty()) {
                _placement = findSnappedLinePlacement(_placement->pos, _transformedVerticesList);
                _flippedPlacement = reversePlacement(_placement);
            }
            return;
        }
        
        _placement = findSnappedLinePlacement(_placement->pos, _transformedVerticesList);
        _flippedPlacement = reversePlacement(_placement);
    }

    bool TileLabel::updatePlacement(const ViewState& viewState) {
        if (_placement) {
            std::array<cglib::vec3<float>, 4> envelope;
            calculateEnvelope(viewState, envelope);
            cglib::bbox3<double> bbox = cglib::bbox3<double>::smallest();
            for (const cglib::vec3<float>& pos : envelope) {
                bbox.add(viewState.origin + cglib::vec3<double>::convert(pos));
            }
            if (viewState.frustum.inside(bbox)) {
                return false;
            }
        }

        if (!_transformedPositions.empty()) {
            _placement = _flippedPlacement = findClippedPointPlacement(viewState, _transformedPositions);
            if (_placement && !_transformedVerticesList.empty()) {
                _placement = findSnappedLinePlacement(_placement->pos, _transformedVerticesList);
                _flippedPlacement = reversePlacement(_placement);
            }
            return true;
        }

        _placement = findClippedLinePlacement(viewState, _transformedVerticesList);
        _flippedPlacement = reversePlacement(_placement);
        return true;
    }

    bool TileLabel::calculateCenter(cglib::vec3<double>& pos) const {
        if (!_placement) {
            return false;
        }

        pos = _placement->pos;
        return true;
    }

    bool TileLabel::calculateEnvelope(float size, const ViewState& viewState, std::array<cglib::vec3<float>, 4>& envelope) const {
        std::shared_ptr<const Placement> placement = getPlacement(viewState);
        float scale = size * viewState.zoomScale * _style->scale;
        if (!placement || scale <= 0) {
            cglib::vec3<float> origin(0, 0, static_cast<float>(-viewState.origin(2)));
            for (int i = 0; i < 4; i++) {
                envelope[i] = origin;
            }
            return false;
        }

        cglib::vec3<float> origin, xAxis, yAxis;
        setupCoordinateSystem(viewState, placement, origin, xAxis, yAxis);

        if (_style->orientation == LabelOrientation::LINE) {
            // For line orientation, we have to calculate vertex data and then project vertices to the principal axes
            if (scale != _cachedScale || placement != _cachedPlacement) {
                _cachedVertices.clear();
                _cachedTexCoords.clear();
                _cachedAttribs.clear();
                _cachedIndices.clear();
                _cachedValid = buildLineVertexData(placement, scale, _cachedVertices, _cachedTexCoords, _cachedAttribs, _cachedIndices);
                _cachedScale = scale;
                _cachedPlacement = placement;
            }

            float minX = std::numeric_limits<float>::max(), maxX = -std::numeric_limits<float>::max();
            float minY = std::numeric_limits<float>::max(), maxY = -std::numeric_limits<float>::max();
            for (const cglib::vec2<float>& vertex : _cachedVertices) {
                cglib::vec3<float> pos = origin + cglib::expand(vertex);
                float x = cglib::dot_product(xAxis, pos);
                float y = cglib::dot_product(yAxis, pos);
                minX = std::min(minX, x); maxX = std::max(maxX, x);
                minY = std::min(minY, y); maxY = std::max(maxY, y);
            }

            cglib::vec3<float> zAxis = cglib::vector_product(xAxis, yAxis);
            cglib::vec3<float> zOrigin = zAxis * cglib::dot_product(origin, zAxis);

            envelope[0] = zOrigin + xAxis * minX + yAxis * minY;
            envelope[1] = zOrigin + xAxis * maxX + yAxis * minY;
            envelope[2] = zOrigin + xAxis * maxX + yAxis * maxY;
            envelope[3] = zOrigin + xAxis * minX + yAxis * maxY;
            return _cachedValid;
        }
        else {
            xAxis *= scale;
            yAxis *= scale;

            // Use bounding box for envelope
            if (_style->transform) {
                cglib::vec2<float> p00 = cglib::transform_point_affine(cglib::vec2<float>(_bbox.min(0), _bbox.min(1)), _style->transform.get());
                cglib::vec2<float> p01 = cglib::transform_point_affine(cglib::vec2<float>(_bbox.min(0), _bbox.max(1)), _style->transform.get());
                cglib::vec2<float> p10 = cglib::transform_point_affine(cglib::vec2<float>(_bbox.max(0), _bbox.min(1)), _style->transform.get());
                cglib::vec2<float> p11 = cglib::transform_point_affine(cglib::vec2<float>(_bbox.max(0), _bbox.max(1)), _style->transform.get());
                envelope[0] = origin + xAxis * p00(0) + yAxis * p00(1);
                envelope[1] = origin + xAxis * p10(0) + yAxis * p10(1);
                envelope[2] = origin + xAxis * p11(0) + yAxis * p11(1);
                envelope[3] = origin + xAxis * p01(0) + yAxis * p01(1);
            }
            else {
                envelope[0] = origin + xAxis * _bbox.min(0) + yAxis * _bbox.min(1);
                envelope[1] = origin + xAxis * _bbox.max(0) + yAxis * _bbox.min(1);
                envelope[2] = origin + xAxis * _bbox.max(0) + yAxis * _bbox.max(1);
                envelope[3] = origin + xAxis * _bbox.min(0) + yAxis * _bbox.max(1);
            }
            return true;
        }
    }

    bool TileLabel::calculateVertexData(float size, const ViewState& viewState, int styleIndex, VertexArray<cglib::vec3<float>>& vertices, VertexArray<cglib::vec2<short>>& texCoords, VertexArray<cglib::vec4<char>>& attribs, VertexArray<unsigned short>& indices) const {
        std::shared_ptr<const Placement> placement = getPlacement(viewState);
        float scale = size * viewState.zoomScale * _style->scale;
        if (!placement || scale <= 0) {
            return false;
        }

        // Build vertex data cache
        unsigned short offset = static_cast<unsigned short>(vertices.size());
        if (_style->orientation == LabelOrientation::LINE) {
            // Check if cached vertex data can be used
            if (scale != _cachedScale || placement != _cachedPlacement) {
                _cachedVertices.clear();
                _cachedTexCoords.clear();
                _cachedAttribs.clear();
                _cachedIndices.clear();
                _cachedValid = buildLineVertexData(placement, scale, _cachedVertices, _cachedTexCoords, _cachedAttribs, _cachedIndices);
                _cachedScale = scale;
                _cachedPlacement = placement;
            }

            cglib::vec3<float> origin = cglib::vec3<float>::convert(placement->pos - viewState.origin);
            for (const cglib::vec2<float>& vertex : _cachedVertices) {
                vertices.append(origin + cglib::vec3<float>(vertex(0), vertex(1), 0));
            }
        }
        else {
            // If no cached data, recalculate and cache it
            if (!_cachedValid) {
                _cachedVertices.clear();
                _cachedTexCoords.clear();
                _cachedAttribs.clear();
                _cachedIndices.clear();
                buildPointVertexData(_cachedVertices, _cachedTexCoords, _cachedAttribs, _cachedIndices);
                _cachedValid = true;
            }

            cglib::vec3<float> origin, xAxis, yAxis;
            setupCoordinateSystem(viewState, placement, origin, xAxis, yAxis);
            for (const cglib::vec2<float>& vertex : _cachedVertices) {
                vertices.append(origin + xAxis * (vertex(0) * scale) + yAxis * (vertex(1) * scale));
            }
        }

        texCoords.copy(_cachedTexCoords, 0, _cachedTexCoords.size());

        attribs.copy(_cachedAttribs, 0, _cachedAttribs.size());
        for (cglib::vec4<char>* it = attribs.end() - _cachedAttribs.size(); it != attribs.end(); it++) {
            *it = cglib::vec4<char>(static_cast<int>(styleIndex), (*it)(1), static_cast<char>(_opacity * 127.0f), 0);
        }

        indices.copy(_cachedIndices, 0, _cachedIndices.size());
        for (unsigned short* it = indices.end() - _cachedIndices.size(); it != indices.end(); it++) {
            *it += offset;
        }

        return _cachedValid;
    }

    void TileLabel::buildPointVertexData(VertexArray<cglib::vec2<float>>& vertices, VertexArray<cglib::vec2<short>>& texCoords, VertexArray<cglib::vec4<char>>& attribs, VertexArray<unsigned short>& indices) const {
        cglib::vec2<float> pen(0, 0);
        for (const Font::Glyph& glyph : _glyphs) {
            // If carriage return, reposition pen and state to the initial position
            if (glyph.codePoint == Font::CR_CODEPOINT) {
                pen = cglib::vec2<float>(0, 0);
            }
            else if (glyph.codePoint != Font::SPACE_CODEPOINT) {
                unsigned short i0 = static_cast<unsigned short>(vertices.size());
                indices.append(i0 + 0, i0 + 1, i0 + 2);
                indices.append(i0 + 0, i0 + 2, i0 + 3);

                short u0 = static_cast<short>(glyph.baseGlyph.x), u1 = static_cast<short>(glyph.baseGlyph.x + glyph.baseGlyph.width);
                short v0 = static_cast<short>(glyph.baseGlyph.y), v1 = static_cast<short>(glyph.baseGlyph.y + glyph.baseGlyph.height);
                texCoords.append(cglib::vec2<short>(u0, v1), cglib::vec2<short>(u1, v1), cglib::vec2<short>(u1, v0), cglib::vec2<short>(u0, v0));

                cglib::vec4<char> attrib(0, glyph.baseGlyph.sdfMode ? -1 : 1, 0, 0);
                attribs.append(attrib, attrib, attrib, attrib);

                if (_style->transform) {
                    cglib::vec2<float> p0 = cglib::transform_point_affine(pen + glyph.offset, _style->transform.get());
                    cglib::vec2<float> p1 = cglib::transform_point_affine(pen + glyph.offset + cglib::vec2<float>(glyph.size(0), 0), _style->transform.get());
                    cglib::vec2<float> p2 = cglib::transform_point_affine(pen + glyph.offset + glyph.size, _style->transform.get());
                    cglib::vec2<float> p3 = cglib::transform_point_affine(pen + glyph.offset + cglib::vec2<float>(0, glyph.size(1)), _style->transform.get());
                    vertices.append(p0, p1, p2, p3);
                }
                else {
                    cglib::vec2<float> p0 = pen + glyph.offset;
                    cglib::vec2<float> p3 = pen + glyph.offset + glyph.size;
                    vertices.append(p0, cglib::vec2<float>(p3(0), p0(1)), p3, cglib::vec2<float>(p0(0), p3(1)));
                }
            }

            // Move pen
            pen += glyph.advance;
        }
    }

    bool TileLabel::buildLineVertexData(const std::shared_ptr<const Placement>& placement, float scale, VertexArray<cglib::vec2<float>>& vertices, VertexArray<cglib::vec2<short>>& texCoords, VertexArray<cglib::vec4<char>>& attribs, VertexArray<unsigned short>& indices) const {
        std::size_t edgeIndex = placement->index;
        cglib::vec2<float> edgePos(0, 0);
        float edgeLen = cglib::length(placement->edges[edgeIndex].pos1 - edgePos) / scale;

        bool valid = true;
        cglib::vec2<float> pen(0, 0);
        for (const Font::Glyph& glyph : _glyphs) {
            // If carriage return, reposition pen and state to the initial position
            if (glyph.codePoint == Font::CR_CODEPOINT) {
                pen = cglib::vec2<float>(0, 0);

                edgeIndex = placement->index;
                edgePos = cglib::vec2<float>(0, 0);
                edgeLen = cglib::length(placement->edges[edgeIndex].pos1 - edgePos) / scale;
            }
            else if (glyph.codePoint != Font::SPACE_CODEPOINT) {
                unsigned short i0 = static_cast<unsigned short>(vertices.size());
                indices.append(i0 + 0, i0 + 1, i0 + 2);
                indices.append(i0 + 0, i0 + 2, i0 + 3);

                short u0 = static_cast<short>(glyph.baseGlyph.x), u1 = static_cast<short>(glyph.baseGlyph.x + glyph.baseGlyph.width);
                short v0 = static_cast<short>(glyph.baseGlyph.y), v1 = static_cast<short>(glyph.baseGlyph.y + glyph.baseGlyph.height);
                texCoords.append(cglib::vec2<short>(u0, v1), cglib::vec2<short>(u1, v1), cglib::vec2<short>(u1, v0), cglib::vec2<short>(u0, v0));

                cglib::vec4<char> attrib(0, glyph.baseGlyph.sdfMode ? -1 : 1, 0, 0);
                attribs.append(attrib, attrib, attrib, attrib);
                
                const cglib::vec2<float>& xAxis = placement->edges[edgeIndex].xAxis;
                const cglib::vec2<float>& yAxis = placement->edges[edgeIndex].yAxis;
                if (_style->transform) {
                    cglib::vec2<float> p0 = cglib::transform_point_affine(pen + glyph.offset, _style->transform.get()) * scale;
                    cglib::vec2<float> p1 = cglib::transform_point_affine(pen + glyph.offset + cglib::vec2<float>(glyph.size(0), 0), _style->transform.get()) * scale;
                    cglib::vec2<float> p2 = cglib::transform_point_affine(pen + glyph.offset + glyph.size, _style->transform.get()) * scale;
                    cglib::vec2<float> p3 = cglib::transform_point_affine(pen + glyph.offset + cglib::vec2<float>(0, glyph.size(1)), _style->transform.get()) * scale;
                    vertices.append(edgePos + xAxis * p0(0) + yAxis * p0(1), edgePos + xAxis * p1(0) + yAxis * p1(1), edgePos + xAxis * p2(0) + yAxis * p2(1), edgePos + xAxis * p3(0) + yAxis * p3(1));
                }
                else {
                    cglib::vec2<float> p0 = (pen + glyph.offset) * scale;
                    cglib::vec2<float> p3 = (pen + glyph.offset + glyph.size) * scale;
                    vertices.append(edgePos + xAxis * p0(0) + yAxis * p0(1), edgePos + xAxis * p3(0) + yAxis * p0(1), edgePos + xAxis * p3(0) + yAxis * p3(1), edgePos + xAxis * p0(0) + yAxis * p3(1));
                }
            }

            // Move pen
            pen += glyph.advance;

            // Check if we the pen has gone 'over' line segment
            int edgeDir = 0;
            if (glyph.codePoint != Font::SPACE_CODEPOINT && glyph.codePoint != Font::CR_CODEPOINT) {
                edgeDir = glyph.advance(0) > 0 ? 1 : -1;
            }

            const std::vector<Placement::Edge>& edges = placement->edges;
            if (edgeDir <= 0 && pen(0) < 0) {
                while (true) {
                    const cglib::vec2<float>& p0 = edges[edgeIndex].pos0;
                    float len = cglib::length(edgePos - p0) / scale;
                    pen(0) += len;
                    edgePos = p0;
                    if (pen(0) >= 0) {
                        break;
                    }
                    if (edgeIndex == 0) {
                        valid = false;
                        break;
                    }
                    edgeIndex--;

                    if (edgeDir < 0) {
                        float cos = cglib::dot_product(edges[edgeIndex].xAxis, edges[edgeIndex + 1].xAxis);
                        float sin = cglib::dot_product(edges[edgeIndex].xAxis, edges[edgeIndex + 1].yAxis);
                        pen(0) = cos * pen(0) - sin * (sin < 0 ? pen(1) + _style->ascent * 0.5f : 0);
                    }
                }

                edgeLen = cglib::length(edges[edgeIndex].pos1 - edgePos) / scale;
            }
            else if (edgeDir >= 0 && pen(0) >= edgeLen) {
                while (true) {
                    const cglib::vec2<float>& p1 = edges[edgeIndex].pos1;
                    float len = cglib::length(p1 - edgePos) / scale;
                    if (pen(0) < len) {
                        break;
                    }
                    pen(0) -= len;
                    edgePos = p1;
                    if (edgeIndex + 1 >= edges.size()) {
                        valid = false;
                        break;
                    }
                    edgeIndex++;

                    if (edgeDir > 0) {
                        float cos = cglib::dot_product(edges[edgeIndex - 1].xAxis, edges[edgeIndex].xAxis);
                        float sin = cglib::dot_product(edges[edgeIndex - 1].xAxis, edges[edgeIndex].yAxis);
                        pen(0) = cos * pen(0) + sin * (sin > 0 ? pen(1) + _style->ascent * 0.5f : 0);
                    }
                }

                edgeLen = cglib::length(edges[edgeIndex].pos1 - edgePos) / scale;
            }
        }

        return valid;
    }

    void TileLabel::setupCoordinateSystem(const ViewState& viewState, const std::shared_ptr<const Placement>& placement, cglib::vec3<float>& origin, cglib::vec3<float>& xAxis, cglib::vec3<float>& yAxis) const {
        origin = cglib::vec3<float>::convert(placement->pos - viewState.origin);
        switch (_style->orientation) {
        case LabelOrientation::BILLBOARD_2D:
            xAxis = viewState.orientation[0];
            yAxis = cglib::vector_product(cglib::vec3<float>(0, 0, 1), xAxis);
            break;
        case LabelOrientation::BILLBOARD_3D:
            xAxis = viewState.orientation[0];
            yAxis = viewState.orientation[1];
            break;
        case LabelOrientation::POINT:
            xAxis = cglib::vec3<float>(1, 0, 0);
            yAxis = cglib::vec3<float>(0, 1, 0);
            break;
        case LabelOrientation::POINT_FLIPPING:
            if (_style->transform) {
                const cglib::mat3x3<float>& m = _style->transform.get();
                float s = (cglib::dot_product(cglib::vec3<float>(m(0, 0), m(1, 0), m(2, 0)), viewState.orientation[0]) < 0 ? -1.0f : 1.0f);
                xAxis = cglib::vec3<float>(s, 0, 0);
                yAxis = cglib::vec3<float>(0, s, 0);
            } else {
                float s = (viewState.orientation[0][0] < 0 ? -1.0f : 1.0f);
                xAxis = cglib::vec3<float>(s, 0, 0);
                yAxis = cglib::vec3<float>(0, s, 0);
            }
            break;
        default: // LabelOrientation::LINE
            xAxis = cglib::expand(placement->edges[placement->index].xAxis);
            yAxis = cglib::expand(placement->edges[placement->index].yAxis);
            break;
        }
    }

    std::shared_ptr<const TileLabel::Placement> TileLabel::getPlacement(const ViewState& viewState) const {
        if (_style->orientation != LabelOrientation::LINE) {
            return _placement;
        }
        if (!_placement || _placement->edges.empty()) {
            return std::shared_ptr<const Placement>();
        }
        if (cglib::dot_product(_placement->edges[_placement->index].xAxis, cglib::proj_o(viewState.orientation[0])) > 0) {
            return _placement;
        }
        if (!_flippedPlacement || _flippedPlacement->edges.empty()) {
            return std::shared_ptr<const Placement>();
        }
        return _flippedPlacement;
    }

    std::shared_ptr<const TileLabel::Placement> TileLabel::reversePlacement(const std::shared_ptr<const Placement>& placement) const {
        if (!placement) {
            return placement;
        }
        std::vector<Placement::Edge> flippedEdges = placement->edges;
        std::reverse(flippedEdges.begin(), flippedEdges.end());
        for (Placement::Edge& edge : flippedEdges) {
            std::swap(edge.pos0, edge.pos1);
            edge.xAxis = -edge.xAxis;
            edge.yAxis = -edge.yAxis;
        }
        return std::make_shared<const Placement>(std::move(flippedEdges), flippedEdges.size() - 1 - placement->index, placement->pos);
    }

    std::shared_ptr<const TileLabel::Placement> TileLabel::findSnappedPointPlacement(const Vertex& position, const Vertices& vertices) const {
        cglib::vec3<double> bestPos = position;
        double bestDist = std::numeric_limits<double>::infinity();
        for (const Vertex& vertex : vertices) {
            double dist = cglib::length(vertex - position);
            if (dist < bestDist) {
                bestPos = vertex;
                bestDist = dist;
            }
        }
        
        if (_placement && _placement->pos == bestPos && _placement->edges.empty()) {
            return _placement;
        }
        return std::make_shared<const Placement>(std::vector<Placement::Edge>(), 0, bestPos);
    }

    std::shared_ptr<const TileLabel::Placement> TileLabel::findSnappedLinePlacement(const Vertex& position, const VerticesList& verticesList) const {
        std::size_t bestIndex = 0;
        const Vertices* bestVertices = nullptr;
        cglib::vec3<double> bestPos = position;
        double bestDist = std::numeric_limits<double>::infinity();
        for (const Vertices& vertices : verticesList) {
            // Try to find a closest point on vertices to the given position
            for (std::size_t j = 1; j < vertices.size(); j++) {
                cglib::vec3<double> edgeVec = vertices[j] - vertices[j - 1];
                double edgeLen2 = cglib::dot_product(edgeVec, edgeVec);
                if (edgeLen2 == 0) {
                    continue;
                }
                double t = cglib::dot_product(edgeVec, position - vertices[j - 1]) / edgeLen2;
                cglib::vec3<double> edgePos = vertices[j - 1] + edgeVec * std::max(0.0, std::min(1.0, t));
                double weight = (1.0 / j) + (1.0 / (vertices.size() - j)); // favor positions far from endpoint, will result in more stable placements
                double dist = cglib::length(edgePos - position) * weight;
                if (dist < bestDist) {
                    bestIndex = j - 1;
                    bestVertices = &vertices;
                    bestPos = edgePos;
                    bestDist = dist;
                }
            }
        }
        if (!bestVertices) {
            return std::shared_ptr<const Placement>();
        }

        std::vector<Placement::Edge> edges;
        for (std::size_t j = 1; j < bestVertices->size(); j++) {
            edges.emplace_back((*bestVertices)[j - 1], (*bestVertices)[j], bestPos);
        }

        // Postprocess edges, keep only relatively straight parts, to avoid distorted texts
        float summedAngle = 0;
        for (std::size_t j0 = bestIndex, j1 = bestIndex + 1; true; ) {
            bool r0 = false;
            if (j0 > 0) {
                cglib::vec2<float> edgeVec1 = edges[j0 - 1].pos1 - edges[j0 - 1].pos0;
                cglib::vec2<float> edgeVec2 = edges[j0].pos1 - edges[j0].pos0;
                float cos = cglib::dot_product(cglib::unit(edgeVec1), cglib::unit(edgeVec2));
                float angle = std::acos(std::min(1.0f, std::max(-1.0f, cos)));
                if (angle < MAX_SINGLE_SEGMENT_ANGLE && angle + summedAngle < MAX_SUMMED_SEGMENT_ANGLE) {
                    summedAngle += angle;
                    j0--;
                    r0 = true;
                }
            }

            bool r1 = false;
            if (j1 < edges.size()) {
                cglib::vec2<float> edgeVec1 = edges[j1 - 1].pos1 - edges[j1 - 1].pos0;
                cglib::vec2<float> edgeVec2 = edges[j1].pos1 - edges[j1].pos0;
                float cos = cglib::dot_product(cglib::unit(edgeVec1), cglib::unit(edgeVec2));
                float angle = std::acos(std::min(1.0f, std::max(-1.0f, cos)));
                if (angle < MAX_SINGLE_SEGMENT_ANGLE && angle + summedAngle < MAX_SUMMED_SEGMENT_ANGLE) {
                    summedAngle += angle;
                    j1++;
                    r1 = true;
                }
            }

            if (!r0 && !r1) {
                edges = std::vector<Placement::Edge>(edges.begin() + j0, edges.begin() + j1);
                bestIndex -= j0;
                break;
            }
        }

        // If the placement did not change, return original object. Otherwise create new.
        if (_placement && _placement->index == bestIndex && _placement->pos == bestPos && _placement->edges.size() == edges.size()) {
            return _placement;
        }
        return std::make_shared<const Placement>(std::move(edges), bestIndex, bestPos);
    }

    std::shared_ptr<const TileLabel::Placement> TileLabel::findClippedPointPlacement(const ViewState& viewState, const Vertices& vertices) const {
        cglib::bbox2<float> bbox = _bbox;
        if (_style->transform) {
            bbox = cglib::transform_bbox(bbox, _style->transform.get());
        }
        
        for (const Vertex& vertex : vertices) {
            // Check that text is visible, calculate text distance from all frustum planes
            bool inside = true;
            for (int plane = 0; plane < 6; plane++) {
                float size = 0;
                switch (plane) {
                case 2:
                    size = -bbox.min(1);
                    break;
                case 3:
                    size = bbox.max(1);
                    break;
                case 4:
                    size = bbox.max(0) / viewState.aspect;
                    break;
                case 5:
                    size = -bbox.min(0) / viewState.aspect;
                    break;
                }
                double dist = viewState.frustum.plane_distance(plane, vertex);
                if (dist < -size * _style->scale * viewState.zoomScale) {
                    inside = false;
                    break;
                }
            }
            if (inside) {
                return std::make_shared<const Placement>(std::vector<Placement::Edge>(), 0, vertex);
            }
        }
        return std::shared_ptr<const Placement>();
    }

    std::shared_ptr<const TileLabel::Placement> TileLabel::findClippedLinePlacement(const ViewState& viewState, const VerticesList& verticesList) const {
        // Split vertices list into relatively straight segments
        VerticesList splitVerticesList;
        for (const Vertices& vertices : verticesList) {
            std::size_t i0 = 0;
            float summedAngle = 0;
            cglib::vec3<double> lastEdgeVec(0, 0, 0);
            for (std::size_t i = 1; i < vertices.size(); i++) {
                cglib::vec3<double> edgeVec = cglib::unit(vertices[i] - vertices[i - 1]);
                if (lastEdgeVec != cglib::vec3<double>::zero()) {
                    float cos = static_cast<float>(cglib::dot_product(edgeVec, lastEdgeVec));
                    float angle = std::acos(std::min(1.0f, std::max(-1.0f, cos)));
                    summedAngle += angle;
                    if (angle > MAX_SINGLE_SEGMENT_ANGLE || summedAngle > MAX_SUMMED_SEGMENT_ANGLE) {
                        splitVerticesList.emplace_back(vertices.begin() + i0, vertices.begin() + i);
                        i0 = i - 1;
                        summedAngle = 0;
                    }
                }
                lastEdgeVec = edgeVec;
            }
            splitVerticesList.emplace_back(vertices.begin() + i0, vertices.end());
        }

        // Clip each vertex list against frustum, if resulting list is inside frustum, return its center
        double bestLen = (_style->orientation == LabelOrientation::LINE ? (_bbox.size()(0) + EXTRA_PLACEMENT_PIXELS) * _style->scale * viewState.zoomScale : 0);
        std::shared_ptr<const Placement> bestPlacement;
        for (const Vertices& vertices : splitVerticesList) {
            if (vertices.size() < 2) {
                continue;
            }

            std::pair<std::size_t, double> t0(0, 0);
            std::pair<std::size_t, double> t1(vertices.size() - 2, 1);
            for (int plane = 0; plane < 6; plane++) {
                if (t0 > t1) {
                    break;
                }
                double prevDist = viewState.frustum.plane_distance(plane, vertices[t0.first]);
                for (std::size_t i = t0.first; i <= t1.first; i++) {
                    double nextDist = viewState.frustum.plane_distance(plane, vertices[i + 1]);
                    if (nextDist > 0 && prevDist < 0) {
                        t0 = std::max(t0, std::pair<std::size_t, double>(i, 1 - nextDist / (nextDist - prevDist)));
                    }
                    else if (nextDist < 0 && prevDist > 0) {
                        t1 = std::min(t1, std::pair<std::size_t, double>(i, 1 - nextDist / (nextDist - prevDist)));
                    }
                    else if (nextDist < 0 && prevDist < 0) {
                        t0 = std::max(t0, std::pair<std::size_t, double>(i + 1, 0));
                    }
                    prevDist = nextDist;
                }
            }
            if (t0 < t1) {
                double len = 0;
                for (std::size_t i = t0.first; i <= t1.first; i++) {
                    Vertex pos0 = vertices[i];
                    if (i == t0.first) {
                        pos0 = vertices[i] * (1 - t0.second) + vertices[i + 1] * t0.second;
                    }
                    Vertex pos1 = vertices[i + 1];
                    if (i == t1.first) {
                        pos1 = vertices[i] * (1 - t1.second) + vertices[i + 1] * t1.second;
                    }
                    double diff = cglib::length(pos1 - pos0);
                    len += diff;
                }
                    
                if (len > bestLen) {
                    double ofs = len * 0.5;
                    for (std::size_t i = t0.first; i <= t1.first; i++) {
                        Vertex p0 = vertices[i];
                        if (i == t0.first) {
                            p0 = vertices[i] * (1 - t0.second) + vertices[i + 1] * t0.second;
                        }
                        Vertex p1 = vertices[i + 1];
                        if (i == t1.first) {
                            p1 = vertices[i] * (1 - t1.second) + vertices[i + 1] * t1.second;
                        }
                        double diff = cglib::length(p1 - p0);
                        if (ofs < diff) {
                            Vertex pos = p0 + (p1 - p0) * (ofs / diff); // this assumes central anchor point
                            std::vector<Placement::Edge> edges;
                            for (std::size_t j = 1; j < vertices.size(); j++) {
                                edges.emplace_back(vertices[j - 1], vertices[j], pos);
                            }
                            bestPlacement = std::make_shared<const Placement>(std::move(edges), i, pos);
                            bestLen = len;
                            break;
                        }
                        ofs -= diff;
                    }
                }
            }
        }
        return bestPlacement;
    }
} }
