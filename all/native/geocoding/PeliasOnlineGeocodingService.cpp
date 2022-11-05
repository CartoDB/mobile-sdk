#if defined(_CARTO_GEOCODING_SUPPORT)

#include "PeliasOnlineGeocodingService.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "geocoding/utils/PeliasGeocodingProxy.h"
#include "projections/Projection.h"
#include "utils/GeneralUtils.h"
#include "utils/NetworkUtils.h"
#include "utils/Log.h"

#include <algorithm>

#include <boost/lexical_cast.hpp>

namespace carto {

    PeliasOnlineGeocodingService::PeliasOnlineGeocodingService(const std::string& apiKey) :
        _apiKey(apiKey),
        _autocomplete(false),
        _language(),
        _maxResults(10),
        _serviceURL(),
        _mutex()
    {
    }

    PeliasOnlineGeocodingService::~PeliasOnlineGeocodingService() {
    }

    std::string PeliasOnlineGeocodingService::getCustomServiceURL() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _serviceURL;
    }

    void PeliasOnlineGeocodingService::setCustomServiceURL(const std::string& serviceURL) {
        std::lock_guard<std::mutex> lock(_mutex);
        _serviceURL = serviceURL;
    }

    bool PeliasOnlineGeocodingService::isAutocomplete() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _autocomplete;
    }

    void PeliasOnlineGeocodingService::setAutocomplete(bool autocomplete) {
        std::lock_guard<std::mutex> lock(_mutex);
        _autocomplete = autocomplete;
    }

    std::string PeliasOnlineGeocodingService::getLanguage() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _language;
    }

    void PeliasOnlineGeocodingService::setLanguage(const std::string& lang) {
        std::lock_guard<std::mutex> lock(_mutex);
        _language = lang;
    }

    int PeliasOnlineGeocodingService::getMaxResults() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _maxResults;
    }

    void PeliasOnlineGeocodingService::setMaxResults(int maxResults) {
        std::lock_guard<std::mutex> lock(_mutex);
        _maxResults = maxResults;
    }

    std::vector<std::shared_ptr<GeocodingResult> > PeliasOnlineGeocodingService::calculateAddresses(const std::shared_ptr<GeocodingRequest>& request) const {
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
            tagMap["api_key"] = NetworkUtils::URLEncode(_apiKey);
            tagMap["mode"] = _autocomplete ? "autocomplete": "search";

            baseURL = GeneralUtils::ReplaceTags(_serviceURL.empty() ? MAPZEN_SERVICE_URL : _serviceURL, tagMap);

            params["text"] = request->getQuery();

            if (request->isLocationDefined()) {
                MapPos wgs84Center = request->getProjection()->toWgs84(request->getLocation());
                params["focus.point.lat"] = boost::lexical_cast<std::string>(wgs84Center.getY());
                params["focus.point.lon"] = boost::lexical_cast<std::string>(wgs84Center.getX());
            }
            if (request->getLocationRadius() > 0 && !_autocomplete) {
                MapPos wgs84Center = request->getProjection()->toWgs84(request->getLocation());
                double radius = request->getLocationRadius();
                params["boundary.circle.lat"] = boost::lexical_cast<std::string>(wgs84Center.getY());
                params["boundary.circle.lon"] = boost::lexical_cast<std::string>(wgs84Center.getX());
                params["boundary.circle.radius"] = boost::lexical_cast<std::string>(radius);
            }

            if (!_language.empty()) {
                params["lang"] = _language;
            }

            params["size"] = boost::lexical_cast<std::string>(std::max(1, _maxResults));
        }

        std::string url = NetworkUtils::BuildURLFromParameters(baseURL, params);
        Log::Debugf("PeliasOnlineGeocodingService::calculateAddresses: Loading %s", url.c_str());

        std::string responseString;
        if (!NetworkUtils::GetHTTP(url, responseString, Log::IsShowDebug())) {
            throw NetworkException("Failed to fetch response");
        }
        return PeliasGeocodingProxy::ReadResponse(responseString, request->getProjection());
    }

    const std::string PeliasOnlineGeocodingService::MAPZEN_SERVICE_URL = "https://search.mapzen.com/v1/{mode}?api_key={api_key}";
}

#endif
