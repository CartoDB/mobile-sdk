#if defined(_CARTO_GEOCODING_SUPPORT)

#include "TomTomOnlineReverseGeocodingService.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "geocoding/utils/TomTomGeocodingProxy.h"
#include "projections/Projection.h"
#include "utils/GeneralUtils.h"
#include "utils/NetworkUtils.h"
#include "utils/Log.h"

#include <boost/lexical_cast.hpp>

namespace carto {

    TomTomOnlineReverseGeocodingService::TomTomOnlineReverseGeocodingService(const std::string& apiKey) :
        _apiKey(apiKey),
        _language(),
        _serviceURL(),
        _mutex()
    {
    }

    TomTomOnlineReverseGeocodingService::~TomTomOnlineReverseGeocodingService() {
    }

    std::string TomTomOnlineReverseGeocodingService::getCustomServiceURL() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _serviceURL;
    }

    void TomTomOnlineReverseGeocodingService::setCustomServiceURL(const std::string& serviceURL) {
        std::lock_guard<std::mutex> lock(_mutex);
        _serviceURL = serviceURL;
    }

    std::string TomTomOnlineReverseGeocodingService::getLanguage() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _language;
    }

    void TomTomOnlineReverseGeocodingService::setLanguage(const std::string& lang) {
        std::lock_guard<std::mutex> lock(_mutex);
        _language = lang;
    }

    std::vector<std::shared_ptr<GeocodingResult> > TomTomOnlineReverseGeocodingService::calculateAddresses(const std::shared_ptr<ReverseGeocodingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        MapPos point = request->getProjection()->toWgs84(request->getLocation());

        std::string baseURL;

        std::map<std::string, std::string> params;
        {
            std::lock_guard<std::mutex> lock(_mutex);

            std::map<std::string, std::string> tagMap;
            tagMap["query"] = NetworkUtils::URLEncode(boost::lexical_cast<std::string>(point.getY()) + "," + boost::lexical_cast<std::string>(point.getX()));
            tagMap["api_key"] = NetworkUtils::URLEncode(_apiKey);

            baseURL = GeneralUtils::ReplaceTags(_serviceURL.empty() ? TOMTOM_SERVICE_URL : _serviceURL, tagMap);

            params["radius"] = boost::lexical_cast<std::string>(request->getSearchRadius());
            if (!_language.empty()) {
                params["language"] = _language;
            }
        }

        std::string url = NetworkUtils::BuildURLFromParameters(baseURL, params);
        Log::Debugf("TomTomOnlineReverseGeocodingService::calculateAddresses: Loading %s", url.c_str());

        std::string responseString;
        if (!NetworkUtils::GetHTTP(url, responseString, Log::IsShowDebug())) {
            throw NetworkException("Failed to fetch response");
        }
        return TomTomGeocodingProxy::ReadResponse(responseString, request->getProjection());
    }

    const std::string TomTomOnlineReverseGeocodingService::TOMTOM_SERVICE_URL = "https://api.tomtom.com/search/2/reverseGeocode/{query}.json?key={api_key}";
}

#endif
