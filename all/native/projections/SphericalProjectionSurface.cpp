#include "SphericalProjectionSurface.h"
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
        // TODO: make it faster, non-recursive
        MapPos mapPosM;
        if (SplitSegment(mapPos0, mapPos1, mapPosM)) {
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
        
        // TODO: make it faster, non-recursive
        MapPos mapPosM;
        if (SplitSegment(mapPos0, mapPos1, mapPosM)) {
            unsigned int iM = static_cast<int>(mapPoses.size());
            mapPoses.push_back(mapPosM);
            tesselateTriangle(i2, i0, iM, indices, mapPoses);
            tesselateTriangle(i1, i2, iM, indices, mapPoses);
            return;
        } else if (SplitSegment(mapPos0, mapPos2, mapPosM)) {
            unsigned int iM = static_cast<int>(mapPoses.size());
            mapPoses.push_back(mapPosM);
            tesselateTriangle(i0, i1, iM, indices, mapPoses);
            tesselateTriangle(i1, i2, iM, indices, mapPoses);
            return;
        } else if (SplitSegment(mapPos1, mapPos2, mapPosM)) {
            unsigned int iM = static_cast<int>(mapPoses.size());
            mapPoses.push_back(mapPosM);
            tesselateTriangle(i0, i1, iM, indices, mapPoses);
            tesselateTriangle(i2, i0, iM, indices, mapPoses);
            return;
        }

        indices.push_back(i0);
        indices.push_back(i1);
        indices.push_back(i2);
    }

    double SphericalProjectionSurface::calculateDistance(const cglib::vec3<double> pos0, const cglib::vec3<double>& pos1) const {
        double dot = cglib::dot_product(cglib::unit(pos0), cglib::unit(pos1));
        double angle = std::acos(std::min(1.0, std::max(-1.0, dot)));
        if (angle < PLANAR_APPROX_ANGLE) {
            cglib::vec3<double> delta = cglib::unit(pos0) - cglib::unit(pos1);
            return cglib::length(delta) * Const::WORLD_SIZE;
        }
        return angle * Const::WORLD_SIZE / Const::PI;
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

    cglib::mat4x4<double> SphericalProjectionSurface::calculateLocalFrameMatrix(const cglib::vec3<double>& pos) const {
        cglib::mat3x3<double> localFrameMat3 = LocalFrame(pos * (1.0 / SPHERE_SIZE));
        cglib::mat4x4<double> localFrameMat4 = cglib::mat4x4<double>::identity();
        for (int i = 0; i < 3; i++) {
            cglib::vec3<double> dir = cglib::unit(cglib::col_vector(localFrameMat3, i));
            for (int j = 0; j < 3; j++) {
                localFrameMat4(j, i) = dir(j) * SPHERE_SIZE / Const::EARTH_RADIUS;
            }
            localFrameMat4(i, 3) = pos(i);
        }
        return localFrameMat4;
    }

    cglib::mat4x4<double> SphericalProjectionSurface::calculateTranslateMatrix(const cglib::vec3<double>& pos0, const cglib::vec3<double>& pos1, double t) const {
        double scale = (1 - t) + (cglib::length(pos1) / cglib::length(pos0)) * t;
        cglib::mat4x4<double> scaleMat = cglib::scale4_matrix(cglib::vec3<double>(scale, scale, scale));

        double dot = cglib::dot_product(cglib::unit(pos0), cglib::unit(pos1));
        double angle = std::acos(std::min(1.0, std::max(-1.0, dot)));
        cglib::vec3<double> axis = cglib::vector_product(cglib::unit(pos0), cglib::unit(pos1));
        if (cglib::length(axis) > 0) {
            if (angle < PLANAR_APPROX_ANGLE) { // less than 10m
                return cglib::translate4_matrix((cglib::unit(pos1) - cglib::unit(pos0)) * (SPHERE_SIZE * t)) * scaleMat;
            }
            return cglib::rotate4_matrix(axis, angle * t) * scaleMat;
        }
        return scaleMat;
    }

    bool SphericalProjectionSurface::SplitSegment(const MapPos& mapPos0, const MapPos& mapPos1, MapPos& mapPosM) {
        cglib::vec3<double> pos0 = cglib::unit(InternalToSpherical(mapPos0));
        cglib::vec3<double> pos1 = cglib::unit(InternalToSpherical(mapPos1));
        double dot = cglib::dot_product(pos0, pos1);
        if (dot <= -1) {
            return false; // exactly on the opposite side, no way to split
        }
        double angle = std::acos(std::min(1.0, std::max(-1.0, dot)));
        if (angle * Const::EARTH_RADIUS < SEGMENT_SPLIT_THRESHOLD) {
            return false;
        }
        mapPosM = SphericalToInternal(cglib::unit(pos0 + pos1));
        return true;
    }

    MapPos SphericalProjectionSurface::SphericalToInternal(const cglib::vec3<double>& pos) {
        double scale = Const::WORLD_SIZE / (2 * Const::PI);
        double len = cglib::length(pos);
        double rz = pos(2) / len;
        double ss = std::sqrt(std::max(0.0, 1.0 - rz * rz));
        double x1 = (pos(0) == 0.0 && pos(1) == 0.0 ? 0.0 : std::atan2(pos(1), pos(0)));
        double y1 = (std::isnan(rz) ? rz : std::atanh(std::max(-1.0, std::min(1.0, rz))));
        double z1 = (len == 1.0 && ss == 0.0 ? 0.0 : (len - 1.0) / ss);
        return MapPos(x1 * scale, y1 * scale, z1 * scale);
    }

    cglib::vec3<double> SphericalProjectionSurface::InternalToSpherical(const MapPos& mapPos) {
        double scale = 2 * Const::PI / Const::WORLD_SIZE;
        double x1 = mapPos.getX() * scale;
        double y1 = mapPos.getY() * scale;
        double z1 = mapPos.getZ() * scale;
        double rz = std::tanh(y1);
        double ss = std::sqrt(std::max(0.0, 1.0 - rz * rz));
        double rx = ss * std::cos(x1);
        double ry = ss * std::sin(x1);
        double len = 1.0 + z1 * ss;
        return cglib::vec3<double>(rx, ry, rz) * len;
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

    const double SphericalProjectionSurface::SEGMENT_SPLIT_THRESHOLD = Const::EARTH_CIRCUMFERENCE / 120.0;
    
}
