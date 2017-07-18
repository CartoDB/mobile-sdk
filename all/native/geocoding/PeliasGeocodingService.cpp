#if defined(_CARTO_GEOCODING_SUPPORT)

#include "PeliasGeocodingService.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "geocoding/PeliasGeocodingProxy.h"
#include "projections/Projection.h"
#include "utils/NetworkUtils.h"
#include "utils/Log.h"

#include <map>

#include <boost/lexical_cast.hpp>

namespace carto {

    PeliasGeocodingService::PeliasGeocodingService(const std::string& apiKey) :
        _apiKey(apiKey),
        _httpClient(Log::IsShowDebug()),
        _autocomplete(false),
        _mutex()
    {
    }

    PeliasGeocodingService::~PeliasGeocodingService() {
    }

    bool PeliasGeocodingService::isAutocomplete() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _autocomplete;
    }

    void PeliasGeocodingService::setAutocomplete(bool autocomplete) {
        std::lock_guard<std::mutex> lock(_mutex);
        _autocomplete = autocomplete;
    }

    std::vector<std::shared_ptr<GeocodingResult> > PeliasGeocodingService::calculateAddresses(const std::shared_ptr<GeocodingRequest>& request) const {
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
        Log::Debugf("PeliasGeocodingService::calculateAddresses: Loading %s", url.c_str());

        std::shared_ptr<BinaryData> responseData;
        if (!NetworkUtils::GetHTTP(url, responseData, Log::IsShowDebug())) {
            Log::Warnf("PeliasGeocodingService: Failed to fetch tile response"); // NOTE: we may have error messages, thus do not return from here
        }

        std::string responseString;
        if (responseData) {
            responseString = std::string(reinterpret_cast<const char*>(responseData->data()), responseData->size());
        } else {
            Log::Error("PeliasGeocodingService: Empty response");
            throw GenericException("Empty response");
        }

        return PeliasGeocodingProxy::ReadResponse(responseString, request->getProjection());
    }

    const std::string PeliasGeocodingService::PELIAS_AUTOCOMPLETE_URL = "https://search.mapzen.com/v1/autocomplete";
    const std::string PeliasGeocodingService::PELIAS_SEARCH_URL = "https://search.mapzen.com/v1/search";
}

#endif
