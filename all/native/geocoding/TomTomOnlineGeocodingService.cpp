#if defined(_CARTO_GEOCODING_SUPPORT)

#include "TomTomOnlineGeocodingService.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "geocoding/utils/TomTomGeocodingProxy.h"
#include "projections/Projection.h"
#include "utils/GeneralUtils.h"
#include "utils/NetworkUtils.h"
#include "utils/Log.h"

#include <boost/lexical_cast.hpp>

namespace carto {

    TomTomOnlineGeocodingService::TomTomOnlineGeocodingService(const std::string& apiKey) :
        _apiKey(apiKey),
        _autocomplete(false),
        _language(),
        _maxResults(10),
        _serviceURL(),
        _mutex()
    {
    }

    TomTomOnlineGeocodingService::~TomTomOnlineGeocodingService() {
    }

    std::string TomTomOnlineGeocodingService::getCustomServiceURL() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _serviceURL;
    }

    void TomTomOnlineGeocodingService::setCustomServiceURL(const std::string& serviceURL) {
        std::lock_guard<std::mutex> lock(_mutex);
        _serviceURL = serviceURL;
    }

    bool TomTomOnlineGeocodingService::isAutocomplete() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _autocomplete;
    }

    void TomTomOnlineGeocodingService::setAutocomplete(bool autocomplete) {
        std::lock_guard<std::mutex> lock(_mutex);
        _autocomplete = autocomplete;
    }

    std::string TomTomOnlineGeocodingService::getLanguage() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _language;
    }

    void TomTomOnlineGeocodingService::setLanguage(const std::string& lang) {
        std::lock_guard<std::mutex> lock(_mutex);
        _language = lang;
    }

    int TomTomOnlineGeocodingService::getMaxResults() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _maxResults;
    }

    void TomTomOnlineGeocodingService::setMaxResults(int maxResults) {
        std::lock_guard<std::mutex> lock(_mutex);
        _maxResults = maxResults;
    }

    std::vector<std::shared_ptr<GeocodingResult> > TomTomOnlineGeocodingService::calculateAddresses(const std::shared_ptr<GeocodingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        if (request->getQuery().empty()) {
            return std::vector<std::shared_ptr<GeocodingResult> >();
        }

        std::string baseURL;

        std::map<std::string, std::string> params;
        {
            std::lock_guard<std::mutex> lock(_mutex);

            std::map<std::string, std::string> tagMap;
            tagMap["query"] = NetworkUtils::URLEncode(request->getQuery());
            tagMap["api_key"] = NetworkUtils::URLEncode(_apiKey);

            baseURL = GeneralUtils::ReplaceTags(_serviceURL.empty() ? TOMTOM_SERVICE_URL : _serviceURL, tagMap);

            params["typeahead"] = _autocomplete ? "true": "false";

            if (request->isLocationDefined()) {
                MapPos wgs84Center = request->getProjection()->toWgs84(request->getLocation());
                params["lat"] = boost::lexical_cast<std::string>(wgs84Center.getY());
                params["lon"] = boost::lexical_cast<std::string>(wgs84Center.getX());
            }
            if (request->getLocationRadius() > 0) {
                double radius = request->getLocationRadius();
                params["radius"] = boost::lexical_cast<std::string>(radius);
            }

            if (!_language.empty()) {
                params["language"] = _language;
            }

            params["limit"] = boost::lexical_cast<std::string>(std::max(1, std::min(100, _maxResults)));
        }

        std::string url = NetworkUtils::BuildURLFromParameters(baseURL, params);
        Log::Debugf("TomTomOnlineGeocodingService::calculateAddresses: Loading %s", url.c_str());

        std::string responseString;
        if (!NetworkUtils::GetHTTP(url, responseString, Log::IsShowDebug())) {
            throw NetworkException("Failed to fetch response");
        }
        return TomTomGeocodingProxy::ReadResponse(responseString, request->getProjection());
    }

    const std::string TomTomOnlineGeocodingService::TOMTOM_SERVICE_URL = "https://api.tomtom.com/search/2/geocode/{query}.json?key={api_key}";
}

#endif
