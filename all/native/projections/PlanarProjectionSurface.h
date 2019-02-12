/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_PLANARPROJECTIONSURFACE_H_
#define _CARTO_PLANARPROJECTIONSURFACE_H_

#include "projections/ProjectionSurface.h"

#include <cglib/vec.h>
#include <cglib/mat.h>
#include <cglib/ray.h>

namespace carto {
    
    class PlanarProjectionSurface : public ProjectionSurface {
    public:
        PlanarProjectionSurface();
        
        virtual MapPos calculateMapPos(const cglib::vec3<double>& pos) const;
        virtual MapVec calculateMapVec(const cglib::vec3<double>& pos, const cglib::vec3<double>& vec) const;
        virtual double calculateMapDistance(const cglib::vec3<double> pos0, const cglib::vec3<double>& pos1) const;

        virtual cglib::vec3<double> calculatePosition(const MapPos& mapPos) const;
        virtual cglib::vec3<double> calculateNormal(const MapPos& mapPos) const;
        virtual cglib::vec3<double> calculateVector(const MapPos& mapPos, const MapVec& mapVec) const;

        virtual double calculateRayHit(const cglib::ray3<double>& ray) const;

        virtual cglib::mat4x4<double> calculateLocalMatrix(const MapPos& mapPos, const Projection& projection) const;

        virtual cglib::mat4x4<double> calculateTranslateMatrix(const cglib::vec3<double>& pos0, const cglib::vec3<double>& pos1, double t) const;
    };
    
}

#endif
