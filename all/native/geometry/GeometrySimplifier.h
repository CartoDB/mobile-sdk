/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOMETRYSIMPLIFIER_H_
#define _CARTO_GEOMETRYSIMPLIFIER_H_

#include <memory>
#include <string>

namespace carto {
    class Geometry;
    class Projection;
    class ProjectionSurface;

    /**
     * Base class for geometry simplifiers.
     */
    class GeometrySimplifier {
    public:
        virtual ~GeometrySimplifier() { }

        /**
         * Perform the simplification of the given geometry, given relative scale.
         * @param geometry The geometry to simplify.
         * @param projection The projection used for the geometry.
         * @param scale Relative scale for simplification, this is proportional to 2^zoom.
         * @return The simplified geometry. If simplification failed, original geometry may be returned. 
         *         Null pointer may be returned if geometry should be discarded.
         */
        virtual std::shared_ptr<Geometry> simplify(const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<Projection>& projection, const std::shared_ptr<ProjectionSurface>& projectionSurface, float scale) const = 0;
        
    protected:
        GeometrySimplifier() { }
    };
}

#endif
