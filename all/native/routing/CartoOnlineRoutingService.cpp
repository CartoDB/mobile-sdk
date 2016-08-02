#include "CartoOnlineRoutingService.h"
#include "components/Exceptions.h"
#include "projections/Projection.h"
#include "routing/RoutingProxy.h"
#include "network/HTTPClient.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <boost/lexical_cast.hpp>

namespace carto {

    CartoOnlineRoutingService::CartoOnlineRoutingService(const std::string& source) :
        _source(source)
    {
    }

    CartoOnlineRoutingService::~CartoOnlineRoutingService() {
    }

    std::shared_ptr<RoutingResult> CartoOnlineRoutingService::calculateRoute(const std::shared_ptr<RoutingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        std::shared_ptr<Projection> proj = request->getProjection();
        
        std::string url = ROUTING_SERVICE_URL + _source + "/viaroute?instructions=true&alt=false&geometry=true&output=json";
        for (const MapPos& pos : request->getPoints()) {
            MapPos wgsPos = proj->toWgs84(pos);
            url += "&loc=" + boost::lexical_cast<std::string>(wgsPos.getY()) + "," + boost::lexical_cast<std::string>(wgsPos.getX());
        }

        HTTPClient httpClient(false);
        return RoutingProxy::CalculateRoute(httpClient, url, request);
    }

    const std::string CartoOnlineRoutingService::ROUTING_SERVICE_URL = "http://api.nutiteq.com/routing/v2/";
    
}
