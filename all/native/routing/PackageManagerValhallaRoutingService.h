/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_PACKAGEMANAGERVALHALLAROUTINGSERVICE_H_
#define _CARTO_PACKAGEMANAGERVALHALLAROUTINGSERVICE_H_

#if defined(_CARTO_VALHALLA_ROUTING_SUPPORT) && defined(_CARTO_PACKAGEMANAGER_SUPPORT)

#include "packagemanager/PackageManager.h"
#include "routing/RoutingService.h"

#include <memory>
#include <string>
#include <map>
#include <mutex>

namespace sqlite3pp {
    class database;
}

namespace carto {
    class RouteMatchingRequest;
    class RouteMatchingResult;

    /**
     * A routing service that uses routing packages from package manager.
     */
    class PackageManagerValhallaRoutingService : public RoutingService {
    public:
        /**
         * Constructs a new instance of the PackageManagerValhallaRoutingService given package manager instance.
         * @param packageManager The package manager instance to use.
         */
        explicit PackageManagerValhallaRoutingService(const std::shared_ptr<PackageManager>& packageManager);
        virtual ~PackageManagerValhallaRoutingService();

        /**
         * Returns the current routing profile.
         * @return The current routing profile. Can be either "car", "auto", "auto_shorter", "bus", "bicycle", "pedestrian" or "truck". The default is "pedestrian".
         */
        std::string getProfile() const;
        /**
         * Sets the current routing profile.
         * @param profile The new profile. Can be either "car", "auto", "auto_shorter", "bus", "bicycle", "pedestrian" or "truck".
         */
        void setProfile(const std::string& profile);

        /**
         * Matches specified points to the points on road network.
         * @param request The matching request.
         * @return The matching result or null if route matching failed.
         * @throws std::runtime_error If IO error occured during the route matching.
         */
        std::shared_ptr<RouteMatchingResult> matchRoute(const std::shared_ptr<RouteMatchingRequest>& request) const;

        virtual std::shared_ptr<RoutingResult> calculateRoute(const std::shared_ptr<RoutingRequest>& request) const;

    protected:
        class PackageManagerListener : public PackageManager::OnChangeListener {
        public:
            explicit PackageManagerListener(PackageManagerValhallaRoutingService& service);
                
            virtual void onPackagesChanged();
            virtual void onStylesChanged();

        private:
            PackageManagerValhallaRoutingService& _service;
        };

        const std::shared_ptr<PackageManager> _packageManager;
        std::string _profile;

        mutable std::vector<std::shared_ptr<sqlite3pp::database> > _cachedPackageDatabases;

        mutable std::mutex _mutex;

    private:
        std::shared_ptr<PackageManagerListener> _packageManagerListener;
    };
    
}

#endif

#endif
