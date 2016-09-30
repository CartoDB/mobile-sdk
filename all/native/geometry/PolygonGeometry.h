/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_POLYGONGEOMETRY_H_
#define _CARTO_POLYGONGEOMETRY_H_

#include "geometry/Geometry.h"

#include <vector>

namespace carto {

    /**
     * Polygon geometry defined by an outer ring and optional multiple inner rings (holes).
     */
    class PolygonGeometry : public Geometry {
    public:
        /**
         * Constructs a PolygonGeometry objects from an outer ring.
         * @param poses The list of map positions defining the outer ring.
         */
        explicit PolygonGeometry(const std::vector<MapPos>& poses);
        /**
         * Constructs a PolygonGeometry objects from an outer ring and list of inner rings (holes).
         * @param poses The list of map positions defining the outer ring.
         * @param holes The list of map position lists defining the inner rings.
         */
        PolygonGeometry(const std::vector<MapPos>& poses, const std::vector<std::vector<MapPos> >& holes);
        /**
         * Constructs a PolygonGeometry objects from a list of rings.
         * It is assumed the the first ring is outer ring and all other rings are inner rings.
         * @param rings The list of map position lists defining the rings
         */
        explicit PolygonGeometry(const std::vector<std::vector<MapPos> >& rings);
        virtual ~PolygonGeometry();
        
        virtual MapPos getCenterPos() const;
    
        /**
         * Returns the list of map positions defining the outer ring of the polygon.
         * @returns The list of map positions defining the outer ring of the polygon.
         */
        const std::vector<MapPos>& getPoses() const;
    
        /**
         * Returns the list of map position lists defining the inner rings of the polygon (holes).
         * @returns The list of map position lists defining the inner rings of the polygon (holes).
         */
        std::vector<std::vector<MapPos> > getHoles() const;
    
        /**
         * Returns the list of map position lists defining the rings of the polygon.
         * @returns The list of map position lists defining the rings of the polygon.
         */
        const std::vector<std::vector<MapPos> >& getRings() const;

    private:
        std::vector<std::vector<MapPos> > _rings;
    };
    
}

#endif
