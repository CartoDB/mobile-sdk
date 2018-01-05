#if defined(_CARTO_GEOCODING_SUPPORT)

#include "MapBoxOnlineReverseGeocodingService.h"
#include "core/BinaryData.h"
#include "components/Exceptions.h"
#include "geocoding/MapBoxGeocodingProxy.h"
#include "projections/Projection.h"
#include "utils/NetworkUtils.h"
#include "utils/Log.h"

#include <map>

#include <boost/lexical_cast.hpp>

namespace carto {

    MapBoxOnlineReverseGeocodingService::MapBoxOnlineReverseGeocodingService(const std::string& apiKey) :
        _apiKey(apiKey),
        _mutex()
    {
    }

    MapBoxOnlineReverseGeocodingService::~MapBoxOnlineReverseGeocodingService() {
    }

    std::vector<std::shared_ptr<GeocodingResult> > MapBoxOnlineReverseGeocodingService::calculateAddresses(const std::shared_ptr<ReverseGeocodingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        MapPos point = request->getProjection()->toWgs84(request->getLocation());
        std::string baseURL = MAPBOX_SERVICE_URL + NetworkUtils::URLEncode(boost::lexical_cast<std::string>(point.getX()) + "," + boost::lexical_cast<std::string>(point.getY())) + ".json";

        std::map<std::string, std::string> params;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            params["access_token"] = _apiKey;
            // TODO: language
        }

        std::string url = NetworkUtils::BuildURLFromParameters(baseURL, params);
        Log::Debugf("MapBoxOnlineReverseGeocodingService::calculateAddresses: Loading %s", url.c_str());

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

    const std::string MapBoxOnlineReverseGeocodingService::MAPBOX_SERVICE_URL = "https://api.mapbox.com/geocoding/v5/mapbox.places/";
}

#endif
