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

#include <boost/variant.hpp>
#include <boost/optional.hpp>

namespace carto { namespace vt {
    class TileLabel final {
    public:
        explicit TileLabel(const TileId& tileId, long long localId, long long globalId, long long groupId, std::shared_ptr<const Font> font, std::vector<Font::Glyph> glyphs, boost::optional<cglib::vec3<double>> position, std::vector<cglib::vec3<double>> vertices, LabelOrientation orientation, const boost::optional<cglib::mat3x3<float>>& transform, float scale, const Color& color);

        const TileId& getTileId() const { return _tileId; }
        long long getLocalId() const { return _localId; }
        long long getGlobalId() const { return _globalId; }
        long long getGroupId() const { return _groupId; }
        bool isValid() const { return (bool) _placement; }
        const Color& getColor() const { return _color; }

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

        LabelOrientation getOrientation() const { return _orientation; }
        const std::shared_ptr<const Font>& getFont() const { return _font; }

        void snapPlacement(const TileLabel& label);
        bool updatePlacement(const ViewState& viewState);

        bool calculateCenter(cglib::vec3<double>& pos) const;
        bool calculateEnvelope(const ViewState& viewState, std::array<cglib::vec3<float>, 4>& envelope) const;
        bool calculateVertexData(const ViewState& viewState, VertexArray<cglib::vec3<float>>& vertices, VertexArray<cglib::vec2<float>>& texCoords, VertexArray<unsigned short>& indices) const;

    private:
        constexpr static float EXTRA_PLACEMENT_PIXELS = 30.0f; // extra visible pixels required for placement
        constexpr static float MIN_SEGMENT_DOT = 0.866f; // minimum dot product between consecutive segments, to avoid very distorted placement. 0.886 is approx cos(30deg)

        using Vertex = cglib::vec3<double>;
        using Vertices = std::vector<Vertex>;
        using VerticesList = std::list<Vertices>;

        struct Placement {
            struct Edge {
                cglib::vec2<float> pos0;
                cglib::vec2<float> pos1;
                cglib::vec2<float> xAxis;
                cglib::vec2<float> yAxis;
                float length;
                
                explicit Edge(const cglib::vec3<double>& p0, const cglib::vec3<double>& p1, const cglib::vec3<double>& origin) {
                    pos0 = cglib::vec2<float>::convert(cglib::proj_o(p0 - origin));
                    pos1 = cglib::vec2<float>::convert(cglib::proj_o(p1 - origin));
                    length = static_cast<float>(cglib::length(pos1 - pos0));
                    xAxis = cglib::vec2<float>::convert(pos1 - pos0) * (1.0f / length);
                    yAxis = cglib::vec2<float>(-xAxis(1), xAxis(0));
                }
            };
            
            const std::vector<Edge> edges;
            const std::size_t index;
            const cglib::vec3<double> pos;
            
            explicit Placement(std::vector<Edge> edges, std::size_t index, const cglib::vec3<double>& pos) : edges(std::move(edges)), index(index), pos(pos) { }
        };
        
        void setupCoordinateSystem(const ViewState& viewState, const std::shared_ptr<const Placement>& placement, cglib::vec3<float>& origin, cglib::vec3<float>& xAxis, cglib::vec3<float>& yAxis) const;
        void buildPointVertexData(VertexArray<cglib::vec2<float>>& vertices, VertexArray<cglib::vec2<float>>& texCoords, VertexArray<unsigned short>& indices) const;
        bool buildLineVertexData(const std::shared_ptr<const Placement>& placement, float scale, VertexArray<cglib::vec2<float>>& vertices, VertexArray<cglib::vec2<float>>& texCoords, VertexArray<unsigned short>& indices) const;

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
        const std::shared_ptr<const Font> _font;
        const std::vector<Font::Glyph> _glyphs;
        const LabelOrientation _orientation;
        const boost::optional<Vertex> _originalPosition;
        const Vertices _originalVertices;
        const float _scale;
        const Color _color;

        boost::optional<cglib::mat3x3<float>> _transform;
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
        mutable cglib::vec3<double> _cachedOrigin;
        mutable std::shared_ptr<const Placement> _cachedPlacement;
        mutable VertexArray<cglib::vec2<float>> _cachedVertices;
        mutable VertexArray<cglib::vec2<float>> _cachedTexCoords;
        mutable VertexArray<unsigned short> _cachedIndices;
    };
} }

#endif
