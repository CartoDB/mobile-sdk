/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VALHALLAROUTINGPROXY_H_
#define _CARTO_VALHALLAROUTINGPROXY_H_

#ifdef _CARTO_VALHALLA_ROUTING_SUPPORT

#include "routing/RoutingService.h"

#include <memory>
#include <vector>

namespace sqlite3pp {
    class database;
}

namespace carto {
    class RouteMatchingRequest;
    class RouteMatchingResult;
    
    class ValhallaRoutingProxy {
    public:
        static std::shared_ptr<RouteMatchingResult> MatchRoute(const std::vector<std::shared_ptr<sqlite3pp::database> >& databases, const std::string& profile, const std::shared_ptr<RouteMatchingRequest>& request);

        static std::shared_ptr<RoutingResult> CalculateRoute(const std::string& baseURL, const std::string& profile, const std::shared_ptr<RoutingRequest>& request);
        static std::shared_ptr<RoutingResult> CalculateRoute(const std::vector<std::shared_ptr<sqlite3pp::database> >& databases, const std::string& profile, const std::shared_ptr<RoutingRequest>& request);
        
    private:
        ValhallaRoutingProxy();

        static float CalculateTurnAngle(const std::vector<MapPos>& epsg3857Points, int pointIndex);

        static float CalculateAzimuth(const std::vector<MapPos>& epsg3857Points, int pointIndex);

        static bool TranslateManeuverType(int maneuverType, RoutingAction::RoutingAction& action);
    };
    
}

#endif

#endif
