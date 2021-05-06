#if defined(_CARTO_GEOCODING_SUPPORT)

#include "MapBoxOnlineGeocodingService.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "geocoding/utils/MapBoxGeocodingProxy.h"
#include "projections/Projection.h"
#include "projections/EPSG3857.h"
#include "utils/Const.h"
#include "utils/GeneralUtils.h"
#include "utils/NetworkUtils.h"
#include "utils/Log.h"

#include <cmath>
#include <algorithm>

#include <boost/lexical_cast.hpp>

namespace carto {

    MapBoxOnlineGeocodingService::MapBoxOnlineGeocodingService(const std::string& accessToken) :
        _accessToken(accessToken),
        _autocomplete(false),
        _language(),
        _maxResults(10),
        _serviceURL(),
        _mutex()
    {
    }

    MapBoxOnlineGeocodingService::~MapBoxOnlineGeocodingService() {
    }

    std::string MapBoxOnlineGeocodingService::getCustomServiceURL() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _serviceURL;
    }

    void MapBoxOnlineGeocodingService::setCustomServiceURL(const std::string& serviceURL) {
        std::lock_guard<std::mutex> lock(_mutex);
        _serviceURL = serviceURL;
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

    int MapBoxOnlineGeocodingService::getMaxResults() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _maxResults;
    }

    void MapBoxOnlineGeocodingService::setMaxResults(int maxResults) {
        std::lock_guard<std::mutex> lock(_mutex);
        _maxResults = maxResults;
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

            params["limit"] = boost::lexical_cast<std::string>(std::max(1, std::min(10, _maxResults)));
        }

        if (request->isLocationDefined()) {
            MapPos wgs84Center = request->getProjection()->toWgs84(request->getLocation());
            params["proximity"] = boost::lexical_cast<std::string>(wgs84Center.getX()) + "," + boost::lexical_cast<std::string>(wgs84Center.getY());
        }
        if (request->getLocationRadius() > 0) {
            EPSG3857 epsg3857;
            MapPos wgs84Center = request->getProjection()->toWgs84(request->getLocation());
            double mercRadius = request->getLocationRadius() / std::cos(std::min(89.9, std::abs(wgs84Center.getY())) * Const::DEG_TO_RAD);
            MapPos mercPos0 = epsg3857.fromWgs84(wgs84Center) - MapVec(mercRadius, mercRadius);
            MapPos mercPos1 = epsg3857.fromWgs84(wgs84Center) + MapVec(mercRadius, mercRadius);
            mercPos0[0] = std::max(mercPos0[0], epsg3857.getBounds().getMin()[0] * 0.9999);
            mercPos1[0] = std::min(mercPos1[0], epsg3857.getBounds().getMax()[0] * 0.9999);
            MapPos wgs84Pos0 = epsg3857.toWgs84(mercPos0);
            MapPos wgs84Pos1 = epsg3857.toWgs84(mercPos1);
            params["bbox"] = boost::lexical_cast<std::string>(wgs84Pos0.getX()) + "," + boost::lexical_cast<std::string>(wgs84Pos0.getY()) + "," + boost::lexical_cast<std::string>(wgs84Pos1.getX()) + "," + boost::lexical_cast<std::string>(wgs84Pos1.getY());
        }

        std::string url = NetworkUtils::BuildURLFromParameters(baseURL, params);
        Log::Debugf("MapBoxOnlineGeocodingService::calculateAddresses: Loading %s", url.c_str());

        std::string responseString;
        if (!NetworkUtils::GetHTTP(url, responseString, Log::IsShowDebug())) {
            throw NetworkException("Failed to fetch response");
        }
        return MapBoxGeocodingProxy::ReadResponse(responseString, request->getProjection());
    }

    const std::string MapBoxOnlineGeocodingService::MAPBOX_SERVICE_URL = "https://api.mapbox.com/geocoding/v5/mapbox.places-permanent/{query}.json?access_token={access_token}";
}

#endif
