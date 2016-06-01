/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOMUTILS_H_
#define _CARTO_GEOMUTILS_H_

#include <vector>

namespace carto {
    class MapBounds;
    class MapPos;
    class MapVec;
    
    class GeomUtils {
    public:
        static double DistanceFromPoint(const MapPos& pos, const MapPos& p);
    
        static double DistanceFromLine(const MapPos& pos, const MapPos& a, const MapPos& b);

        static double DistanceFromLineSegment(const MapPos& pos, const MapPos& a, const MapPos& b);
    
        static MapPos CalculateNearestPointOnLineSegment(const MapPos& pos, const MapPos& a, const MapPos& b);
    
        static bool IsConvexPolygonClockwise(const std::vector<MapPos>& polygon);
    
        static bool IsConcavePolygonClockwise(const std::vector<MapPos>& polygon);
    
        static bool PointInsidePolygon(const std::vector<MapPos>& polygon, const MapPos& point);
    
        static MapPos CalculatePointInsidePolygon(const std::vector<MapPos>& polygon, const std::vector<std::vector<MapPos> >& holes);
        
        static MapPos CalculatePointOnLine(const std::vector<MapPos>& line);
    
        static bool PolygonsIntersect(const std::vector<MapPos>& polygon1, const std::vector<MapPos>& polygon2);
    
        static std::vector<MapPos> CalculateConvexHull(std::vector<MapPos> points);
    
        static bool RayTriangleIntersect(const MapPos& rayOrig, const MapVec& rayDir,
                const MapPos& triPoint0, const MapPos& triPoint1, const MapPos& triPoint2, MapPos& result);
    
        static bool RayBoundingBoxIntersect(const MapPos& rayOrig, const MapVec& rayDir, const MapBounds& bbox);
    
    private:
        GeomUtils();

        static bool LexicalComparator(const MapPos& mapPos1, const MapPos& mapPos2);
    
        static bool PointsInsidePolygonEdges(const std::vector<MapPos>& polygon, const std::vector<MapPos>& points);
    };
    
}

#endif
