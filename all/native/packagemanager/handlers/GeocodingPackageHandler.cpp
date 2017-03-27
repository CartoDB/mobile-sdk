#if defined(_CARTO_GEOCODING_SUPPORT) && defined(_CARTO_PACKAGEMANAGER_SUPPORT)

#include "GeocodingPackageHandler.h"
#include "packagemanager/PackageTileMask.h"
#include "utils/Log.h"

#include <sqlite3pp.h>

namespace carto {

    GeocodingPackageHandler::GeocodingPackageHandler(const std::string& fileName) :
        PackageHandler(fileName),
        _database()
    {
    }

    GeocodingPackageHandler::~GeocodingPackageHandler() {
    }

    std::shared_ptr<sqlite3pp::database> GeocodingPackageHandler::getGeocodingDatabase() {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        if (!_database) {
            _database = std::make_shared<sqlite3pp::database>();
            if (_database->connect_v2(_fileName.c_str(), SQLITE_OPEN_READONLY) != SQLITE_OK) {
                Log::Errorf("GeocodingPackageHandler::getGeocodingDatabase: Can not connect to database %s", _fileName.c_str());
                _database.reset();
            }
        }
        return _database;
    }

    void GeocodingPackageHandler::importPackage() {
    }

    std::shared_ptr<PackageTileMask> GeocodingPackageHandler::calculateTileMask() const {
        return std::shared_ptr<PackageTileMask>();
    }

}

#endif
