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

#include <boost/variant.hpp>

#include <cglib/vec.h>
#include <cglib/mat.h>
#include <cglib/bbox.h>

namespace carto { namespace vt {
    class TileLayerBuilder {
    public:
        using Vertex = cglib::vec2<float>;
        using Vertices = std::vector<Vertex>;
        using VerticesList = std::vector<Vertices>;

        explicit TileLayerBuilder(float tileSize, float geomScale);

        void addBitmap(const std::shared_ptr<TileBitmap>& bitmap);
        void addPoints(const Vertices& vertices, const PointStyle& style);
        void addLines(const VerticesList& verticesList, const LineStyle& style);
        void addPolygons(const std::list<VerticesList>& polygonList, const PolygonStyle& style);
        void addPolygons3D(const std::list<VerticesList>& polygonList, float height, const Polygon3DStyle& style);
        void addBitmapLabel(long long id, long long groupId, const boost::variant<Vertex, Vertices>& position, float minimumGroupDistance, const BitmapLabelStyle& style);
        void addTextLabel(long long id, long long groupId, const std::string& text, const boost::optional<Vertex>& position, const Vertices& vertices, float minimumGroupDistance, const TextLabelStyle& style);

        std::shared_ptr<TileLayer> build(int layerIdx, float opacity, boost::optional<CompOp> compOp);

    private:
        enum { RESERVED_VERTICES = 4096 };

        // TODO: aggregate
        struct StyleBuilderParameters : TileGeometry::StyleParameters {
            TileGeometry::Type type;
            std::array<StrokeMap::StrokeId, MAX_PARAMETERS> lineStrokeIds;
            std::array<Font::CodePoint, MAX_PARAMETERS> pointGlyphIds;
            std::shared_ptr<StrokeMap> strokeMap;
            std::shared_ptr<GlyphMap> glyphMap;

            StyleBuilderParameters() : StyleParameters(), type(TileGeometry::Type::NONE), lineStrokeIds(), pointGlyphIds(), strokeMap(), glyphMap() { }
        };

        void appendGeometry();
        void appendGeometry(float verticesScale, float binormalsScale, float texCoordsScale, const VertexArray<cglib::vec2<float>>& vertices, const VertexArray<cglib::vec2<float>>& texCoords, const VertexArray<cglib::vec2<float>>& binormals, const VertexArray<float>& heights, const VertexArray<cglib::vec4<char>>& attribs, const VertexArray<unsigned int>& indices, std::size_t offset, std::size_t count);
        float calculateScale(VertexArray<cglib::vec2<float>>& values) const;

        bool tesselatePoint(const Vertex& vertex, char styleIndex, const Font::Glyph* glyph, const PointStyle& style);
        bool tesselatePolygon(const VerticesList& verticesList, char styleIndex, const PolygonStyle& style);
        bool tesselatePolygon3D(const VerticesList& verticesList, float height, char styleIndex, const Polygon3DStyle& style);
        bool tesselateLine(const Vertices& points, char styleIndex, const StrokeMap::Stroke* stroke, const LineStyle& style);
        bool tesselateLineEndPoint(const Vertex& p0, float u0, float v0, float v1, int i0, const cglib::vec2<float>& tangent, const cglib::vec2<float>& binormal, char styleIndex, const LineStyle& style);

        float _tileSize;
        float _geomScale;
        StyleBuilderParameters _styleParameters;

        VertexArray<cglib::vec2<float>> _vertices;
        VertexArray<cglib::vec2<float>> _texCoords;
        VertexArray<cglib::vec2<float>> _binormals;
        VertexArray<float> _heights;
        VertexArray<cglib::vec4<char>> _attribs;
        VertexArray<unsigned int> _indices;

        std::vector<std::shared_ptr<TileBitmap>> _bitmapList;
        std::vector<std::shared_ptr<TileGeometry>> _geometryList;
        std::vector<std::shared_ptr<TileLabel>> _labelList;

        std::shared_ptr<PoolAllocator> _tessPoolAllocator;
    };
} }

#endif
