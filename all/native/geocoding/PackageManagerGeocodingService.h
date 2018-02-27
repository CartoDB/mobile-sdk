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

        /**
         * Returns the language of the expected results.
         * @return The language of the expected results. As ISO 639-1 code or empty string.
         */
        std::string getLanguage() const;
        /**
         * Sets the language of the expected results.
         * @param lang The language to use as ISO 639-1 code. Empty string can be used for default language.
         */
        void setLanguage(const std::string& lang);

        virtual std::vector<std::shared_ptr<GeocodingResult> > calculateAddresses(const std::shared_ptr<GeocodingRequest>& request) const;

    protected:
        class PackageManagerListener : public PackageManager::OnChangeListener {
        public:
            explicit PackageManagerListener(PackageManagerGeocodingService& service);

            virtual void onPackagesChanged();
            virtual void onStylesChanged();

        private:
            PackageManagerGeocodingService& _service;
        };

        const std::shared_ptr<PackageManager> _packageManager;
        bool _autocomplete;
        std::string _language;

        mutable std::map<std::shared_ptr<PackageInfo>, std::shared_ptr<sqlite3pp::database> > _cachedPackageDatabaseMap;
        mutable std::shared_ptr<geocoding::Geocoder> _cachedGeocoder;

        mutable std::mutex _mutex;

    private:
        std::shared_ptr<PackageManagerListener> _packageManagerListener;
    };
    
}

#endif

#endif
