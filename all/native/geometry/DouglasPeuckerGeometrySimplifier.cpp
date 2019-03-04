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

#include <stack>
#include <utility>
#include <algorithm>

namespace {

    // This is taken from http://psimpl.sourceforge.net/douglas-peucker.html
    class DPHelper {
    public:
        DPHelper(const std::shared_ptr<carto::Projection>& projection) : _projection(projection) {
        }

        void approximate(const carto::MapPos* points, std::size_t pointCount, double minDist, unsigned char* keys) const {
            double minDist2 = minDist * minDist;
            std::stack<SubPoly> stack;
            stack.push(SubPoly(0, pointCount - 1));
            while (!stack.empty()) {
                SubPoly subPoly = stack.top();
                stack.pop();
                KeyInfo keyInfo = findKey(points, subPoly.first, subPoly.last);
                if (keyInfo.index && minDist2 < keyInfo.dist2) {
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
            KeyInfo(std::size_t index = 0, double dist2 = 0) : index(index), dist2(dist2) { }

            std::size_t index;
            double dist2;
        };

        static double FindSegmentDistance2(const carto::MapPos& s1, const carto::MapPos& s2, const carto::MapPos& p) {
            carto::MapVec v = (s2 - s1);
            carto::MapVec w = (p - s1);
            double cw = v.dotProduct(w);
            if (cw <= 0) {
                return (p - s1).lengthSqr();
            }
            double cv = v.lengthSqr();
            if (cv <= cw) {
                return (p - s2).lengthSqr();
            }

            double fraction = cv == 0 ? 0 : cw / cv;
            carto::MapPos pProj = s1 + (s2 - s1) * fraction;
            return (pProj - p).lengthSqr();
        }

        KeyInfo findKey(const carto::MapPos* points, std::size_t first, std::size_t last) const {
            KeyInfo keyInfo;

            for (std::size_t current = first + 1; current < last; current++) {
                double d2 = FindSegmentDistance2(_projection->toInternal(points[first]), _projection->toInternal(points[last]), _projection->toInternal(points[current]));
                if (d2 < keyInfo.dist2) {
                    continue;
                }
                keyInfo.index = current;
                keyInfo.dist2 = d2;
            }
            return keyInfo;
        }

        std::shared_ptr<carto::Projection> _projection;
    };

}

namespace carto {

    DouglasPeuckerGeometrySimplifier::DouglasPeuckerGeometrySimplifier(float tolerance) :
        GeometrySimplifier(),
        _tolerance(tolerance)
    {
    }

    std::shared_ptr<Geometry> DouglasPeuckerGeometrySimplifier::simplify(const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<Projection>& projection, float scale) const {
        if (auto lineGeometry = std::dynamic_pointer_cast<LineGeometry>(geometry)) {
            std::vector<MapPos> mapPoses = simplifyRing(lineGeometry->getPoses(), projection, scale);
            if (mapPoses.size() < 2) {
                return std::shared_ptr<Geometry>();
            }
            bool simplified = mapPoses.size() < lineGeometry->getPoses().size();
            if (simplified) {
                return std::make_shared<LineGeometry>(mapPoses);
            }
        } else if (auto polygonGeometry = std::dynamic_pointer_cast<PolygonGeometry>(geometry)) {
            std::vector<MapPos> mapPoses = simplifyRing(polygonGeometry->getPoses(), projection, scale);
            if (mapPoses.size() < 3) {
                return std::shared_ptr<Geometry>();
            }
            bool simplified = mapPoses.size() < polygonGeometry->getPoses().size();
            std::vector<std::vector<MapPos> > holes;
            for (const std::vector<MapPos>& holeRing : polygonGeometry->getHoles()) {
                std::vector<MapPos> holeMapPoses = simplifyRing(holeRing, projection, scale);
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
                std::shared_ptr<Geometry> geom = simplify(multiLineGeometry->getGeometry(i), projection, scale);
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
                std::shared_ptr<Geometry> geom = simplify(multiPolygonGeometry->getGeometry(i), projection, scale);
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
                std::shared_ptr<Geometry> geom = simplify(multiGeometry->getGeometry(i), projection, scale);
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

    std::vector<MapPos> DouglasPeuckerGeometrySimplifier::simplifyRing(const std::vector<MapPos>& ring, const std::shared_ptr<Projection>& projection, float scale) const {
        return simplifyRingDP(simplifyRingRD(ring, projection, scale), projection, scale);
    }

    std::vector<MapPos> DouglasPeuckerGeometrySimplifier::simplifyRingRD(const std::vector<MapPos>& ring, const std::shared_ptr<Projection>& projection, float scale) const {
        if (ring.size() <= 2) {
            return ring;
        }

        double minDist2 = scale * _tolerance * scale * _tolerance;
        std::vector<MapPos> simplifiedRing;
        simplifiedRing.reserve(ring.size());
        simplifiedRing.push_back(ring.front());
        for (std::size_t i = 1; i + 1 < ring.size(); i++) {
            double dist2 = (projection->toInternal(ring[i]) - projection->toInternal(simplifiedRing.back())).lengthSqr();
            if (dist2 > minDist2) {
                simplifiedRing.push_back(ring[i]);
            }
        }
        simplifiedRing.push_back(ring.back());
        return simplifiedRing;
    }

    std::vector<MapPos> DouglasPeuckerGeometrySimplifier::simplifyRingDP(const std::vector<MapPos>& ring, const std::shared_ptr<Projection>& projection, float scale) const {
        if (ring.size() <= 2) {
            return ring;
        }

        std::vector<unsigned char> keys(ring.size(), 0);
        keys.front() = 1;
        keys.back() = 1;
        DPHelper helper(projection);
        helper.approximate(&ring[0], ring.size(), scale * _tolerance, &keys[0]);
        std::vector<MapPos> simplifiedRing;
        simplifiedRing.reserve(std::count(keys.begin(), keys.end(), 1));
        for (std::size_t i = 0; i < ring.size(); i++) {
            if (keys[i] == 1) {
                simplifiedRing.push_back(ring[i]);
            }
        }
        return simplifiedRing;
    }

}
