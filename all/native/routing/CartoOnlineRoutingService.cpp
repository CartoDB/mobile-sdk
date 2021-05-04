#ifdef _CARTO_ROUTING_SUPPORT

#include "CartoOnlineRoutingService.h"
#include "components/Exceptions.h"
#include "components/LicenseManager.h"
#include "projections/Projection.h"
#include "routing/utils/OSRMRoutingProxy.h"
#include "network/HTTPClient.h"
#include "utils/Log.h"
#include "utils/PlatformUtils.h"
#include "utils/NetworkUtils.h"

#include <boost/lexical_cast.hpp>

namespace carto {

    CartoOnlineRoutingService::CartoOnlineRoutingService(const std::string& source) :
        RoutingService(),
        _source(source)
    {
    }

    CartoOnlineRoutingService::~CartoOnlineRoutingService() {
    }

    std::string CartoOnlineRoutingService::getProfile() const {
        return std::string();
    }

    void CartoOnlineRoutingService::setProfile(const std::string& profile) {
        if (!profile.empty()) {
            throw GenericException("Only default profile supported");
        }
    }

    std::shared_ptr<RouteMatchingResult> CartoOnlineRoutingService::matchRoute(const std::shared_ptr<RouteMatchingRequest>& request) const {
        throw GenericException("matchRoute not implemented for this RoutingService");
    }

    std::shared_ptr<RoutingResult> CartoOnlineRoutingService::calculateRoute(const std::shared_ptr<RoutingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        std::shared_ptr<Projection> proj = request->getProjection();
        
        std::string baseURL = NetworkUtils::CreateServiceURL(ROUTING_SERVICE_TEMPLATE, _source);
        for (const MapPos& pos : request->getPoints()) {
            MapPos wgsPos = proj->toWgs84(pos);
            baseURL += "&loc=" + boost::lexical_cast<std::string>(wgsPos.getY()) + "," + boost::lexical_cast<std::string>(wgsPos.getX());
        }

        std::string appToken;
        if (!LicenseManager::GetInstance().getParameter("appToken", appToken, false)) {
            throw GenericException("Service not available (license issue?)");
        }

        std::map<std::string, std::string> params;
        params["appId"] = PlatformUtils::GetAppIdentifier();
        params["deviceId"] = PlatformUtils::GetDeviceId();
        params["platform"] = PlatformUtils::GetPlatformId();
        params["sdk_build"] = PlatformUtils::GetSDKVersion();
        params["appToken"] = appToken;

        std::string url = NetworkUtils::BuildURLFromParameters(baseURL, params);
        Log::Debugf("CartoOnlineRoutingService::calculateRoute: Loading %s", url.c_str());

        HTTPClient httpClient(Log::IsShowDebug());
        return OSRMRoutingProxy::CalculateRoute(httpClient, url, request);
    }

    const std::string CartoOnlineRoutingService::ROUTING_SERVICE_TEMPLATE = "https://api.nutiteq.com/routing/v2/{source}/1/viaroute?instructions=true&alt=false&geometry=true&output=json";
    
}

#endif
