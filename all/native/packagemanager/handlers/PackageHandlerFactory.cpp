#include "PackageHandlerFactory.h"
#include "packagemanager/handlers/MapPackageHandler.h"
#include "packagemanager/handlers/RoutingPackageHandler.h"
#include "packagemanager/handlers/GeocodingPackageHandler.h"
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
        case PackageType::PACKAGE_TYPE_GEOCODING:
            return std::make_shared<GeocodingPackageHandler>(filePath);
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
        case PackageType::PACKAGE_TYPE_GEOCODING:
            return ".nutigeodb";
        default:
            Log::Warnf("PackageHandlerFactory::GetPackageTypeExtension: Unsupported package type");
            return "";
        }
    }

    PackageType::PackageType PackageHandlerFactory::DetectPackageType(const std::string& url) {
        auto fileNameEndsWith = [&url](const std::string& ext) {
            std::string fileName = url;
            std::string::size_type pos = fileName.find('?');
            if (pos != std::string::npos) {
                fileName = fileName.substr(0, pos);
            }
            return fileName.size() >= ext.size() && fileName.substr(fileName.size() - ext.size()) == ext;
        };

        if (fileNameEndsWith(GetPackageTypeExtension(PackageType::PACKAGE_TYPE_MAP))) {
            return PackageType::PACKAGE_TYPE_MAP;
        }
        else if (fileNameEndsWith(GetPackageTypeExtension(PackageType::PACKAGE_TYPE_ROUTING))) {
            return PackageType::PACKAGE_TYPE_ROUTING;
        }
        else if (fileNameEndsWith(GetPackageTypeExtension(PackageType::PACKAGE_TYPE_GEOCODING))) {
            return PackageType::PACKAGE_TYPE_GEOCODING;
        }
        Log::Warnf("PackageHandlerFactory::DetectPackageType: Unexpected package extension: %s", url.c_str());
        return PackageType::PACKAGE_TYPE_MAP; // sensible default
    }

}
