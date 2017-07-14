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
        _database()
    {
    }

    GeocodingPackageHandler::~GeocodingPackageHandler() {
    }

    std::shared_ptr<sqlite3pp::database> GeocodingPackageHandler::getGeocodingDatabase() {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        if (!_database) {
            _database = std::make_shared<sqlite3pp::database>();
            if (_database->connect_v2(_uncompressedFileName.c_str(), SQLITE_OPEN_READONLY) != SQLITE_OK) { // try locally uncompressed package first
                if (_database->connect_v2(_fileName.c_str(), SQLITE_OPEN_READONLY) != SQLITE_OK) { // assume that the package was not gzipped, so use original file
                    Log::Errorf("GeocodingPackageHandler::getGeocodingDatabase: Can not connect to database %s", _fileName.c_str());
                    _database.reset();
                }
            }
        }
        return _database;
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
