#if defined(_CARTO_ROUTING_SUPPORT) && defined(_CARTO_VALHALLA_ROUTING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

#include "ValhallaOfflineRoutingService.h"
#include "components/Exceptions.h"
#include "routing/ValhallaRoutingProxy.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <boost/algorithm/string.hpp>

#include <sqlite3pp.h>

namespace carto {

    ValhallaOfflineRoutingService::ValhallaOfflineRoutingService(const std::string& path) :
        _database(),
        _profile("pedestrian"),
        _configuration(ValhallaRoutingProxy::GetDefaultConfiguration()),
        _mutex()
    {
        _database.reset(new sqlite3pp::database());
        if (_database->connect_v2(path.c_str(), SQLITE_OPEN_READONLY) != SQLITE_OK) {
            throw FileException("Failed to open routing database", path);
        }
    }

    ValhallaOfflineRoutingService::~ValhallaOfflineRoutingService() {
    }

    Variant ValhallaOfflineRoutingService::getConfigurationParameter(const std::string& param) const {
        std::lock_guard<std::mutex> lock(_mutex);
        std::vector<std::string> keys;
        boost::split(keys, param, boost::is_any_of("."));
        picojson::value subValue = _configuration.toPicoJSON();
        for (const std::string& key : keys) {
            if (!subValue.is<picojson::object>()) {
                return Variant();
            }
            subValue = subValue.get(key);
        }
        return Variant::FromPicoJSON(subValue);
    }

    void ValhallaOfflineRoutingService::setConfigurationParameter(const std::string& param, const Variant& value) {
        std::lock_guard<std::mutex> lock(_mutex);
        std::vector<std::string> keys;
        boost::split(keys, param, boost::is_any_of("."));
        picojson::value config = _configuration.toPicoJSON();
        picojson::value* subValue = &config;
        for (const std::string& key : keys) {
            if (!subValue->is<picojson::object>()) {
                subValue->set(picojson::object());
            }
            subValue = &subValue->get<picojson::object>()[key];
        }
        *subValue = value.toPicoJSON();
        _configuration = Variant::FromPicoJSON(config);
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

        std::lock_guard<std::mutex> lock(_mutex);
        return ValhallaRoutingProxy::MatchRoute(std::vector<std::shared_ptr<sqlite3pp::database> > { _database }, _profile, _configuration, request);
    }

    std::shared_ptr<RoutingResult> ValhallaOfflineRoutingService::calculateRoute(const std::shared_ptr<RoutingRequest>& request) const {
        if (!request) {
            throw NullArgumentException("Null request");
        }

        std::lock_guard<std::mutex> lock(_mutex);
        return ValhallaRoutingProxy::CalculateRoute(std::vector<std::shared_ptr<sqlite3pp::database> > { _database }, _profile, _configuration, request);
    }
}

#endif
