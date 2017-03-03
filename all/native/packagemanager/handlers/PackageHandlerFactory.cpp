#include "PackageHandlerFactory.h"
#include "packagemanager/handlers/MapPackageHandler.h"
#include "packagemanager/handlers/RoutingPackageHandler.h"
#include "utils/Log.h"

namespace carto {

    PackageHandlerFactory::PackageHandlerFactory(const std::string& serverEncKey, const std::string& localEncKey) :
        _serverEncKey(serverEncKey),
        _localEncKey(localEncKey)
    {
    }

    PackageHandlerFactory::~PackageHandlerFactory() {
    }

    std::shared_ptr<PackageHandler> PackageHandlerFactory::createPackageHandler(PackageType::PackageType packageType, const std::string& filePath) const {
        switch (packageType) {
        case PackageType::PACKAGE_TYPE_MAP:
            return std::make_shared<MapPackageHandler>(filePath, _serverEncKey, _localEncKey);
        case PackageType::PACKAGE_TYPE_ROUTING:
            return std::make_shared<RoutingPackageHandler>(filePath);
        default:
            Log::Warnf("PackageHandlerFactory::CreatePackageHandler: Unsupported package type");
            return std::shared_ptr<PackageHandler>();
        }
    }
    
    std::string PackageHandlerFactory::GetPackageTypeExtension(PackageType::PackageType packageType) {
        switch (packageType) {
        case PackageType::PACKAGE_TYPE_MAP:
            return ".mbtiles";
        case PackageType::PACKAGE_TYPE_ROUTING:
            return ".nutigraph";
        default:
            Log::Warnf("PackageHandlerFactory::GetPackageTypeExtension: Unsupported package type");
            return "";
        }
    }

    PackageType::PackageType PackageHandlerFactory::DetectPackageType(const std::string& fileName) {
        auto fileNameEndsWith = [&fileName](const std::string& ext) {
            return fileName.size() >= ext.size() && fileName.substr(fileName.size() - ext.size()) == ext;
        };

        if (fileNameEndsWith(".mbtiles")) {
            return PackageType::PACKAGE_TYPE_MAP;
        }
        else if (fileNameEndsWith(".nutigraph")) {
            return PackageType::PACKAGE_TYPE_ROUTING;
        }
        Log::Warnf("PackageHandlerFactory::DetectPackageType: Unexpected package file: %s", fileName.c_str());
        return PackageType::PACKAGE_TYPE_MAP; // sensible default
    }

}
