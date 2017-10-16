#if defined(_CARTO_VALHALLA_ROUTING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

#include "ValhallaOfflineRoutingService.h"
#include "components/Exceptions.h"
#include "routing/RouteMatchingRequest.h"
#include "routing/RouteMatchingResult.h"
#include "routing/ValhallaRoutingProxy.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <sqlite3pp.h>

namespace carto {

    ValhallaOfflineRoutingService::ValhallaOfflineRoutingService(const std::string& path) :
        _database(),
        _profile("pedestrian"),
        _mutex()
    {
        _database.reset(new sqlite3pp::database());
        if (_database->connect_v2(path.c_str(), SQLITE_OPEN_READONLY) != SQLITE_OK) {
            throw FileException("Failed to open routing database", path);
        }
    }

    ValhallaOfflineRoutingService::~ValhallaOfflineRoutingService() {
    }

    std::string ValhallaOfflineRoutingService::getProfile() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _profile;
    }

    void ValhallaOfflineRoutingService::setProfile(const std::string& profile) {
        std::lock_guard<std::mutex> lock(_mutex);
        _profile = profile;
    }

    std::shared_ptr<RouteMatchingResult> ValhallaOfflineRoutingService::matchRoute(const std::shared_ptr<RouteMatchingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        return ValhallaRoutingProxy::MatchRoute(std::vector<std::shared_ptr<sqlite3pp::database> > { _database }, getProfile(), request);
    }

    std::shared_ptr<RoutingResult> ValhallaOfflineRoutingService::calculateRoute(const std::shared_ptr<RoutingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        return ValhallaRoutingProxy::CalculateRoute(std::vector<std::shared_ptr<sqlite3pp::database> > { _database }, getProfile(), request);
    }

}

#endif
