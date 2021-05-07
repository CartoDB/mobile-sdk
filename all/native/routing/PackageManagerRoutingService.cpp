#if defined(_CARTO_ROUTING_SUPPORT) && defined(_CARTO_PACKAGEMANAGER_SUPPORT)

#include "PackageManagerRoutingService.h"
#include "components/Exceptions.h"
#include "packagemanager/PackageInfo.h"
#include "packagemanager/handlers/RoutingPackageHandler.h"
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

    PackageManagerRoutingService::PackageManagerRoutingService(const std::shared_ptr<PackageManager>& packageManager) :
        RoutingService(),
        _packageManager(packageManager),
        _cachedPackageFileMap(),
        _cachedRouteFinder(),
        _mutex()
    {
        if (!packageManager) {
            throw NullArgumentException("Null packageManager");
        }

        _packageManagerListener = std::make_shared<PackageManagerListener>(*this);
        _packageManager->registerOnChangeListener(_packageManagerListener);
    }

    PackageManagerRoutingService::~PackageManagerRoutingService() {
        _packageManager->unregisterOnChangeListener(_packageManagerListener);
        _packageManagerListener.reset();
    }

    std::string PackageManagerRoutingService::getProfile() const {
        return std::string();
    }

    void PackageManagerRoutingService::setProfile(const std::string& profile) {
        if (!profile.empty()) {
            throw GenericException("Only default profile supported");
        }
    }

    std::shared_ptr<RouteMatchingResult> PackageManagerRoutingService::matchRoute(const std::shared_ptr<RouteMatchingRequest>& request) const {
        throw GenericException("matchRoute not implemented for this RoutingService");
    }

    std::shared_ptr<RoutingResult> PackageManagerRoutingService::calculateRoute(const std::shared_ptr<RoutingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        // Do routing via package manager, so that all packages are locked during routing
        std::shared_ptr<RoutingResult> result;
        _packageManager->accessLocalPackages([this, &result, &request](const std::map<std::shared_ptr<PackageInfo>, std::shared_ptr<PackageHandler> >& packageHandlerMap) {
            // Build map of routing packages and graph files
            std::map<std::shared_ptr<PackageInfo>, std::shared_ptr<std::ifstream> > packageFileMap;
            for (auto it = packageHandlerMap.begin(); it != packageHandlerMap.end(); it++) {
                if (auto routingHandler = std::dynamic_pointer_cast<RoutingPackageHandler>(it->second)) {
                    if (std::shared_ptr<std::ifstream> graphFile = routingHandler->getGraphFile()) {
                        packageFileMap[it->first] = graphFile;
                    }
                }
            }

            // Now check if we have already a cached route finder for the files. If not, create new instance.
            std::shared_ptr<osrm::RouteFinder> routeFinder;
            {
                std::lock_guard<std::mutex> lock(_mutex);
                if (!_cachedRouteFinder || packageFileMap != _cachedPackageFileMap) {
                    osrm::Graph::Settings graphSettings;
                    auto graph = std::make_shared<osrm::Graph>(graphSettings);
                    for (auto it = packageFileMap.begin(); it != packageFileMap.end(); it++) {
                        try {
                            if (!graph->import(it->second)) {
                                throw FileException("Failed to import graph " + it->first->getPackageId(), "");
                            }
                        }
                        catch (const std::exception& ex) {
                            throw GenericException("Exception while importing graph " + it->first->getPackageId(), ex.what());
                        }
                    }
                    _cachedPackageFileMap = packageFileMap;
                    _cachedRouteFinder = std::make_shared<osrm::RouteFinder>(graph);
                }
                routeFinder = _cachedRouteFinder;
            }

            result = OSRMRoutingProxy::CalculateRoute(routeFinder, request);
        });

        return result;
    }
            
    PackageManagerRoutingService::PackageManagerListener::PackageManagerListener(PackageManagerRoutingService& service) :
        _service(service)
    {
    }
        
    void PackageManagerRoutingService::PackageManagerListener::onPackagesChanged() {
        std::lock_guard<std::mutex> lock(_service._mutex);
        _service._cachedPackageFileMap.clear();
        _service._cachedRouteFinder.reset();
    }

    void PackageManagerRoutingService::PackageManagerListener::onStylesChanged() {
        // Impossible
    }

}

#endif
