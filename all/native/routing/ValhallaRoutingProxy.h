/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VALHALLAROUTINGPROXY_H_
#define _CARTO_VALHALLAROUTINGPROXY_H_

#ifdef _CARTO_ROUTING_SUPPORT

#include "core/Variant.h"
#include "routing/RoutingInstruction.h"

#include <memory>
#include <vector>
#include <unordered_map>

namespace sqlite3pp {
    class database;
}

namespace carto {
    class HTTPClient;
    class Projection;
    class RoutingRequest;
    class RoutingResult;
    class RouteMatchingRequest;
    class RouteMatchingResult;
    class TileDataSource;
    class ElevationDecoder;
    
    class ValhallaRoutingProxy {
    public:
        static std::shared_ptr<RouteMatchingResult> MatchRoute(HTTPClient& httpClient, const std::string& baseURL, const std::string& profile, const std::shared_ptr<RouteMatchingRequest>& request);
        static std::shared_ptr<RoutingResult> CalculateRoute(HTTPClient& httpClient, const std::string& baseURL, const std::string& profile, const std::shared_ptr<RoutingRequest>& request);

#ifdef _CARTO_VALHALLA_ROUTING_SUPPORT
        static std::shared_ptr<RouteMatchingResult> MatchRoute(const std::vector<std::shared_ptr<sqlite3pp::database> >& databases, const std::string& profile, const Variant& config, const std::shared_ptr<RouteMatchingRequest>& request);
        static std::shared_ptr<RoutingResult> CalculateRoute(const std::vector<std::shared_ptr<sqlite3pp::database> >& databases, const std::string& profile, const Variant& config, const std::shared_ptr<RoutingRequest>& request);
#endif

        static Variant GetDefaultConfiguration();

    private:
        ValhallaRoutingProxy();

        static float CalculateTurnAngle(const std::vector<MapPos>& epsg3857Points, int pointIndex);

        static float CalculateAzimuth(const std::vector<MapPos>& epsg3857Points, int pointIndex);

        static bool TranslateManeuverType(int maneuverType, RoutingAction::RoutingAction& action);

        static std::string SerializeRouteMatchingRequest(const std::string& profile, const std::shared_ptr<RouteMatchingRequest>& request);

        static std::string SerializeRoutingRequest(const std::string& profile, const std::shared_ptr<RoutingRequest>& request);

        static std::shared_ptr<RouteMatchingResult> ParseRouteMatchingResult(const std::shared_ptr<Projection>& proj, const std::string& resultString);

        static std::shared_ptr<RoutingResult> ParseRoutingResult(const std::shared_ptr<Projection>& proj, const std::string& resultString);

        static std::string MakeHTTPRequest(HTTPClient& httpClient, const std::string& url);
    };

}

#endif

#endif
