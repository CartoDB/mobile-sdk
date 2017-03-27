/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_PACKAGEHANDLERFACTORY_H_
#define _CARTO_PACKAGEHANDLERFACTORY_H_

#if defined(_CARTO_PACKAGEMANAGER_SUPPORT)

#include "packagemanager/PackageInfo.h"

#include <memory>
#include <string>

namespace carto {
    class PackageHandler;
    
    class PackageHandlerFactory {
    public:
        PackageHandlerFactory(const std::string& serverEncKey, const std::string& localEncKey);
        virtual ~PackageHandlerFactory();

        std::shared_ptr<PackageHandler> createPackageHandler(PackageType::PackageType packageType, const std::string& filePath) const;
    
        static std::string GetPackageTypeExtension(PackageType::PackageType packageType);

        static PackageType::PackageType DetectPackageType(const std::string& url);

    private:
        const std::string _serverEncKey;
        const std::string _localEncKey;
    };
    
}

#endif

#endif
