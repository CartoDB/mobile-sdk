#if defined(_CARTO_GEOCODING_SUPPORT)

#include "MapBoxOnlineReverseGeocodingService.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "geocoding/utils/MapBoxGeocodingProxy.h"
#include "projections/Projection.h"
#include "utils/GeneralUtils.h"
#include "utils/NetworkUtils.h"
#include "utils/Log.h"

#include <boost/lexical_cast.hpp>

namespace carto {

    MapBoxOnlineReverseGeocodingService::MapBoxOnlineReverseGeocodingService(const std::string& accessToken) :
        _accessToken(accessToken),
        _language(),
        _serviceURL(),
        _mutex()
    {
    }

    MapBoxOnlineReverseGeocodingService::~MapBoxOnlineReverseGeocodingService() {
    }

    std::string MapBoxOnlineReverseGeocodingService::getCustomServiceURL() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _serviceURL;
    }

    void MapBoxOnlineReverseGeocodingService::setCustomServiceURL(const std::string& serviceURL) {
        std::lock_guard<std::mutex> lock(_mutex);
        _serviceURL = serviceURL;
    }

    std::string MapBoxOnlineReverseGeocodingService::getLanguage() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _language;
    }

    void MapBoxOnlineReverseGeocodingService::setLanguage(const std::string& lang) {
        std::lock_guard<std::mutex> lock(_mutex);
        _language = lang;
    }

    std::vector<std::shared_ptr<GeocodingResult> > MapBoxOnlineReverseGeocodingService::calculateAddresses(const std::shared_ptr<ReverseGeocodingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        MapPos point = request->getProjection()->toWgs84(request->getLocation());

        std::string baseURL;

        std::map<std::string, std::string> params;
        {
            std::lock_guard<std::mutex> lock(_mutex);

            std::map<std::string, std::string> tagMap;
            tagMap["query"] = NetworkUtils::URLEncode(boost::lexical_cast<std::string>(point.getX()) + "," + boost::lexical_cast<std::string>(point.getY()));
            tagMap["access_token"] = NetworkUtils::URLEncode(_accessToken);

            baseURL = GeneralUtils::ReplaceTags(_serviceURL.empty() ? MAPBOX_SERVICE_URL : _serviceURL, tagMap);

            if (!_language.empty()) {
                params["language"] = _language;
            }
        }

        std::string url = NetworkUtils::BuildURLFromParameters(baseURL, params);
        Log::Debugf("MapBoxOnlineReverseGeocodingService::calculateAddresses: Loading %s", url.c_str());

        std::string responseString;
        if (!NetworkUtils::GetHTTP(url, responseString, Log::IsShowDebug())) {
            throw NetworkException("Failed to fetch response");
        }
        return MapBoxGeocodingProxy::ReadResponse(responseString, request->getProjection());
    }

    const std::string MapBoxOnlineReverseGeocodingService::MAPBOX_SERVICE_URL = "https://api.mapbox.com/geocoding/v5/mapbox.places-permanent/{query}.json?access_token={access_token}";
}

#endif
