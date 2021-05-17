#if defined(_CARTO_ROUTING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

#include "OSRMOfflineRoutingService.h"
#include "components/Exceptions.h"
#include "projections/Projection.h"
#include "routing/utils/OSRMRoutingProxy.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <osrm/Graph.h>
#include <osrm/Query.h>
#include <osrm/Result.h>
#include <osrm/Instruction.h>
#include <osrm/RouteFinder.h>

namespace carto {

    OSRMOfflineRoutingService::OSRMOfflineRoutingService(const std::string& path) :
        RoutingService(),
        _routeFinder()
    {
        osrm::Graph::Settings graphSettings;
        auto graph = std::make_shared<osrm::Graph>(graphSettings);
        try {
            if (!graph->import(path)) {
                throw FileException("Failed to import routing graph", path);
            }
        }
        catch (const std::exception& ex) {
            throw GenericException("Exception while importing routing graph", ex.what());
        }
        _routeFinder = std::make_shared<osrm::RouteFinder>(graph);
    }

    OSRMOfflineRoutingService::~OSRMOfflineRoutingService() {
    }

    std::string OSRMOfflineRoutingService::getProfile() const {
        return std::string();
    }

    void OSRMOfflineRoutingService::setProfile(const std::string& profile) {
        if (!profile.empty()) {
            throw GenericException("Only default profile supported");
        }
    }

    std::shared_ptr<RouteMatchingResult> OSRMOfflineRoutingService::matchRoute(const std::shared_ptr<RouteMatchingRequest>& request) const {
        throw GenericException("matchRoute not implemented for this RoutingService");
    }

    std::shared_ptr<RoutingResult> OSRMOfflineRoutingService::calculateRoute(const std::shared_ptr<RoutingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        return OSRMRoutingProxy::CalculateRoute(_routeFinder, request);
    }

}

#endif
