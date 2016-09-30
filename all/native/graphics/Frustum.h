/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_FRUSTUM_H_
#define _CARTO_FRUSTUM_H_

#include <array>

#include <cglib/mat.h>

namespace carto {
    class MapBounds;
    class MapPos;
    
    /**
     * A 3D frustum defined by 6 clipping planes.
     */
    class Frustum {
    public:
        /**
         * Constructs an empty frustum where all corners coordinate are 0.
         */
        Frustum();
        /**
         * Constructs a frustum from a modelview projection matrix.
         * @param mvpMatrix The modelview projection matrix.
         */
        explicit Frustum(const cglib::mat4x4<double>& mvpMatrix);
    
        /**
         * Tests if a point is inside with the frustum.
         * @param point The point to be tested.
         * @return True if the point lies within the frustum.
         */
        bool pointInside(const MapPos& point) const;
    
        /**
         * Tests if a circle on the z = 0 plane inersects with the frustum.
         * @param center The center position of the circle.
         * @param radius The radius of the circle.
         * @return True if the circle intersects with the frustum.
         */
        bool circleIntersects(const MapPos& center, double radius) const;
        /**
         * Tests if a sphere inersects with the frustum.
         * @param center The center position of the sphere.
         * @param radius The radius of the sphere.
         * @return True if the sphere intersects with the frustum.
         */
        bool sphereIntersects(const MapPos& center, double radius) const;
    
        /**
         * Tests if an axis aligned square on the z = 0 plane inersects with the frustum.
         * @param square The axis aligned square.
         * @return True if the square intersects with the frustum.
         */
        bool squareIntersects(const MapBounds& square) const;
        /**
         * Tests if an axis aligned cuboid inersects with the frustum.
         * @param cuboid The axis aligned cuboid.
         * @return True if the cuboid intersects with the frustum.
         */
        bool cuboidIntersects(const MapBounds& cuboid) const;
    
    private:
        void normalize();
    
        // Right, left, bottom, top, far, near
        std::array<std::array<double, 4>, 6> _planes;
    };
    
}

#endif
