#if defined(_CARTO_VALHALLA_ROUTING_SUPPORT)

#include "ValhallaOnlineRoutingService.h"
#include "components/Exceptions.h"
#include "routing/ValhallaRoutingProxy.h"
#include "utils/NetworkUtils.h"
#include "utils/Log.h"

#include <map>

namespace carto {

    ValhallaOnlineRoutingService::ValhallaOnlineRoutingService(const std::string& apiKey) :
        _apiKey(apiKey),
        _profile("pedestrian"),
        _mutex()
    {
    }

    ValhallaOnlineRoutingService::~ValhallaOnlineRoutingService() {
    }

    std::string ValhallaOnlineRoutingService::getProfile() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _profile;
    }

    void ValhallaOnlineRoutingService::setProfile(const std::string& profile) {
        std::lock_guard<std::mutex> lock(_mutex);
        _profile = profile;
    }

    std::shared_ptr<RoutingResult> ValhallaOnlineRoutingService::calculateRoute(const std::shared_ptr<RoutingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        std::map<std::string, std::string> params;
        params["api_key"] = _apiKey;
        std::string url = NetworkUtils::BuildURLFromParameters(VALHALLA_ROUTING_URL, params);
        return ValhallaRoutingProxy::CalculateRoute(url, getProfile(), request);
    }

    const std::string ValhallaOnlineRoutingService::VALHALLA_ROUTING_URL = "https://valhalla.mapzen.com/route";

}

#endif
