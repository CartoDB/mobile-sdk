#include "SphericalProjectionSurface.h"
#include "core/MapBounds.h"
#include "projections/Projection.h"
#include "utils/Const.h"

namespace carto {
    
    SphericalProjectionSurface::SphericalProjectionSurface() {
    }

    MapPos SphericalProjectionSurface::calculateMapPos(const cglib::vec3<double>& pos) const {
        return SphericalToInternal(pos * (1.0 / SPHERE_SIZE));
    }

    MapVec SphericalProjectionSurface::calculateMapVec(const cglib::vec3<double>& pos, const cglib::vec3<double>& vec) const {
        cglib::vec3<double> xyzPos = pos * (1.0 / SPHERE_SIZE);
        cglib::mat3x3<double> xyzFrame = LocalFrame(xyzPos);
        double scale = std::sqrt(xyzPos(0) * xyzPos(0) + xyzPos(1) * xyzPos(1)) / Const::PI;
        cglib::vec3<double> mapVec = cglib::transform(vec, cglib::transpose(xyzFrame));
        return MapVec(mapVec(0) * scale, mapVec(1) * scale, mapVec(2) * scale);
    }

    double SphericalProjectionSurface::calculateMapDistance(const cglib::vec3<double> pos0, const cglib::vec3<double>& pos1) const {
        double dot = cglib::dot_product(cglib::unit(pos0), cglib::unit(pos1));
        double angle = std::acos(std::min(1.0, std::max(-1.0, dot)));
        if (angle < PLANAR_APPROX_ANGLE) {
            cglib::vec3<double> delta = cglib::unit(pos0) - cglib::unit(pos1);
            return cglib::length(delta) * Const::EARTH_RADIUS;
        }
        return angle * Const::EARTH_RADIUS;
    }

    cglib::vec3<double> SphericalProjectionSurface::calculatePosition(const MapPos& mapPos) const {
        return InternalToSpherical(mapPos) * SPHERE_SIZE;
    }

    cglib::vec3<double> SphericalProjectionSurface::calculateNormal(const MapPos& mapPos) const {
        return InternalToSpherical(mapPos);
    }

    cglib::vec3<double> SphericalProjectionSurface::calculateVector(const MapPos& mapPos, const MapVec& mapVec) const {
        cglib::vec3<double> xyzPos = InternalToSpherical(mapPos);
        cglib::mat3x3<double> xyzFrame = LocalFrame(xyzPos);
        double scale = Const::PI / std::sqrt(xyzPos(0) * xyzPos(0) + xyzPos(1) * xyzPos(1));
        return cglib::transform(cglib::vec3<double>(mapVec.getX() * scale, mapVec.getY() * scale, mapVec.getZ() * scale), xyzFrame);
    }

    void SphericalProjectionSurface::tesselateSegment(const MapPos& mapPos0, const MapPos& mapPos1, std::vector<MapPos>& mapPoses) const {
        double t = 0.5;
        if (SplitSegment(mapPos0, mapPos1, t)) {
            MapPos mapPosM = mapPos0 + (mapPos1 - mapPos0) * t;
            tesselateSegment(mapPos0, mapPosM, mapPoses);
            tesselateSegment(mapPosM, mapPos1, mapPoses);
            return;
        }

        mapPoses.push_back(mapPos0);
        mapPoses.push_back(mapPos1);
    }

    void SphericalProjectionSurface::tesselateTriangle(unsigned int i0, unsigned int i1, unsigned int i2, std::vector<unsigned int>& indices, std::vector<MapPos>& mapPoses) const {
        const MapPos& mapPos0 = mapPoses.at(i0);
        const MapPos& mapPos1 = mapPoses.at(i1);
        const MapPos& mapPos2 = mapPoses.at(i2);
        
        double t = 0.5;
        if (SplitSegment(mapPos0, mapPos1, t)) {
            MapPos mapPosM = mapPos0 + (mapPos1 - mapPos0) * t;
            unsigned int iM = static_cast<int>(mapPoses.size());
            mapPoses.push_back(mapPosM);
            tesselateTriangle(i0, iM, i2, indices, mapPoses);
            tesselateTriangle(iM, i1, i2, indices, mapPoses);
            return;
        } else if (SplitSegment(mapPos0, mapPos2, t)) {
            MapPos mapPosM = mapPos0 + (mapPos2 - mapPos0) * t;
            unsigned int iM = static_cast<int>(mapPoses.size());
            mapPoses.push_back(mapPosM);
            tesselateTriangle(i0, i1, iM, indices, mapPoses);
            tesselateTriangle(iM, i1, i2, indices, mapPoses);
            return;
        } else if (SplitSegment(mapPos1, mapPos2, t)) {
            MapPos mapPosM = mapPos1 + (mapPos2 - mapPos1) * t;
            unsigned int iM = static_cast<int>(mapPoses.size());
            mapPoses.push_back(mapPosM);
            tesselateTriangle(i0, i1, iM, indices, mapPoses);
            tesselateTriangle(iM, i2, i0, indices, mapPoses);
            return;
        }

        indices.push_back(i0);
        indices.push_back(i1);
        indices.push_back(i2);
    }

    cglib::vec3<double> SphericalProjectionSurface::calculateNearestPoint(const cglib::vec3<double>& pos, double height) const {
        return cglib::unit(pos) * (SPHERE_SIZE + height);
    }
    
    cglib::vec3<double> SphericalProjectionSurface::calculateNearestPoint(const cglib::ray3<double>& ray, double height, double& t) const {
        if (calculateHitPoint(ray, height, t)) {
            return ray(t);
        }
        t = -cglib::dot_product(ray.origin, ray.direction) / cglib::norm(ray.direction);
        return cglib::unit(ray(t)) * (SPHERE_SIZE + height);
    }

    bool SphericalProjectionSurface::calculateHitPoint(const cglib::ray3<double>& ray, double height, double& t) const {
        double a = cglib::norm(ray.direction);
        double b = 2.0 * cglib::dot_product(ray.direction, ray.origin);
        double c = cglib::norm(ray.origin) - (SPHERE_SIZE + height) * (SPHERE_SIZE + height);
        double d = b * b - 4.0 * a * c;
        if (d < 0.0) {
            return false;
        }
        t = (-b - std::sqrt(d)) / (2.0 * a);
        return true;
    }

    cglib::mat4x4<double> SphericalProjectionSurface::calculateLocalMatrix(const MapPos& mapPos, const Projection& projection) const {
        // TODO: check
        const MapBounds& bounds = projection.getBounds();
        const MapVec& boundsDelta = bounds.getDelta();
        double scaleX = Const::WORLD_SIZE / boundsDelta.getX();
        double scaleY = Const::WORLD_SIZE / boundsDelta.getY();
        double scaleZ = std::min(scaleX, scaleY); // TODO: projection should supply this
        double localScale = projection.getLocalScale(mapPos);
        cglib::vec3<double> pos = calculatePosition(projection.toInternal(mapPos));
        cglib::mat4x4<double> localMat(cglib::mat4x4<double>::identity());
        localMat(0, 0) = scaleX * localScale;
        localMat(1, 1) = scaleY * localScale;
        localMat(2, 2) = scaleZ * localScale;
        localMat(0, 3) = pos(0);
        localMat(1, 3) = pos(1);
        localMat(2, 3) = pos(2);
        return localMat;
    }

    cglib::mat4x4<double> SphericalProjectionSurface::calculateTranslateMatrix(const cglib::vec3<double>& pos0, const cglib::vec3<double>& pos1, double t) const {
        double dot = cglib::dot_product(cglib::unit(pos0), cglib::unit(pos1));
        double angle = std::acos(std::min(1.0, std::max(-1.0, dot)));
        cglib::vec3<double> axis = cglib::vector_product(cglib::unit(pos0), cglib::unit(pos1));
        if (cglib::length(axis) > 0) {
            if (angle < PLANAR_APPROX_ANGLE) { // less than 10m
                return cglib::translate4_matrix((cglib::unit(pos1) - cglib::unit(pos0)) * (SPHERE_SIZE * t));
            }
            return cglib::rotate4_matrix(axis, angle * t);
        }
        return cglib::mat4x4<double>::identity();
    }

    bool SphericalProjectionSurface::SplitSegment(const MapPos& mapPos0, const MapPos& mapPos1, double& t) {
        // TODO: use better metric once internal coordinate system is updated
        double dist = (mapPos1 - mapPos0).length();
        if (dist < SEGMENT_SPLIT_THRESHOLD) {
            return false;
        }

        t = 0.5;
        return true;
    }

    MapPos SphericalProjectionSurface::SphericalToInternal(const cglib::vec3<double>& pos) {
        double scale = Const::WORLD_SIZE / (2 * Const::PI);
        double length = cglib::length(pos);
        double x1 = pos(0) != 0 || pos(1) != 0 ? std::atan2(pos(1), pos(0)) : 0;
        double y1 = std::atanh(std::max(-1.0, std::min(1.0, pos(2) / length)));
        double z1 = length - 1;
        return MapPos(x1 * scale, y1 * scale, z1 * Const::EARTH_RADIUS);
    }

    cglib::vec3<double> SphericalProjectionSurface::InternalToSpherical(const MapPos& mapPos) {
        double scale = 2 * Const::PI / Const::WORLD_SIZE;
        double x1 = mapPos.getX() * scale;
        double y1 = mapPos.getY() * scale;
        double rz = std::tanh(y1);
        double ss = std::sqrt(std::max(0.0, 1.0 - rz * rz));
        double rx = ss * std::cos(x1);
        double ry = ss * std::sin(x1);
        return cglib::vec3<double>(rx, ry, rz);
    }

    cglib::mat3x3<double> SphericalProjectionSurface::LocalFrame(const cglib::vec3<double>& pos) {
        double x = pos(0);
        double y = pos(1);
        double z = pos(2);
        
        double dx_du = -y;
        double dy_du = x;
        double dz_du = 0;

        double dx_dv = -z * x;
        double dy_dv = -z * y;
        double dz_dv = x * x + y * y;

        double dx_dh = x;
        double dy_dh = y;
        double dz_dh = z;

        return cglib::mat3x3<double> { { dx_du, dx_dv, dx_dh }, { dy_du, dy_dv, dy_dh }, { dz_du, dz_dv, dz_dh } };
    }

    const double SphericalProjectionSurface::SPHERE_SIZE = Const::WORLD_SIZE / Const::PI;

    const double SphericalProjectionSurface::PLANAR_APPROX_ANGLE = 1.0e-6;

    const double SphericalProjectionSurface::SEGMENT_SPLIT_THRESHOLD = Const::WORLD_SIZE / 1000.0;
    
}
