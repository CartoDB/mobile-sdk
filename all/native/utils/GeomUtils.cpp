#include "GeomUtils.h"
#include "core/MapBounds.h"
#include "core/MapPos.h"
#include "core/MapVec.h"
#include "utils/Log.h"
#include "utils/GeneralUtils.h"

#include <algorithm>
#include <utility>
#include <cmath>

namespace carto {

    double GeomUtils::DistanceFromPoint(const MapPos& pos, const MapPos& p) {
        MapVec diff(p - pos);
        return std::sqrt(diff.dotProduct(diff));
    }
    
    double GeomUtils::DistanceFromLine(const MapPos& pos, const MapPos& a, const MapPos& b) {
        if (a == b) {
            return DistanceFromPoint(pos, a);
        }
        MapVec diff = pos - a;
        MapVec dir = b - a;
        double u = diff.dotProduct(dir) / dir.dotProduct(dir);
        dir *= u;
        return DistanceFromPoint(a + dir, pos);
    }
    
    double GeomUtils::DistanceFromLineSegment(const MapPos& pos, const MapPos& a, const MapPos& b) {
        MapPos nearest(CalculateNearestPointOnLineSegment(pos, a, b));
        return DistanceFromPoint(nearest, pos);
    }
    
    MapPos GeomUtils::CalculateNearestPointOnLineSegment(const MapPos& pos, const MapPos& a, const MapPos& b) {
        if (a == b) {
            return a;
        }
        MapVec diff = pos - a;
        MapVec dir = b - a;
        double u = GeneralUtils::Clamp(diff.dotProduct(dir) / dir.dotProduct(dir), 0.0, 1.0);
        dir *= u;
        return a + dir;
    }
    
    bool GeomUtils::IsConvexPolygonClockwise(const std::vector<MapPos>& polygon) {
        for (size_t i = 0; i < polygon.size(); i++) {
            const MapPos& v1 = polygon[(i + polygon.size() - 1) % polygon.size()];
            const MapPos& v2 = polygon[i];
            const MapPos& v3 = polygon[(i + 1) % polygon.size()];
            double d = (v2 - v1).crossProduct2D(v3 - v2);
            if (d < 0) {
                return true;
            } else if (d > 0) {
                return false;
            }
        }
        return true;
    }
    
    bool GeomUtils::IsConcavePolygonClockwise(const std::vector<MapPos>& polygon) {
        double area = 0;
        MapVec prevLine;
        for (size_t i = 0; i < polygon.size(); i++) {
            const MapPos& pos1 = polygon[i];
            const MapPos& pos2 = (i + 1 < polygon.size()) ? polygon[i + 1] : polygon[1];
            MapVec nextLine(pos2 - pos1);
            if (i > 0) {
                area += prevLine.crossProduct2D(nextLine);
            }
            prevLine = nextLine;
        }
        return area < 0;
    }
    
    bool GeomUtils::PointInsidePolygon(const std::vector<MapPos>& polygon, const MapPos& point) {
        float c = IsConvexPolygonClockwise(polygon) ? -1.0f : 1.0f;
        for (size_t i = 0; i < polygon.size(); i++) {
            const MapPos& v1 = polygon[i];
            const MapPos& v2 = polygon[(i + 1) % polygon.size()];
            double d = (v2 - v1).crossProduct2D(point - v1);
            if (c * d > 0) {
                return false;
            }
        }
        return true;
    }
    
    MapPos GeomUtils::CalculatePointInsidePolygon(const std::vector<MapPos>& polygon, const std::vector<std::vector<MapPos> >& holes) {
        // Calculate centroid from ring
        MapPos centroid(0, 0);
        for (const MapPos& mapPos : polygon) {
            centroid += (mapPos - MapPos(0, 0)) * (1.0 / polygon.size());
        }
        
        // Find all intersection of horizontal ray starting from centroid (bidirectional ray)
        std::vector<double> ts;
        std::vector<std::vector<MapPos> > rings;
        rings.push_back(polygon);
        rings.insert(rings.end(), holes.begin(), holes.end());
        for (const std::vector<MapPos>& ring : rings) {
            for (size_t i = 0; i < ring.size(); i++) {
                size_t j = (i + 1) % ring.size();
                const MapPos& p0 = ring[i];
                const MapPos& p1 = ring[j];
                if (p0.getY() == p1.getY()) {
                    continue;
                }
                double s = (centroid.getY() - p0.getY()) / (p1.getY() - p0.getY());
                if (s < 0 || s > 1) {
                    continue;
                }
                double t = p0.getX() - centroid.getX() + (p1.getX() - p0.getX()) * s;
                ts.push_back(t);
            }
        }
        
        // Sort intersections and find one close to centroid
        std::sort(ts.begin(), ts.end());
        double best_t = ts.size() >= 2 ? std::numeric_limits<double>::infinity() : 0;
        for (size_t i = 0; i + 1 < ts.size(); i += 2) {
            double t = (ts[i + 0] + ts[i + 1]) * 0.5;
            if (std::abs(t) < std::abs(best_t)) {
                best_t = t;
            }
        }
        return centroid + MapVec(best_t, 0);
    }
        
    MapPos GeomUtils::CalculatePointOnLine(const std::vector<MapPos>& line) {
        // Calculate total line length
        double len = 0;
        for (size_t i = 1; i < line.size(); i++) {
            len += (line[i] - line[i - 1]).length();
        }
        
        // Find point at the center of the line
        MapPos midPoint(0, 0);
        double t = 0;
        for (size_t i = 1; i < line.size(); i++) {
            double dt = (line[i] - line[i - 1]).length() / len;
            if (t + dt >= 0.5) {
                midPoint = line[i - 1] + (line[i] - line[i - 1]) * ((0.5 - t) / dt);
                break;
            }
            t += dt;
        }
        return midPoint;
    }
    
    bool GeomUtils::PolygonsIntersect(const std::vector<MapPos>& polygon1, const std::vector<MapPos>& polygon2) {
        return PointsInsidePolygonEdges(polygon1, polygon2) && PointsInsidePolygonEdges(polygon2, polygon1);
    }
    
    std::vector<MapPos> GeomUtils::CalculateConvexHull(std::vector<MapPos> points) {
        std::vector<MapPos> h(points.size() * 2);
        std::sort(points.begin(), points.end(), [](const MapPos& mapPos1, const MapPos& mapPos2) {
            double dx = mapPos1.getX() - mapPos2.getX();
            if (dx != 0) {
                return (dx < 0 ? true : false);
            }
            double dy = mapPos1.getY() - mapPos2.getY();
        
            return (dy < 0 ? true : false);
        });
        int k = 0;
    
        // Build lower hull
        for (size_t i = 0; i < points.size(); i++) {
            while (k >= 2) {
                if ((h[k - 1] - h[k - 2]).crossProduct2D(points[i] - h[k - 2]) < 0) {
                    break;
                }
                k--;
            }
            h[k++] = points[i];
        }
    
        // Build upper hull
        for (int i = static_cast<int>(points.size()) - 2, t = k + 1; i >= 0; i--) {
            while (k >= t) {
                if ((h[k - 1] - h[k - 2]).crossProduct2D(points[i] - h[k - 2]) < 0) {
                    break;
                }
                k--;
            }
            h[k++] = points[i];
        }
    
        // Remove consecutive duplicates
        for (int i = 0; i < k;) {
            int j = (i + k - 1) % k;
            if (h[i] == h[j]) {
                for (j = i + 1; j < k; j++) {
                    h[j - 1] = h[j];
                }
                k--;
            } else {
                i++;
            }
        }
    
        // Create result list
        h.resize(k);
        return h;
    }
    
    bool GeomUtils::RayZPlaneIntersect(const MapPos& rayOrig, const MapVec& rayDir, double z, MapPos& result) {
        if (rayDir.getZ() == 0) {
            return false;
        }

        double t = (z - rayOrig.getZ()) / rayDir.getZ();
        result = rayOrig;
        result += rayDir * t;
        return true;
    }

    bool GeomUtils::RayTriangleIntersect(const MapPos& rayOrig, const MapVec& rayDir,
            const MapPos& triPoint0, const MapPos& triPoint1, const MapPos& triPoint2, MapPos& result) {
        static const double EPSILON = 0.000001;
     
        // Find vectors for two edges sharing vert0
        MapVec edge1(triPoint1 - triPoint0);
        MapVec edge2(triPoint2 - triPoint0);
     
        // Begin calculating determinant - also used to calculate U parameter
        MapVec pvec(rayDir.crossProduct3D(edge2));
     
        // If determinant is near zero, ray lies in plane of triangle
        MapVec qvec;
        double det = edge1.dotProduct(pvec);
        if (det > EPSILON) {
            // Calculate distance from vert0 to ray origin
            MapVec tvec(rayOrig - triPoint0);
      
            // Calculate U parameter and test bounds
            double u = tvec.dotProduct(pvec);
            if (u < 0.0 || u > det) {
                return false;
            }
      
            // Prepare to test V parameter
            qvec = tvec.crossProduct3D(edge1);
      
            // Calculate V parameter and test bounds
            double v = rayDir.dotProduct(qvec);
            if (v < 0.0 || u + v > det) {
                return false;
            }
        } else if (det < -EPSILON) {
             // Calculate distance from vert0 to ray origin
            MapVec tvec(rayOrig - triPoint0);
      
            // Calculate U parameter and test bounds
            double u = tvec.dotProduct(pvec);
            if (u > 0.0 || u < det) {
                return false;
            }
      
            // Prepare to test V parameter
            qvec = tvec.crossProduct3D(edge1);
      
            // Calculate V parameter and test bounds
            double v = rayDir.dotProduct(qvec);
            if (v > 0.0 || u + v < det) {
          	  return false;
            }
        } else {
            // Ray is parallel to the plane of the triangle
            return false;
        }
     
        // Calculate t, ray intersects triangle
        double t = edge2.dotProduct(qvec) / det;
        result = rayOrig;
        result += rayDir * t;
        return true;
    }
    
    bool GeomUtils::RayBoundingBoxIntersect(const MapPos& rayOrig, const MapVec& rayDir, const MapBounds& bbox) {
        MapVec invRayDir(1 / rayDir.getX(), 1 / rayDir.getY(), 1 / rayDir.getZ());
        double tx1 = (bbox.getMin().getX() - rayOrig.getX()) * invRayDir.getX();
        double tx2 = (bbox.getMax().getX() - rayOrig.getX()) * invRayDir.getX();
    
        double tmin = std::min(tx1, tx2);
        double tmax = std::max(tx1, tx2);
    
        double ty1 = (bbox.getMin().getY() - rayOrig.getY()) * invRayDir.getY();
        double ty2 = (bbox.getMax().getY() - rayOrig.getY()) * invRayDir.getY();
    
        tmin = std::max(tmin, std::min(ty1, ty2));
        tmax = std::min(tmax, std::max(ty1, ty2));
    
        double tz1 = (bbox.getMin().getZ() - rayOrig.getZ()) * invRayDir.getZ();
        double tz2 = (bbox.getMax().getZ() - rayOrig.getZ()) * invRayDir.getZ();
    
        tmin = std::max(tmin, std::min(tz1, tz2));
        tmax = std::min(tmax, std::max(tz1, tz2));
    
        return tmax >= std::max(0.0, tmin);
    }
    
    bool GeomUtils::PointsInsidePolygonEdges(const std::vector<MapPos>& polygon, const std::vector<MapPos>& points) {
        float c = IsConvexPolygonClockwise(polygon) ? -1.0f : 1.0f;
        for (size_t i = 0; i < polygon.size(); i++) {
            const MapPos& v1 = polygon[i];
            const MapPos& v2 = polygon[(i + 1) % polygon.size()];
            bool inside = false;
            for (size_t j = 0; j < polygon.size(); j++) {
                MapPos v3(polygon[j]);
                double d = ((v2 - v1).crossProduct2D(v3 - v1));
                if (c * d >= 0) {
                    inside = true;
                    break;
                }
            }
            if (!inside) {
                return false;
            }
        }
        return true;
    }

    GeomUtils::GeomUtils() {
    }

}
