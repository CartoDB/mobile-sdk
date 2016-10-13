#ifdef _CARTO_ROUTING_SUPPORT

#include "CartoOnlineRoutingService.h"
#include "components/Exceptions.h"
#include "components/LicenseManager.h"
#include "projections/Projection.h"
#include "routing/RoutingProxy.h"
#include "network/HTTPClient.h"
#include "utils/Log.h"
#include "utils/PlatformUtils.h"
#include "utils/NetworkUtils.h"

#include <boost/lexical_cast.hpp>

namespace carto {

    CartoOnlineRoutingService::CartoOnlineRoutingService(const std::string& source) :
        _source(source)
    {
    }

    CartoOnlineRoutingService::~CartoOnlineRoutingService() {
    }

    std::shared_ptr<RoutingResult> CartoOnlineRoutingService::calculateRoute(const std::shared_ptr<RoutingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        std::shared_ptr<Projection> proj = request->getProjection();
        
        std::string baseURL = ROUTING_SERVICE_URL + NetworkUtils::URLEncode(_source) + "/1/viaroute?instructions=true&alt=false&geometry=true&output=json";
        for (const MapPos& pos : request->getPoints()) {
            MapPos wgsPos = proj->toWgs84(pos);
            baseURL += "&loc=" + boost::lexical_cast<std::string>(wgsPos.getY()) + "," + boost::lexical_cast<std::string>(wgsPos.getX());
        }

        std::map<std::string, std::string> params;
        params["appId"] = PlatformUtils::GetAppIdentifier();
        params["deviceId"] = PlatformUtils::GetDeviceId();
        params["platform"] = PlatformUtils::GetPlatformId();
        params["sdk_build"] = _CARTO_MOBILE_SDK_VERSION;
        std::string appToken;
        if (LicenseManager::GetInstance().getParameter("appToken", appToken)) {
            params["appToken"] = appToken;
        }

        std::string url = NetworkUtils::BuildURLFromParameters(baseURL, params);
        Log::Debugf("CartoOnlineRoutingService::calculateRoute: Loading %s", url.c_str());

        HTTPClient httpClient(Log::IsShowDebug());
        return RoutingProxy::CalculateRoute(httpClient, url, request);
    }

    const std::string CartoOnlineRoutingService::ROUTING_SERVICE_URL = "http://api.nutiteq.com/routing/v2/";
    
}

#endif
