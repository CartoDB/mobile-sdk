#if defined(_CARTO_VALHALLA_ROUTING_SUPPORT)

#include "ValhallaOnlineRoutingService.h"
#include "components/Exceptions.h"
#include "routing/ValhallaRoutingProxy.h"
#include "utils/GeneralUtils.h"
#include "utils/NetworkUtils.h"
#include "utils/Log.h"

#include <map>

namespace carto {

    ValhallaOnlineRoutingService::ValhallaOnlineRoutingService(const std::string& apiKey) :
        _apiKey(apiKey),
        _profile("pedestrian"),
        _serviceURL(),
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

    std::string ValhallaOnlineRoutingService::getCustomServiceURL() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _serviceURL;
    }

    void ValhallaOnlineRoutingService::setCustomServiceURL(const std::string& serviceURL) {
        std::lock_guard<std::mutex> lock(_mutex);
        _serviceURL = serviceURL;
    }

    std::shared_ptr<RoutingResult> ValhallaOnlineRoutingService::calculateRoute(const std::shared_ptr<RoutingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        std::string baseURL;

        std::map<std::string, std::string> params;
        {
            std::lock_guard<std::mutex> lock(_mutex);

            std::map<std::string, std::string> tagMap;
            tagMap["api_key"] = NetworkUtils::URLEncode(_apiKey);
            baseURL = GeneralUtils::ReplaceTags(_serviceURL.empty() ? MAPZEN_SERVICE_URL : _serviceURL, tagMap);
        }

        std::string url = NetworkUtils::BuildURLFromParameters(baseURL, params);
        return ValhallaRoutingProxy::CalculateRoute(url, getProfile(), request);
    }

    const std::string ValhallaOnlineRoutingService::MAPZEN_SERVICE_URL = "https://valhalla.mapzen.com/route?api_key={api_key}";

}

#endif
