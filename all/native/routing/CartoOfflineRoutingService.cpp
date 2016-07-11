#include "CartoOfflineRoutingService.h"
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

    CartoOfflineRoutingService::CartoOfflineRoutingService(const std::string& path) :
        _routeFinder()
    {
        Routing::RoutingGraph::Settings graphSettings;
        auto graph = std::make_shared<Routing::RoutingGraph>(graphSettings);
        try {
            if (!graph->import(path)) {
                throw FileException("Failed to import routing graph", path);
            }
        } catch (const std::exception& ex) {
            throw FileException("Exception while importing routing graph", path);
        }
        _routeFinder = std::make_shared<Routing::RouteFinder>(graph);
    }

    CartoOfflineRoutingService::~CartoOfflineRoutingService() {
    }

    std::shared_ptr<RoutingResult> CartoOfflineRoutingService::calculateRoute(const std::shared_ptr<RoutingRequest>& request) const {
        if (!request) {
            throw std::invalid_argument("Null request");
        }

        if (!_routeFinder) {
            Log::Errorf("CartoOfflineRoutingService::calculateRoute: Router not initialized");
            return std::shared_ptr<RoutingResult>();
        }

        try {
            return RoutingProxy::CalculateRoute(_routeFinder, request);
        }
        catch (const std::exception& ex) {
            Log::Errorf("CartoOfflineRoutingService::calculateRoute: Exception while calculating route: %s", ex.what());
            return std::shared_ptr<RoutingResult>();
        }
        return std::shared_ptr<RoutingResult>();
    }

}
