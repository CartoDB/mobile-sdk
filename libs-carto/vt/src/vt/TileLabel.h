/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VT_TILELABEL_H_
#define _CARTO_VT_TILELABEL_H_

#include "TileId.h"
#include "Color.h"
#include "Bitmap.h"
#include "Font.h"
#include "ViewState.h"
#include "VertexArray.h"
#include "Styles.h"

#include <memory>
#include <array>
#include <list>
#include <vector>
#include <limits>
#include <algorithm>

#include <boost/variant.hpp>
#include <boost/optional.hpp>

namespace carto { namespace vt {
    class TileLabel final {
    public:
        struct LabelStyle {
            LabelOrientation orientation;
            ColorFunction colorFunc;
            FloatFunction sizeFunc;
            ColorFunction haloColorFunc;
            FloatFunction haloRadiusFunc;
            float scale;
            float ascent;
            float descent;
            boost::optional<cglib::mat3x3<float>> transform;
            std::shared_ptr<const GlyphMap> glyphMap;

            explicit LabelStyle(LabelOrientation orientation, ColorFunction colorFunc, FloatFunction sizeFunc, ColorFunction haloColorFunc, FloatFunction haloRadiusFunc, float scale, float ascent, float descent, const boost::optional<cglib::mat3x3<float>>& transform, std::shared_ptr<const GlyphMap> glyphMap) : orientation(orientation), colorFunc(std::move(colorFunc)), sizeFunc(std::move(sizeFunc)), haloColorFunc(std::move(haloColorFunc)), haloRadiusFunc(std::move(haloRadiusFunc)), scale(scale), ascent(ascent), descent(descent), transform(transform), glyphMap(std::move(glyphMap)) { }
        };
        
        explicit TileLabel(const TileId& tileId, long long localId, long long globalId, long long groupId, std::vector<Font::Glyph> glyphs, boost::optional<cglib::vec3<double>> position, std::vector<cglib::vec3<double>> vertices, std::shared_ptr<const LabelStyle> style);

        const TileId& getTileId() const { return _tileId; }
        long long getLocalId() const { return _localId; }
        long long getGlobalId() const { return _globalId; }
        long long getGroupId() const { return _groupId; }
        const std::shared_ptr<const LabelStyle>& getStyle() const { return _style; }
        
        bool isValid() const { return (bool) _placement; }

        int getPriority() const { return _priority; }
        void setPriority(int priority) { _priority = priority; }

        float getMinimumGroupDistance() const { return _minimumGroupDistance; }
        void setMinimumGroupDistance(float distance) { _minimumGroupDistance = distance; }

        float getOpacity() const { return _opacity; }
        void setOpacity(float opacity) { _opacity = opacity; }

        bool isVisible() const { return _visible; }
        void setVisible(bool visible) { _visible = visible; }

        bool isActive() const { return _active; }
        void setActive(bool active) { _active = active; }

        void transformGeometry(const cglib::mat4x4<double>& transform);
        void mergeGeometries(TileLabel& label);

        void snapPlacement(const TileLabel& label);
        bool updatePlacement(const ViewState& viewState);

        bool calculateCenter(cglib::vec3<double>& pos) const;
        bool calculateEnvelope(const ViewState& viewState, std::array<cglib::vec3<float>, 4>& envelope) const { return calculateEnvelope((_style->sizeFunc)(viewState), viewState, envelope); }
        bool calculateEnvelope(float size, const ViewState& viewState, std::array<cglib::vec3<float>, 4>& envelope) const;
        bool calculateVertexData(const ViewState& viewState, int styleIndex, VertexArray<cglib::vec3<float>>& vertices, VertexArray<cglib::vec2<short>>& texCoords, VertexArray<cglib::vec4<char>>& attribs, VertexArray<unsigned short>& indices) const { return calculateVertexData((_style->sizeFunc)(viewState), viewState, styleIndex, vertices, texCoords, attribs, indices); }
        bool calculateVertexData(float size, const ViewState& viewState, int styleIndex, VertexArray<cglib::vec3<float>>& vertices, VertexArray<cglib::vec2<short>>& texCoords, VertexArray<cglib::vec4<char>>& attribs, VertexArray<unsigned short>& indices) const;

    private:
        constexpr static float EXTRA_PLACEMENT_PIXELS = 30.0f; // extra visible pixels required for placement
        constexpr static float MAX_SINGLE_SEGMENT_ANGLE = 1.0472f; // maximum angle between consecutive segments, in radians
        constexpr static float MAX_SUMMED_SEGMENT_ANGLE = 2.0944f; // maximum sum of segment angles, in radians

        using Vertex = cglib::vec3<double>;
        using Vertices = std::vector<Vertex>;
        using VerticesList = std::list<Vertices>;

        struct Placement {
            struct Edge {
                cglib::vec2<float> pos0;
                cglib::vec2<float> pos1;
                cglib::vec2<float> binormal0;
                cglib::vec2<float> binormal1;
                cglib::vec2<float> xAxis;
                cglib::vec2<float> yAxis;
                float length;
                
                explicit Edge(const cglib::vec3<double>& p0, const cglib::vec3<double>& p1, const cglib::vec3<double>& origin) {
                    pos0 = cglib::vec2<float>::convert(cglib::proj_o(p0 - origin));
                    pos1 = cglib::vec2<float>::convert(cglib::proj_o(p1 - origin));
                    length = cglib::length(pos1 - pos0);
                    xAxis = (pos1 - pos0) * (1.0f / length);
                    yAxis = cglib::vec2<float>(-xAxis(1), xAxis(0));
                    binormal0 = yAxis;
                    binormal1 = yAxis;
                }

                void reverse() {
                    std::swap(pos0, pos1);
                    std::swap(binormal0, binormal1);
                    binormal0 = -binormal0;
                    binormal1 = -binormal1;
                    xAxis = -xAxis;
                    yAxis = -yAxis;
                }
            };
            
            std::vector<Edge> edges;
            std::size_t index;
            cglib::vec3<double> pos;
            
            explicit Placement(std::vector<Edge> baseEdges, std::size_t index, const cglib::vec3<double>& pos) : edges(std::move(baseEdges)), index(index), pos(pos) {
                for (std::size_t i = 1; i < edges.size(); i++) {
                    cglib::vec2<float> binormal = edges[i - 1].yAxis + edges[i].yAxis;
                    if (cglib::norm(binormal) != 0) {
                        binormal = cglib::unit(binormal);
                        edges[i - 1].binormal1 = edges[i].binormal0 = binormal * (1.0f / cglib::dot_product(edges[i - 1].yAxis, binormal));
                    }
                }
            }

            void reverse() {
                index = edges.size() - 1 - index;
                std::reverse(edges.begin(), edges.end());
                std::for_each(edges.begin(), edges.end(), [](Edge& edge) { edge.reverse(); });
            }
        };
        
        void setupCoordinateSystem(const ViewState& viewState, const std::shared_ptr<const Placement>& placement, cglib::vec3<float>& origin, cglib::vec3<float>& xAxis, cglib::vec3<float>& yAxis) const;
        void buildPointVertexData(VertexArray<cglib::vec2<float>>& vertices, VertexArray<cglib::vec2<short>>& texCoords, VertexArray<cglib::vec4<char>>& attribs, VertexArray<unsigned short>& indices) const;
        bool buildLineVertexData(const std::shared_ptr<const Placement>& placement, float scale, VertexArray<cglib::vec2<float>>& vertices, VertexArray<cglib::vec2<short>>& texCoords, VertexArray<cglib::vec4<char>>& attribs, VertexArray<unsigned short>& indices) const;

        std::shared_ptr<const Placement> getPlacement(const ViewState& viewState) const;
        std::shared_ptr<const Placement> reversePlacement(const std::shared_ptr<const Placement>& placement) const;
        std::shared_ptr<const Placement> findSnappedPointPlacement(const Vertex& position, const Vertices& vertices) const;
        std::shared_ptr<const Placement> findSnappedLinePlacement(const Vertex& position, const VerticesList& verticesList) const;
        std::shared_ptr<const Placement> findClippedPointPlacement(const ViewState& viewState, const Vertices& vertices) const;
        std::shared_ptr<const Placement> findClippedLinePlacement(const ViewState& viewState, const VerticesList& verticesList) const;

        const TileId _tileId;
        const long long _localId;
        const long long _globalId;
        const long long _groupId;
        const std::vector<Font::Glyph> _glyphs;
        const boost::optional<Vertex> _originalPosition;
        const Vertices _originalVertices;
        const std::shared_ptr<const LabelStyle> _style;

        Vertices _transformedPositions;
        VerticesList _transformedVerticesList;
        cglib::bbox2<float> _bbox = cglib::bbox2<float>::smallest();

        int _priority = 0;
        float _minimumGroupDistance = std::numeric_limits<float>::infinity();
        float _opacity = 0.0f;
        bool _visible = false;
        bool _active = false;

        std::shared_ptr<const Placement> _placement;
        std::shared_ptr<const Placement> _flippedPlacement;

        mutable bool _cachedValid = false;
        mutable float _cachedScale = 0;
        mutable std::shared_ptr<const Placement> _cachedPlacement;
        mutable VertexArray<cglib::vec2<float>> _cachedVertices;
        mutable VertexArray<cglib::vec2<short>> _cachedTexCoords;
        mutable VertexArray<cglib::vec4<char>> _cachedAttribs;
        mutable VertexArray<unsigned short> _cachedIndices;
    };
} }

#endif
