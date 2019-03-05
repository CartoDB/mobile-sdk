/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_PROJECTIONSURFACE_H_
#define _CARTO_PROJECTIONSURFACE_H_

#include "core/MapPos.h"
#include "core/MapVec.h"

#include <vector>

#include <cglib/vec.h>
#include <cglib/mat.h>
#include <cglib/ray.h>

namespace carto {

    class ProjectionSurface {
    public:
        virtual ~ProjectionSurface() { }

        virtual MapPos calculateMapPos(const cglib::vec3<double>& pos) const = 0;
        virtual MapVec calculateMapVec(const cglib::vec3<double>& pos, const cglib::vec3<double>& vec) const = 0;

        virtual cglib::vec3<double> calculatePosition(const MapPos& mapPos) const = 0;
        virtual cglib::vec3<double> calculateNormal(const MapPos& mapPos) const = 0;
        virtual cglib::vec3<double> calculateVector(const MapPos& mapPos, const MapVec& mapVec) const = 0;

        virtual void tesselateSegment(const MapPos& mapPos0, const MapPos& mapPos1, std::vector<MapPos>& mapPoses) const = 0;
        virtual void tesselateTriangle(unsigned int i0, unsigned int i1, unsigned int i2, std::vector<unsigned int>& indices, std::vector<MapPos>& mapPoses) const = 0;

        virtual double calculateDistance(const cglib::vec3<double> pos0, const cglib::vec3<double>& pos1) const = 0;
        virtual cglib::vec3<double> calculateNearestPoint(const cglib::vec3<double>& pos, double height) const = 0;
        virtual cglib::vec3<double> calculateNearestPoint(const cglib::ray3<double>& ray, double height, double& t) const = 0;
        virtual bool calculateHitPoint(const cglib::ray3<double>& ray, double height, double& t) const = 0;

        virtual cglib::mat4x4<double> calculateLocalFrameMatrix(const cglib::vec3<double>& pos) const = 0;
        virtual cglib::mat4x4<double> calculateTranslateMatrix(const cglib::vec3<double>& pos0, const cglib::vec3<double>& pos1, double t) const = 0;
    };
    
}

#endif
