#include "CartoOnlineRoutingService.h"
#include "components/Exceptions.h"
#include "components/LicenseManager.h"
#include "projections/Projection.h"
#include "routing/RoutingProxy.h"
#include "network/HTTPClient.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <sstream>

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
        
        std::stringstream ss;
        ss << ROUTING_SERVICE_URL << _source << "/viaroute" << "?user_key=" << LicenseManager::GetInstance().getUserKey();
        for (std::size_t i = 0; i < request->getPoints().size(); i++) {
            MapPos p = proj->toWgs84(request->getPoints()[i]);
            ss << "&loc=" << p.getY() << "," << p.getX();
        }
        ss << "&instructions=true&alt=false&geometry=true&output=json";
        std::string url = ss.str();

        HTTPClient httpClient(false);
        return RoutingProxy::CalculateRoute(httpClient, url, request);
    }

    const std::string CartoOnlineRoutingService::ROUTING_SERVICE_URL = "http://api.nutiteq.com/routing/v1/";
    
}
