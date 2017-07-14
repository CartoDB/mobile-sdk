/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_PACKAGEHANDLER_H_
#define _CARTO_PACKAGEHANDLER_H_

#if defined(_CARTO_PACKAGEMANAGER_SUPPORT)

#include <memory>
#include <mutex>
#include <string>

namespace carto {
    class PackageTileMask;
    
    class PackageHandler {
    public:
        virtual ~PackageHandler() { }

        virtual void onImportPackage() = 0;
        virtual void onDeletePackage() = 0;

        virtual std::shared_ptr<PackageTileMask> calculateTileMask() const = 0;
    
    protected:
        PackageHandler(const std::string& fileName) : _fileName(fileName), _mutex() { }

        const std::string _fileName;

        mutable std::recursive_mutex _mutex;
    };
    
}

#endif

#endif
