#include "DouglasPeuckerGeometrySimplifier.h"
#include "core/MapPos.h"
#include "core/MapVec.h"
#include "geometry/Geometry.h"
#include "geometry/LineGeometry.h"
#include "geometry/PolygonGeometry.h"
#include "geometry/MultiGeometry.h"
#include "geometry/MultiLineGeometry.h"
#include "geometry/MultiPolygonGeometry.h"
#include "projections/Projection.h"
#include "projections/ProjectionSurface.h"
#include "utils/Const.h"

#include <stack>
#include <utility>
#include <algorithm>

namespace carto {

    class DouglasPeuckerGeometrySimplifier::Helper {
    public:
        Helper(const std::shared_ptr<Projection>& projection, const std::shared_ptr<ProjectionSurface>& projectionSurface) : _projection(projection), _projectionSurface(projectionSurface) {
        }

        void approximate(const MapPos* points, std::size_t pointCount, double minDist, unsigned char* keys) const {
            std::stack<SubPoly> stack;
            stack.push(SubPoly(0, pointCount - 1));
            while (!stack.empty()) {
                SubPoly subPoly = stack.top();
                stack.pop();
                KeyInfo keyInfo = findKey(points, subPoly.first, subPoly.last);
                if (keyInfo.index && minDist < keyInfo.dist) {
                    keys[keyInfo.index] = 1;
                    stack.push(SubPoly(keyInfo.index, subPoly.last));
                    stack.push(SubPoly(subPoly.first, keyInfo.index));
                }
            }
        }

    private:
        struct SubPoly {
            SubPoly(std::size_t first = 0, std::size_t last = 0) : first(first), last(last) { }

            std::size_t first;
            std::size_t last;
        };

        struct KeyInfo {
            KeyInfo(std::size_t index = 0, double dist = 0) : index(index), dist(dist) { }

            std::size_t index;
            double dist;
        };

        static double FindSegmentDistance(const cglib::vec3<double>& s1, const cglib::vec3<double>& s2, const cglib::vec3<double>& p) {
            // NOTE: not really correct on spherical surface but, but should still give reasonable results
            cglib::vec3<double> v = (s2 - s1);
            cglib::vec3<double> w = (p - s1);
            double cw = cglib::dot_product(v, w);
            if (cw <= 0) {
                return cglib::length(p - s1);
            }
            double cv = cglib::norm(v);
            if (cv <= cw) {
                return cglib::length(p - s2);
            }

            double fraction = cv == 0 ? 0 : cw / cv;
            cglib::vec3<double> pProj = s1 + (s2 - s1) * fraction;
            return cglib::length(pProj - p);
        }

        KeyInfo findKey(const MapPos* points, std::size_t first, std::size_t last) const {
            KeyInfo keyInfo;

            cglib::vec3<double> s1 = _projectionSurface->calculatePosition(_projection->toInternal(points[first]));
            cglib::vec3<double> s2 = _projectionSurface->calculatePosition(_projection->toInternal(points[last]));
            for (std::size_t current = first + 1; current < last; current++) {
                cglib::vec3<double> p = _projectionSurface->calculatePosition(_projection->toInternal(points[current]));
                double dist = FindSegmentDistance(s1, s2, p);
                if (dist < keyInfo.dist) {
                    continue;
                }
                keyInfo.index = current;
                keyInfo.dist = dist;
            }
            return keyInfo;
        }

        std::shared_ptr<Projection> _projection;
        std::shared_ptr<ProjectionSurface> _projectionSurface;
    };

    DouglasPeuckerGeometrySimplifier::DouglasPeuckerGeometrySimplifier(float tolerance) :
        GeometrySimplifier(),
        _tolerance(tolerance)
    {
    }

    std::shared_ptr<Geometry> DouglasPeuckerGeometrySimplifier::simplify(const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<Projection>& projection, const std::shared_ptr<ProjectionSurface>& projectionSurface, float scale) const {
        if (auto lineGeometry = std::dynamic_pointer_cast<LineGeometry>(geometry)) {
            std::vector<MapPos> mapPoses = simplifyRing(lineGeometry->getPoses(), projection, projectionSurface, scale);
            if (mapPoses.size() < 2) {
                return std::shared_ptr<Geometry>();
            }
            bool simplified = mapPoses.size() < lineGeometry->getPoses().size();
            if (simplified) {
                return std::make_shared<LineGeometry>(mapPoses);
            }
        } else if (auto polygonGeometry = std::dynamic_pointer_cast<PolygonGeometry>(geometry)) {
            std::vector<MapPos> mapPoses = simplifyRing(polygonGeometry->getPoses(), projection, projectionSurface, scale);
            if (mapPoses.size() < 3) {
                return std::shared_ptr<Geometry>();
            }
            bool simplified = mapPoses.size() < polygonGeometry->getPoses().size();
            std::vector<std::vector<MapPos> > holes;
            for (const std::vector<MapPos>& holeRing : polygonGeometry->getHoles()) {
                std::vector<MapPos> holeMapPoses = simplifyRing(holeRing, projection, projectionSurface, scale);
                if (holeMapPoses.size() < holeRing.size()) {
                    simplified = true;
                }
                if (holes.size() >= 3) {
                    holes.push_back(std::move(holeMapPoses));
                }
            }
            if (simplified) {
                return std::make_shared<PolygonGeometry>(mapPoses, holes);
            }
        } else if (auto multiLineGeometry = std::dynamic_pointer_cast<MultiLineGeometry>(geometry)) {
            std::vector<std::shared_ptr<LineGeometry> > lines;
            bool simplified = false;
            for (int i = 0; i < multiLineGeometry->getGeometryCount(); i++) {
                std::shared_ptr<Geometry> geom = simplify(multiLineGeometry->getGeometry(i), projection, projectionSurface, scale);
                if (geom != multiLineGeometry->getGeometry(i)) {
                    simplified = true;
                }
                if (auto line = std::dynamic_pointer_cast<LineGeometry>(geom)) {
                    lines.push_back(line);
                }
            }
            if (simplified) {
                return std::make_shared<MultiLineGeometry>(lines);
            }
        } else if (auto multiPolygonGeometry = std::dynamic_pointer_cast<MultiPolygonGeometry>(geometry)) {
            std::vector<std::shared_ptr<PolygonGeometry> > polygons;
            bool simplified = false;
            for (int i = 0; i < multiPolygonGeometry->getGeometryCount(); i++) {
                std::shared_ptr<Geometry> geom = simplify(multiPolygonGeometry->getGeometry(i), projection, projectionSurface, scale);
                if (geom != multiPolygonGeometry->getGeometry(i)) {
                    simplified = true;
                }
                if (auto polygon = std::dynamic_pointer_cast<PolygonGeometry>(geom)) {
                    polygons.push_back(polygon);
                }
            }
            if (simplified) {
                return std::make_shared<MultiPolygonGeometry>(polygons);
            }
        } else if (auto multiGeometry = std::dynamic_pointer_cast<MultiGeometry>(geometry)) {
            std::vector<std::shared_ptr<Geometry> > geoms;
            bool simplified = false;
            for (int i = 0; i < multiGeometry->getGeometryCount(); i++) {
                std::shared_ptr<Geometry> geom = simplify(multiGeometry->getGeometry(i), projection, projectionSurface, scale);
                if (geom != multiGeometry->getGeometry(i)) {
                    simplified = true;
                }
                if (geom) {
                    geoms.push_back(geom);
                }
            }
            if (simplified) {
                return std::make_shared<MultiGeometry>(geoms);
            }
        }
        return geometry;
    }

    std::vector<MapPos> DouglasPeuckerGeometrySimplifier::simplifyRing(const std::vector<MapPos>& ring, const std::shared_ptr<Projection>& projection, const std::shared_ptr<ProjectionSurface>& projectionSurface, float scale) const {
        if (ring.size() <= 2) {
            return ring;
        }

        std::vector<MapPos> simplifiedRing1;
        simplifiedRing1.reserve(ring.size());
        simplifiedRing1.push_back(ring.front());
        cglib::vec3<double> pos0 = projectionSurface->calculatePosition(projection->toInternal(simplifiedRing1.back()));
        for (std::size_t i = 1; i + 1 < ring.size(); i++) {
            cglib::vec3<double> pos1 = projectionSurface->calculatePosition(projection->toInternal(ring[i]));
            double dist = cglib::length(pos1 - pos0); // NOTE: not really correct on spherical surface but, but should still give reasonable results
            if (dist > scale * _tolerance) {
                simplifiedRing1.push_back(ring[i]);
                pos0 = pos1;
            }
        }
        simplifiedRing1.push_back(ring.back());

        std::vector<unsigned char> keys(simplifiedRing1.size(), 0);
        keys.front() = 1;
        keys.back() = 1;
        Helper helper(projection, projectionSurface);
        helper.approximate(simplifiedRing1.data(), simplifiedRing1.size(), scale * _tolerance, &keys[0]);
        std::vector<MapPos> simplifiedRing2;
        simplifiedRing2.reserve(std::count(keys.begin(), keys.end(), 1));
        for (std::size_t i = 0; i < simplifiedRing1.size(); i++) {
            if (keys[i] == 1) {
                simplifiedRing2.push_back(simplifiedRing1[i]);
            }
        }

        return simplifiedRing2;
    }

}
