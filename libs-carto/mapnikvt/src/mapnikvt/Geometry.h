/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_MAPNIKVT_GEOMETRY_H_
#define _CARTO_MAPNIKVT_GEOMETRY_H_

#include <memory>
#include <list>
#include <vector>

#include <cglib/vec.h>

namespace carto { namespace mvt {
    class Geometry {
    public:
        virtual ~Geometry() = default;
    };

    class PointGeometry : public Geometry {
    public:
        using Vertices = std::vector<cglib::vec2<float>>;

        explicit PointGeometry(Vertices vertices) : _vertices(std::move(vertices)) { }

        const Vertices& getVertices() const { return _vertices; }

    private:
        Vertices _vertices;
    };

    class LineGeometry : public Geometry {
    public:
        using Vertices = std::vector<cglib::vec2<float>>;
        using VerticesList = std::vector<Vertices>;

        explicit LineGeometry(VerticesList verticesList) : _verticesList(std::move(verticesList)) { }
        
        const VerticesList& getVerticesList() const { return _verticesList; }
        
        Vertices getMidPoints() const;

    private:
        VerticesList _verticesList;
    };

    class PolygonGeometry : public Geometry {
    public:
        using Vertices = std::vector<cglib::vec2<float>>;
        using VerticesList = std::vector<Vertices>;
        using PolygonList = std::vector<VerticesList>;

        explicit PolygonGeometry(PolygonList polygonList) : _polygonList(std::move(polygonList)) { }
        
        const PolygonList& getPolygonList() const { return _polygonList; }

        Vertices getCenterPoints() const;
        Vertices getSurfacePoints() const;

    private:
        PolygonList _polygonList;
    };
} }

#endif
