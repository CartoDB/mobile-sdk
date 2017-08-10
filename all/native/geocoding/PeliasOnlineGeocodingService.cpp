#if defined(_CARTO_GEOCODING_SUPPORT)

#include "PeliasOnlineGeocodingService.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "geocoding/PeliasGeocodingProxy.h"
#include "projections/Projection.h"
#include "utils/NetworkUtils.h"
#include "utils/Log.h"

#include <map>

#include <boost/lexical_cast.hpp>

namespace carto {

    PeliasOnlineGeocodingService::PeliasOnlineGeocodingService(const std::string& apiKey) :
        _apiKey(apiKey),
        _autocomplete(false),
        _mutex()
    {
    }

    PeliasOnlineGeocodingService::~PeliasOnlineGeocodingService() {
    }

    bool PeliasOnlineGeocodingService::isAutocomplete() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _autocomplete;
    }

    void PeliasOnlineGeocodingService::setAutocomplete(bool autocomplete) {
        std::lock_guard<std::mutex> lock(_mutex);
        _autocomplete = autocomplete;
    }

    std::vector<std::shared_ptr<GeocodingResult> > PeliasOnlineGeocodingService::calculateAddresses(const std::shared_ptr<GeocodingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        std::string baseURL;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            baseURL = (_autocomplete ? PELIAS_AUTOCOMPLETE_URL : PELIAS_SEARCH_URL);
        }

        std::map<std::string, std::string> params;
        params["api_key"] = _apiKey;
        params["text"] = request->getQuery();
        if (request->getLocationRadius() > 0) {
            MapPos focusPoint = request->getProjection()->toWgs84(request->getLocation());
            params["focus.point.lat"] = boost::lexical_cast<std::string>(focusPoint.getY());
            params["focus.point.lon"] = boost::lexical_cast<std::string>(focusPoint.getX());
            if (baseURL == PELIAS_SEARCH_URL) {
                params["boundary.circle.lat"] = boost::lexical_cast<std::string>(focusPoint.getY());
                params["boundary.circle.lon"] = boost::lexical_cast<std::string>(focusPoint.getX());
                params["boundary.circle.radius"] = boost::lexical_cast<std::string>(request->getLocationRadius());
            }
        }

        std::string url = NetworkUtils::BuildURLFromParameters(baseURL, params);
        Log::Debugf("PeliasOnlineGeocodingService::calculateAddresses: Loading %s", url.c_str());

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

        return PeliasGeocodingProxy::ReadResponse(responseString, request->getProjection());
    }

    const std::string PeliasOnlineGeocodingService::PELIAS_AUTOCOMPLETE_URL = "https://search.mapzen.com/v1/autocomplete";
    const std::string PeliasOnlineGeocodingService::PELIAS_SEARCH_URL = "https://search.mapzen.com/v1/search";
}

#endif
