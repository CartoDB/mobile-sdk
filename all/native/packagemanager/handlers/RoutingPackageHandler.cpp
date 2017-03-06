#include "RoutingPackageHandler.h"
#include "packagemanager/PackageTileMask.h"
#include "utils/Log.h"

namespace carto {

    RoutingPackageHandler::RoutingPackageHandler(const std::string& fileName) :
        PackageHandler(fileName),
        _graphFile()
    {
    }

    RoutingPackageHandler::~RoutingPackageHandler() {
    }

    std::shared_ptr<std::ifstream> RoutingPackageHandler::getGraphFile() {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        if (!_graphFile) {
            try {
                _graphFile = std::make_shared<std::ifstream>();
                _graphFile->exceptions(std::ifstream::failbit | std::ifstream::badbit);
                _graphFile->rdbuf()->pubsetbuf(0, 0);
                _graphFile->open(_fileName, std::ios::binary);
            }
            catch (const std::exception& ex) {
                Log::Errorf("RoutingPackageHandler::getGraphFile: Failed to open graph file %s", _fileName.c_str());
            }
        }
        return _graphFile;
    }

    void RoutingPackageHandler::importPackage() {
    }

    std::shared_ptr<PackageTileMask> RoutingPackageHandler::calculateTileMask() const {
        return std::shared_ptr<PackageTileMask>();
    }

}
