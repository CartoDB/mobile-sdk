/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_PACKAGEMANAGERROUTINGSERVICE_H_
#define _CARTO_PACKAGEMANAGERROUTINGSERVICE_H_

#if defined(_CARTO_ROUTING_SUPPORT) && defined(_CARTO_PACKAGEMANAGER_SUPPORT)

#include "packagemanager/PackageManager.h"
#include "routing/RoutingService.h"

#include <memory>
#include <string>
#include <map>
#include <mutex>

namespace carto {
    namespace routing {
        class RouteFinder;
    }

    /**
     * A routing service that uses routing packages from package manager.
     */
    class PackageManagerRoutingService : public RoutingService {
    public:
        /**
         * Constructs a new instance of the PackageManagerRoutingService given package manager instance.
         * @param packageManager The package manager instance to use.
         */
        explicit PackageManagerRoutingService(const std::shared_ptr<PackageManager>& packageManager);
        virtual ~PackageManagerRoutingService();

        virtual std::shared_ptr<RoutingResult> calculateRoute(const std::shared_ptr<RoutingRequest>& request) const;

    protected:
        class PackageManagerListener : public PackageManager::OnChangeListener {
        public:
            PackageManagerListener(PackageManagerRoutingService& service);
        		
            virtual void onPackagesChanged();
        		
        private:
            PackageManagerRoutingService& _service;
        };

        std::shared_ptr<PackageManager> _packageManager;

        mutable std::map<std::string, std::shared_ptr<std::ifstream> > _cachedPackageFileMap;
        mutable std::shared_ptr<routing::RouteFinder> _cachedRouteFinder;
        mutable std::mutex _mutex;

    private:
        std::shared_ptr<PackageManagerListener> _packageManagerListener;
    };
    
}

#endif

#endif
