#ifdef _CARTO_PACKAGEMANAGER_SUPPORT

#include "PackageManagerRoutingService.h"
#include "components/Exceptions.h"
#include "packagemanager/PackageManager.h"
#include "packagemanager/PackageInfo.h"
#include "projections/Projection.h"
#include "projections/EPSG3857.h"
#include "routing/RoutingProxy.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "routing/RouteFinder.h"
#include "routing/RoutingGraph.h"
#include "routing/RoutingObjects.h"

namespace carto {

    PackageManagerRoutingService::PackageManagerRoutingService(const std::shared_ptr<PackageManager>& packageManager) :
        _packageManager(packageManager),
        _cachedPackageFileMap(),
        _cachedRouteFinder(),
        _mutex()
    {
        if (!packageManager) {
            throw NullArgumentException("Null packageManager");
        }
    }

    PackageManagerRoutingService::~PackageManagerRoutingService() {
    }

    std::shared_ptr<RoutingResult> PackageManagerRoutingService::calculateRoute(const std::shared_ptr<RoutingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        // Find all routing packages
        std::vector<std::string> packageIds;
        for (const std::shared_ptr<PackageInfo>& localPackage : _packageManager->getLocalPackages()) {
            if (localPackage->getPackageType() == PackageType::PACKAGE_TYPE_ROUTING) {
                packageIds.push_back(localPackage->getPackageId());
            }
        }

        // Call router via package manager
        std::shared_ptr<RoutingResult> result;
        _packageManager->accessPackageFiles(packageIds, [this, request, &result](const std::map<std::string, std::shared_ptr<std::ifstream> >& packageFileMap) {
            std::lock_guard<std::mutex> lock(_mutex);
            if (packageFileMap != _cachedPackageFileMap || !_cachedRouteFinder) {
                Routing::RoutingGraph::Settings graphSettings;
                auto graph = std::make_shared<Routing::RoutingGraph>(graphSettings);
                for (auto it = packageFileMap.begin(); it != packageFileMap.end(); it++) {
                    try {
                        if (!graph->import(it->second)) {
                            throw FileException("Failed to import graph " + it->first, "");
                        }
                    }
                    catch (const std::exception& ex) {
                        throw GenericException("Exception while importing graph" + it->first, ex.what());
                    }
                }
                _cachedPackageFileMap = packageFileMap;
                _cachedRouteFinder = std::make_shared<Routing::RouteFinder>(graph);
            }

            result = RoutingProxy::CalculateRoute(_cachedRouteFinder, request);
        });
        return result;
    }
            
}

#endif
