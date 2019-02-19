#include "PlanarProjectionSurface.h"
#include "utils/Const.h"

namespace carto {
    
    PlanarProjectionSurface::PlanarProjectionSurface() {
    }

    MapPos PlanarProjectionSurface::calculateMapPos(const cglib::vec3<double>& pos) const {
        return MapPos(pos(0), pos(1), pos(2));
    }

    MapVec PlanarProjectionSurface::calculateMapVec(const cglib::vec3<double>& pos, const cglib::vec3<double>& vec) const {
        return MapVec(vec(0), vec(1), vec(2));
    }

    double PlanarProjectionSurface::calculateMapDistance(const cglib::vec3<double> pos0, const cglib::vec3<double>& pos1) const {
        return cglib::length(pos1 - pos0) / Const::WORLD_SIZE * Const::EARTH_CIRCUMFERENCE;
    }

    cglib::vec3<double> PlanarProjectionSurface::calculatePosition(const MapPos& mapPos) const {
        return cglib::vec3<double>(mapPos.getX(), mapPos.getY(), mapPos.getZ());
    }

    cglib::vec3<double> PlanarProjectionSurface::calculateNormal(const MapPos& mapPos) const {
        return cglib::vec3<double>(0, 0, 1);
    }

    cglib::vec3<double> PlanarProjectionSurface::calculateVector(const MapPos& mapPos, const MapVec& mapVec) const {
        return cglib::vec3<double>(mapVec.getX(), mapVec.getY(), mapVec.getZ());
    }

    void PlanarProjectionSurface::tesselateSegment(const MapPos& mapPos0, const MapPos& mapPos1, std::vector<MapPos>& mapPoses) const {
        mapPoses.push_back(mapPos0);
        mapPoses.push_back(mapPos1);
    }

    void PlanarProjectionSurface::tesselateTriangle(unsigned int i0, unsigned int i1, unsigned int i2, std::vector<unsigned int>& indices, std::vector<MapPos>& mapPoses) const {
        indices.push_back(i0);
        indices.push_back(i1);
        indices.push_back(i2);
    }

    cglib::vec3<double> PlanarProjectionSurface::calculateNearestPoint(const cglib::vec3<double>& pos, double height) const {
        return cglib::vec3<double>(pos(0), pos(1), height);
    }
    
    cglib::vec3<double> PlanarProjectionSurface::calculateNearestPoint(const cglib::ray3<double>& ray, double height, double& t) const {
        if (calculateHitPoint(ray, height, t)) {
            return ray(t);
        }
        return calculateNearestPoint(ray.origin, height);
    }

    bool PlanarProjectionSurface::calculateHitPoint(const cglib::ray3<double>& ray, double height, double& t) const {
        t = (height - ray.origin(2)) / ray.direction(2);
        return true;
    }

    cglib::mat4x4<double> PlanarProjectionSurface::calculateLocalFrameMatrix(const cglib::vec3<double>& pos) const {
        double sin = std::tanh(pos(1) * 2 * Const::PI / Const::WORLD_SIZE);
        double cos = std::sqrt(std::max(0.0, 1.0 - sin * sin));
        double scale = Const::WORLD_SIZE / Const::EARTH_CIRCUMFERENCE / cos;
        cglib::mat4x4<double> localFrameMat = cglib::mat4x4<double>::identity();
        for (int i = 0; i < 3; i++) {
            localFrameMat(i, i) = scale;
            localFrameMat(i, 3) = pos(i);
        }
        return localFrameMat;
    }

    cglib::mat4x4<double> PlanarProjectionSurface::calculateTranslateMatrix(const cglib::vec3<double>& pos0, const cglib::vec3<double>& pos1, double t) const {
        return cglib::translate4_matrix((pos1 - pos0) * t);
    }
    
}
