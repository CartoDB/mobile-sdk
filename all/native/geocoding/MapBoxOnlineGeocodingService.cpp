#if defined(_CARTO_GEOCODING_SUPPORT)

#include "MapBoxOnlineGeocodingService.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "geocoding/MapBoxGeocodingProxy.h"
#include "projections/Projection.h"
#include "utils/GeneralUtils.h"
#include "utils/NetworkUtils.h"
#include "utils/Log.h"

#include <map>

#include <boost/lexical_cast.hpp>

namespace carto {

    MapBoxOnlineGeocodingService::MapBoxOnlineGeocodingService(const std::string& accessToken) :
        _accessToken(accessToken),
        _autocomplete(false),
        _language(),
        _serviceURL(),
        _mutex()
    {
    }

    MapBoxOnlineGeocodingService::~MapBoxOnlineGeocodingService() {
    }

    bool MapBoxOnlineGeocodingService::isAutocomplete() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _autocomplete;
    }

    void MapBoxOnlineGeocodingService::setAutocomplete(bool autocomplete) {
        std::lock_guard<std::mutex> lock(_mutex);
        _autocomplete = autocomplete;
    }

    std::string MapBoxOnlineGeocodingService::getLanguage() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _language;
    }

    void MapBoxOnlineGeocodingService::setLanguage(const std::string& lang) {
        std::lock_guard<std::mutex> lock(_mutex);
        _language = lang;
    }

    std::string MapBoxOnlineGeocodingService::getCustomServiceURL() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _serviceURL;
    }

    void MapBoxOnlineGeocodingService::setCustomServiceURL(const std::string& serviceURL) {
        std::lock_guard<std::mutex> lock(_mutex);
        _serviceURL = serviceURL;
    }

    std::vector<std::shared_ptr<GeocodingResult> > MapBoxOnlineGeocodingService::calculateAddresses(const std::shared_ptr<GeocodingRequest>& request) const {
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
            tagMap["access_token"] = NetworkUtils::URLEncode(_accessToken);

            baseURL = GeneralUtils::ReplaceTags(_serviceURL.empty() ? MAPBOX_SERVICE_URL : _serviceURL, tagMap);

            params["autocomplete"] = _autocomplete ? "true" : "false";
            if (!_language.empty()) {
                params["language"] = _language;
            }
        }

        if (request->getLocationRadius() > 0) {
            MapPos focusPoint = request->getProjection()->toWgs84(request->getLocation());
            params["proximity"] = boost::lexical_cast<std::string>(focusPoint.getX()) + "," + boost::lexical_cast<std::string>(focusPoint.getY());
            // TODO: bbox
        }

        std::string url = NetworkUtils::BuildURLFromParameters(baseURL, params);
        Log::Debugf("MapBoxOnlineGeocodingService::calculateAddresses: Loading %s", url.c_str());

        std::shared_ptr<BinaryData> responseData;
        if (!NetworkUtils::GetHTTP(url, responseData, Log::IsShowDebug())) {
            throw NetworkException("Failed to fetch response");
        }

        std::string responseString;
        if (responseData) {
            responseString = std::string(reinterpret_cast<const char*>(responseData->data()), responseData->size());
        } else {
            throw GenericException("Empty response");
        }

        return MapBoxGeocodingProxy::ReadResponse(responseString, request->getProjection());
    }

    const std::string MapBoxOnlineGeocodingService::MAPBOX_SERVICE_URL = "https://api.mapbox.com/geocoding/v5/mapbox.places/{query}.json&access_token={access_token}";
}

#endif
