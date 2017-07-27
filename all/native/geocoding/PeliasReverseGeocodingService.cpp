#if defined(_CARTO_GEOCODING_SUPPORT)

#include "PeliasReverseGeocodingService.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "geocoding/PeliasGeocodingProxy.h"
#include "projections/Projection.h"
#include "utils/NetworkUtils.h"
#include "utils/Log.h"

#include <map>

#include <boost/lexical_cast.hpp>

namespace carto {

    PeliasReverseGeocodingService::PeliasReverseGeocodingService(const std::string& apiKey) :
        _apiKey(apiKey)
    {
    }

    PeliasReverseGeocodingService::~PeliasReverseGeocodingService() {
    }

    std::vector<std::shared_ptr<GeocodingResult> > PeliasReverseGeocodingService::calculateAddresses(const std::shared_ptr<ReverseGeocodingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        std::map<std::string, std::string> params;
        params["api_key"] = _apiKey;

        MapPos point = request->getProjection()->toWgs84(request->getLocation());
        params["point.lat"] = boost::lexical_cast<std::string>(point.getY());
        params["point.lon"] = boost::lexical_cast<std::string>(point.getX());
        params["boundary.circle.lat"] = boost::lexical_cast<std::string>(point.getY());
        params["boundary.circle.lon"] = boost::lexical_cast<std::string>(point.getX());
        params["boundary.circle.radius"] = boost::lexical_cast<std::string>(request->getSearchRadius());

        std::string url = NetworkUtils::BuildURLFromParameters(PELIAS_REVERSE_URL, params);
        Log::Debugf("PeliasReverseGeocodingService::calculateAddresses: Loading %s", url.c_str());

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

    const std::string PeliasReverseGeocodingService::PELIAS_REVERSE_URL = "https://search.mapzen.com/v1/reverse";
}

#endif
