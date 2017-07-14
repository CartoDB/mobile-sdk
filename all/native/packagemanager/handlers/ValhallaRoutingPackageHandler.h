/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VALHALLAROUTINGPACKAGEHANDLER_H_
#define _CARTO_VALHALLAROUTINGPACKAGEHANDLER_H_

#if defined(_CARTO_VALHALLA_ROUTING_SUPPORT) && defined(_CARTO_PACKAGEMANAGER_SUPPORT)

#include "core/MapTile.h"
#include "packagemanager/handlers/PackageHandler.h"

namespace sqlite3pp {
    class database;
}

namespace carto {
    
    class ValhallaRoutingPackageHandler : public PackageHandler {
    public:
        explicit ValhallaRoutingPackageHandler(const std::string& fileName);
        virtual ~ValhallaRoutingPackageHandler();

        std::shared_ptr<sqlite3pp::database> getDatabase();

        virtual void onImportPackage();
        virtual void onDeletePackage();

        virtual std::shared_ptr<PackageTileMask> calculateTileMask() const;

    private:
        std::shared_ptr<sqlite3pp::database> _packageDb;
    };
    
}

#endif

#endif
