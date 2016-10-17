/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VT_TILELAYERBUILDER_H_
#define _CARTO_VT_TILELAYERBUILDER_H_

#include "TileBitmap.h"
#include "TileGeometry.h"
#include "TileLabel.h"
#include "TileLayer.h"
#include "Styles.h"
#include "PoolAllocator.h"
#include "VertexArray.h"

#include <memory>
#include <vector>
#include <list>
#include <functional>

#include <boost/variant.hpp>

#include <cglib/vec.h>
#include <cglib/mat.h>
#include <cglib/bbox.h>

namespace carto { namespace vt {
    class TileLayerBuilder final {
    public:
        using Vertex = cglib::vec2<float>;
        using Vertices = std::vector<Vertex>;
        using VerticesList = std::vector<Vertices>;

        struct BitmapLabelInfo {
            long long id = 0;
            long long groupId = 0;
            boost::variant<Vertex, Vertices> position;
            float minimumGroupDistance = 0;

            BitmapLabelInfo() = default;
            explicit BitmapLabelInfo(long long id, long long groupId, boost::variant<Vertex, Vertices> position, float minimumGroupDistance) : id(id), groupId(groupId), position(std::move(position)), minimumGroupDistance(minimumGroupDistance) { }
        };

        struct TextLabelInfo {
            long long id = 0;
            long long groupId = 0;
            std::string text;
            boost::optional<Vertex> position;
            Vertices vertices;
            float minimumGroupDistance = 0;

            TextLabelInfo() = default;
            explicit TextLabelInfo(long long id, long long groupId, std::string text, boost::optional<Vertex> position, Vertices vertices, float minimumGroupDistance) : id(id), groupId(groupId), text(std::move(text)), position(std::move(position)), vertices(std::move(vertices)), minimumGroupDistance(minimumGroupDistance) { }
        };

        explicit TileLayerBuilder(const TileId& tileId, float tileSize, float geomScale);

        void addBitmap(const std::shared_ptr<TileBitmap>& bitmap);
        void addPoints(const std::function<bool(long long& id, Vertex& vertex)>& generator, const PointStyle& style);
        void addTexts(const std::function<bool(long long& id, Vertex& vertex, std::string& text)>& generator, const TextStyle& style);
        void addLines(const std::function<bool(long long& id, Vertices& vertices)>& generator, const LineStyle& style);
        void addPolygons(const std::function<bool(long long& id, VerticesList& verticesList)>& generator, const PolygonStyle& style);
        void addPolygons3D(const std::function<bool(long long& id, VerticesList& verticesList)>& generator, float height, const Polygon3DStyle& style);
        void addBitmapLabels(const std::function<bool(long long& id, BitmapLabelInfo& labelInfo)>& generator, const BitmapLabelStyle& style);
        void addTextLabels(const std::function<bool(long long& id, TextLabelInfo& labelInfo)>& generator, const TextLabelStyle& style);

        std::shared_ptr<TileLayer> build(std::string layerName, int layerIdx, std::shared_ptr<FloatFunction> opacity, boost::optional<CompOp> compOp);

    private:
        constexpr static int RESERVED_VERTICES = 4096;

        constexpr static float MIN_MITER_DOT = -0.8f; // minimum allowed dot product result between segment direction vectors, if less, then miter-join is not used

        struct BuilderParameters {
            TileGeometry::Type type;
            std::array<StrokeMap::StrokeId, TileGeometry::StyleParameters::MAX_PARAMETERS> lineStrokeIds;
            std::shared_ptr<const StrokeMap> strokeMap;
            std::shared_ptr<const GlyphMap> glyphMap;

            BuilderParameters() : type(TileGeometry::Type::NONE), lineStrokeIds(), strokeMap(), glyphMap() { }
        };

        void appendGeometry();
        void appendGeometry(float verticesScale, float binormalsScale, float texCoordsScale, const VertexArray<cglib::vec2<float>>& vertices, const VertexArray<cglib::vec2<float>>& texCoords, const VertexArray<cglib::vec2<float>>& binormals, const VertexArray<float>& heights, const VertexArray<cglib::vec4<char>>& attribs, const VertexArray<unsigned int>& indices, const VertexArray<long long>& ids, std::size_t offset, std::size_t count);
        float calculateScale(VertexArray<cglib::vec2<float>>& values) const;

        bool tesselateGlyph(const Vertex& vertex, char styleIndex, const cglib::vec2<float>& pen, const Font::Glyph* glyph);
        bool tesselatePolygon(const VerticesList& verticesList, char styleIndex, const PolygonStyle& style);
        bool tesselatePolygon3D(const VerticesList& verticesList, float height, char styleIndex, const Polygon3DStyle& style);
        bool tesselateLine(const Vertices& points, char styleIndex, const StrokeMap::Stroke* stroke, const LineStyle& style);
        bool tesselateLineEndPoint(const Vertex& p0, float u0, float v0, float v1, int i0, const cglib::vec2<float>& tangent, const cglib::vec2<float>& binormal, char styleIndex, const LineStyle& style);

        const TileId _tileId;
        const float _tileSize;
        const float _geomScale;
        BuilderParameters _builderParameters;
        TileGeometry::StyleParameters _styleParameters;

        VertexArray<cglib::vec2<float>> _vertices;
        VertexArray<cglib::vec2<float>> _texCoords;
        VertexArray<cglib::vec2<float>> _binormals;
        VertexArray<float> _heights;
        VertexArray<cglib::vec4<char>> _attribs;
        VertexArray<unsigned int> _indices;
        VertexArray<long long> _ids;

        std::vector<std::shared_ptr<TileBitmap>> _bitmapList;
        std::vector<std::shared_ptr<TileGeometry>> _geometryList;
        std::vector<std::shared_ptr<TileLabel>> _labelList;

        std::unique_ptr<PoolAllocator> _tessPoolAllocator;
    };
} }

#endif
