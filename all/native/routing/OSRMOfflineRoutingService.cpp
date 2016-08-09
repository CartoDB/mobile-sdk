#include "OSRMOfflineRoutingService.h"
#include "components/Exceptions.h"
#include "projections/Projection.h"
#include "projections/EPSG3857.h"
#include "routing/RoutingProxy.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "routing/RouteFinder.h"
#include "routing/RoutingGraph.h"
#include "routing/RoutingObjects.h"

namespace carto {

    OSRMOfflineRoutingService::OSRMOfflineRoutingService(const std::string& path) :
        _routeFinder()
    {
        Routing::RoutingGraph::Settings graphSettings;
        auto graph = std::make_shared<Routing::RoutingGraph>(graphSettings);
        try {
            if (!graph->import(path)) {
                throw FileException("Failed to import routing graph", path);
            }
        } catch (const std::exception& ex) {
            throw GenericException("Exception while importing routing graph", ex.what());
        }
        _routeFinder = std::make_shared<Routing::RouteFinder>(graph);
    }

    OSRMOfflineRoutingService::~OSRMOfflineRoutingService() {
    }

    std::shared_ptr<RoutingResult> OSRMOfflineRoutingService::calculateRoute(const std::shared_ptr<RoutingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        return RoutingProxy::CalculateRoute(_routeFinder, request);
    }

}
