#include "CartoOnlineRoutingService.h"
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
        std::shared_ptr<Projection> proj = request->getProjection();
        
        std::stringstream ss;
        ss << "http://api.nutiteq.com/routing/v1/" << _source << "/viaroute" << "?user_key=" << LicenseManager::GetInstance().getUserKey();
        for (std::size_t i = 0; i < request->getPoints().size(); i++) {
            MapPos p = proj->toWgs84(request->getPoints()[i]);
            ss << "&loc=" << p.getY() << "," << p.getX();
        }
        ss << "&instructions=true&alt=false&geometry=true&output=json";

        try {
            std::string url = ss.str();
            HTTPClient httpClient(false);
            return RoutingProxy::CalculateRoute(httpClient, url, request);
        } catch (const std::exception& ex) {
            Log::Errorf("CartoOnlineRoutingService::calculateRoute: Exception while calculating route: %s", ex.what());
        }
        return std::shared_ptr<RoutingResult>();
    }
    
}
