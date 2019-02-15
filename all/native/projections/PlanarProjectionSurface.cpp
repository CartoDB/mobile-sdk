#include "PlanarProjectionSurface.h"
#include "core/MapBounds.h"
#include "projections/Projection.h"
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

    cglib::mat4x4<double> PlanarProjectionSurface::calculateLocalMatrix(const MapPos& mapPos, const Projection& projection) const {
        const MapBounds& bounds = projection.getBounds();
        const MapVec& boundsDelta = bounds.getDelta();
        double scaleX = Const::WORLD_SIZE / boundsDelta.getX();
        double scaleY = Const::WORLD_SIZE / boundsDelta.getY();
        double scaleZ = std::min(scaleX, scaleY); // TODO: projection should supply this
        double localScale = projection.getLocalScale(mapPos);
        MapPos mapPosInternal = projection.toInternal(mapPos);
        cglib::mat4x4<double> localMat(cglib::mat4x4<double>::identity());
        localMat(0, 0) = scaleX * localScale;
        localMat(1, 1) = scaleY * localScale;
        localMat(2, 2) = scaleZ * localScale;
        localMat(0, 3) = mapPosInternal.getX();
        localMat(1, 3) = mapPosInternal.getY();
        localMat(2, 3) = mapPosInternal.getZ();
        return localMat;
    }

    cglib::mat4x4<double> PlanarProjectionSurface::calculateTranslateMatrix(const cglib::vec3<double>& pos0, const cglib::vec3<double>& pos1, double t) const {
        return cglib::translate4_matrix((pos1 - pos0) * t);
    }
    
}
