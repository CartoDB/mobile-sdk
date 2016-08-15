/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_DOUGLASPEUCKERGEOMETRYSIMPLIFIER_H_
#define _CARTO_DOUGLASPEUCKERGEOMETRYSIMPLIFIER_H_

#include "geometry/GeometrySimplifier.h"

#include <vector>

namespace carto {
    class MapPos;

    /**
     * An implementation of Ramer-Douglas-Peucker algorithm for geometry simplification.
     * Simplifier works on lines and polygons.
     * Simplification is done in two passes - first pass uses fast Radial Distance vertex rejection,
     * second pass uses Ramer-Douglas-Peuckerworst algorithm (with worst case quadratic complexity).
     */
    class DouglasPeuckerGeometrySimplifier : public GeometrySimplifier {
    public:
        /**
         * Constructs a new simplifier, given tolerance.
         * @param tolerance The maximum error for simplification. The tolerance value gives maximum error in pixels.
         */
        explicit DouglasPeuckerGeometrySimplifier(float tolerance);

        virtual std::shared_ptr<Geometry> simplify(const std::shared_ptr<Geometry>& geometry, float scale) const;

    private:
        std::vector<MapPos> simplifyRing(const std::vector<MapPos>& ring, float scale) const;
        std::vector<MapPos> simplifyRingRD(const std::vector<MapPos>& ring, float scale) const;
        std::vector<MapPos> simplifyRingDP(const std::vector<MapPos>& ring, float scale) const;

        const float _tolerance;
    };
}

#endif
