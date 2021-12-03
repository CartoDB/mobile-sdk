#if defined(_CARTO_GEOCODING_SUPPORT) && defined(_CARTO_PACKAGEMANAGER_SUPPORT)

#include "GeocodingPackageHandler.h"
#include "packagemanager/PackageTileMask.h"
#include "utils/Log.h"

#include <stdext/utf8_filesystem.h>
#include <stdext/ungzip.h>

#include <sqlite3pp.h>

namespace carto {

    GeocodingPackageHandler::GeocodingPackageHandler(const std::string& fileName) :
        PackageHandler(fileName),
        _uncompressedFileName(fileName + ".uncompressed"),
        _packageDb()
    {
    }

    GeocodingPackageHandler::~GeocodingPackageHandler() {
    }

    std::shared_ptr<sqlite3pp::database> GeocodingPackageHandler::getDatabase() {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        if (!_packageDb) {
            try {
                // Open package database
                _packageDb = std::make_shared<sqlite3pp::database>();
                if (_packageDb->connect_v2(_uncompressedFileName.c_str(), SQLITE_OPEN_READONLY | SQLITE_OPEN_FULLMUTEX) != SQLITE_OK) { // try locally uncompressed package first
                    if (_packageDb->connect_v2(_fileName.c_str(), SQLITE_OPEN_READONLY | SQLITE_OPEN_FULLMUTEX) != SQLITE_OK) { // assume that the package was not gzipped, so use original file
                        Log::Errorf("GeocodingPackageHandler::getDatabase: Can not connect to database %s", _fileName.c_str());
                        _packageDb.reset();
                    }
                }
                if (_packageDb) {
                    _packageDb->execute("PRAGMA temp_store=MEMORY");
                    _packageDb->execute("PRAGMA cache_size=256");
                }
            }
            catch (const std::exception& ex) {
                Log::Errorf("GeocodingPackageHandler::getDatabase: Exception %s", ex.what());
                _packageDb.reset();
            }
        }
        return _packageDb;
    }

    void GeocodingPackageHandler::onImportPackage() {
        std::shared_ptr<FILE> fpIn(utf8_filesystem::fopen(_fileName.c_str(), "rb"), fclose);
        std::shared_ptr<FILE> fpOut(utf8_filesystem::fopen(_uncompressedFileName.c_str(), "wb"), fclose);
        if (!zlib::ungzip_file(fpIn.get(), fpOut.get())) {
            fpOut.reset();
            utf8_filesystem::unlink(_uncompressedFileName.c_str());
        }
    }

    void GeocodingPackageHandler::onDeletePackage() {
        utf8_filesystem::unlink(_uncompressedFileName.c_str());
    }

    std::shared_ptr<PackageTileMask> GeocodingPackageHandler::calculateTileMask() const {
        return std::shared_ptr<PackageTileMask>();
    }

}

#endif
