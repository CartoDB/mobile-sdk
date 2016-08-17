/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VT_TILEGEOMETRY_H_
#define _CARTO_VT_TILEGEOMETRY_H_

#include "Bitmap.h"
#include "Color.h"
#include "StrokeMap.h"
#include "VertexArray.h"
#include "Styles.h"

#include <memory>
#include <array>
#include <vector>

#include <boost/optional.hpp>

#include <cglib/mat.h>

namespace carto { namespace vt {
    class TileGeometry {
    public:
        enum class Type {
            NONE, POINT, LINE, POLYGON, POLYGON3D
        };

        struct StyleParameters {
            enum { MAX_PARAMETERS = 16 };
            int parameterCount;
            std::array<std::shared_ptr<const ColorFunction>, MAX_PARAMETERS> colorTable;
            std::array<std::shared_ptr<const FloatFunction>, MAX_PARAMETERS> opacityTable;
            std::array<std::shared_ptr<const FloatFunction>, MAX_PARAMETERS> widthTable;
            std::shared_ptr<const BitmapPattern> pattern;
            boost::optional<cglib::mat3x3<float>> transform;
            CompOp compOp;
            PointOrientation pointOrientation;

            StyleParameters() : parameterCount(0), colorTable(), opacityTable(), widthTable(), pattern(), transform(), compOp(CompOp::SRC_OVER), pointOrientation(PointOrientation::POINT) { }
        };

        struct GeometryLayoutParameters {
            int vertexSize;
            int vertexOffset;
            int attribsOffset;
            int texCoordOffset;
            int binormalOffset;
            int heightOffset;
            float vertexScale;
            float texCoordScale;
            float binormalScale;

            GeometryLayoutParameters() : vertexSize(0), vertexOffset(-1), attribsOffset(-1), texCoordOffset(-1), binormalOffset(-1), heightOffset(-1), vertexScale(0), texCoordScale(0), binormalScale(0) { }
        };

        explicit TileGeometry(Type type, float tileSize, float geomScale, const StyleParameters& styleParameters, const GeometryLayoutParameters& geometryLayoutParameters, unsigned int indicesCount, VertexArray<unsigned char> vertexGeometry, VertexArray<unsigned short> indices) : _type(type), _tileSize(tileSize), _geomScale(geomScale), _styleParameters(styleParameters), _geometryLayoutParameters(geometryLayoutParameters), _indicesCount(indicesCount), _vertexGeometry(std::move(vertexGeometry)), _indices(std::move(indices)) { }

        Type getType() const { return _type; }
        float getTileSize() const { return _tileSize; }
        float getGeometryScale() const { return _geomScale; }
        const StyleParameters& getStyleParameters() const { return _styleParameters; }
        const GeometryLayoutParameters& getGeometryLayoutParameters() const { return _geometryLayoutParameters; }
        unsigned int getIndicesCount() const { return _indicesCount; }

        const VertexArray<unsigned char>& getVertexGeometry() const { return _vertexGeometry; }
        const VertexArray<unsigned short>& getIndices() const { return _indices; }

        void releaseVertexArrays() {
            _vertexGeometry.clear();
            _vertexGeometry.shrink_to_fit();
            _indices.clear();
            _indices.shrink_to_fit();
        }

        std::size_t getResidentSize() const {
            return 16 + _vertexGeometry.size() * sizeof(unsigned char) + _indices.size() * sizeof(unsigned short);
        }

    private:
        Type _type;
        float _tileSize;
        float _geomScale;
        StyleParameters _styleParameters;
        GeometryLayoutParameters _geometryLayoutParameters;
        unsigned int _indicesCount;

        VertexArray<unsigned char> _vertexGeometry;
        VertexArray<unsigned short> _indices;
    };
} }

#endif
