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

    private:
        GeomUtils();

        static bool PointsInsidePolygonEdges(const std::vector<MapPos>& polygon, const std::vector<MapPos>& points);
    };
    
}

#endif
