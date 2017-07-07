/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_PACKAGEMANAGERGEOCODINGSERVICE_H_
#define _CARTO_PACKAGEMANAGERGEOCODINGSERVICE_H_

#if defined(_CARTO_GEOCODING_SUPPORT) && defined(_CARTO_PACKAGEMANAGER_SUPPORT)

#include "geocoding/GeocodingService.h"
#include "packagemanager/PackageManager.h"

#include <memory>

namespace sqlite3pp {
    class database;
}

namespace carto {
    namespace geocoding {
        class Geocoder;
    }

    /**
     * A geocoding service that uses geocoding packages from package manager.
     */
    class PackageManagerGeocodingService : public GeocodingService {
    public:
        /**
         * Constructs a new instance of the PackageManagerGeocodingService given package manager instance.
         * @param packageManager The package manager instance to use.
         */
        explicit PackageManagerGeocodingService(const std::shared_ptr<PackageManager>& packageManager);
        virtual ~PackageManagerGeocodingService();

        /**
         * Returns the autocomplete flag of the service.
         * @return The autocomplete flag of the service.
         */
        bool isAutocomplete() const;
        /**
         * Sets the autocomplete flag of the service.
         * By default this flag is off.
         * @param autocomplete The new value for autocomplete flag.
         */
        void setAutocomplete(bool autocomplete);

        virtual std::vector<std::shared_ptr<GeocodingResult> > calculateAddresses(const std::shared_ptr<GeocodingRequest>& request) const;

    protected:
        class PackageManagerListener : public PackageManager::OnChangeListener {
        public:
            PackageManagerListener(PackageManagerGeocodingService& service);
        		
            virtual void onPackagesChanged();
        		
        private:
            PackageManagerGeocodingService& _service;
        };

        const std::shared_ptr<PackageManager> _packageManager;
        bool _autocomplete;

        mutable std::map<std::shared_ptr<PackageInfo>, std::shared_ptr<sqlite3pp::database> > _cachedPackageDatabaseMap;
        mutable std::shared_ptr<geocoding::Geocoder> _cachedGeocoder;

        mutable std::mutex _mutex;

    private:
        std::shared_ptr<PackageManagerListener> _packageManagerListener;
    };
    
}

#endif

#endif
