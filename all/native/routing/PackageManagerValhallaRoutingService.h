/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_PACKAGEMANAGERVALHALLAROUTINGSERVICE_H_
#define _CARTO_PACKAGEMANAGERVALHALLAROUTINGSERVICE_H_

#if defined(_CARTO_ROUTING_SUPPORT) && defined(_CARTO_VALHALLA_ROUTING_SUPPORT) && defined(_CARTO_PACKAGEMANAGER_SUPPORT)

#include "core/Variant.h"
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
         * Returns the value of specified Valhalla configuration parameter.
         * @param param The name of the parameter. For example, "meili.auto.search_radius".
         * @return The value of the parameter. If the parameter does not exist, empty variant is returned.
         */
        Variant getConfigurationParameter(const std::string& param) const;
        /**
         * Sets the value of specified Valhalla configuration parameter.
         * @param param The name of the parameter. For example, "meili.auto.search_radius".
         * @param value The new value of the parameter.
         */
        void setConfigurationParameter(const std::string& param, const Variant& value);

        virtual std::string getProfile() const;
        virtual void setProfile(const std::string& profile);

        virtual std::shared_ptr<RouteMatchingResult> matchRoute(const std::shared_ptr<RouteMatchingRequest>& request) const;

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
        Variant _configuration;

        mutable std::vector<std::shared_ptr<sqlite3pp::database> > _cachedPackageDatabases;

        mutable std::mutex _mutex;

    private:
        std::shared_ptr<PackageManagerListener> _packageManagerListener;
    };
    
}

#endif

#endif
