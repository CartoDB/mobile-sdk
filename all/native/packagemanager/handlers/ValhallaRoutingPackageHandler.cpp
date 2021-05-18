#if defined(_CARTO_VALHALLA_ROUTING_SUPPORT) && defined(_CARTO_PACKAGEMANAGER_SUPPORT)

#include "ValhallaRoutingPackageHandler.h"
#include "packagemanager/PackageTileMask.h"
#include "utils/Log.h"

#include <stdext/zlib.h>

#include <sqlite3pp.h>

namespace carto {

    ValhallaRoutingPackageHandler::ValhallaRoutingPackageHandler(const std::string& fileName) :
        PackageHandler(fileName),
        _packageDb()
    {
    }

    ValhallaRoutingPackageHandler::~ValhallaRoutingPackageHandler() {
    }

    std::shared_ptr<sqlite3pp::database> ValhallaRoutingPackageHandler::getDatabase() {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        if (!_packageDb) {
            try {
                // Open package database
                _packageDb = std::make_unique<sqlite3pp::database>();
                if (_packageDb->connect_v2(_fileName.c_str(), SQLITE_OPEN_READONLY) != SQLITE_OK) {
                    Log::Errorf("ValhallaRoutingPackageHandler::getDatabase: Could not open database %s", _fileName.c_str());
                    _packageDb.reset();
                }
                if (_packageDb) {
                    _packageDb->execute("PRAGMA temp_store=MEMORY");
                    _packageDb->execute("PRAGMA cache_size=256");
                }
            }
            catch (const std::exception& ex) {
                Log::Errorf("ValhallaRoutingPackageHandler::getDatabase: Exception %s", ex.what());
                _packageDb.reset();
            }
        }
        return _packageDb;
    }

    void ValhallaRoutingPackageHandler::onImportPackage() {
    }

    void ValhallaRoutingPackageHandler::onDeletePackage() {
    }

    std::shared_ptr<PackageTileMask> ValhallaRoutingPackageHandler::calculateTileMask() const {
        return std::shared_ptr<PackageTileMask>();
    }

}

#endif
