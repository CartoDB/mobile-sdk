/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VALHALLAOFFLINEROUTINGSERVICE_H_
#define _CARTO_VALHALLAOFFLINEROUTINGSERVICE_H_

#if defined(_CARTO_ROUTING_SUPPORT) && defined(_CARTO_VALHALLA_ROUTING_SUPPORT) && defined(_CARTO_OFFLINE_SUPPORT)

#include "core/Variant.h"
#include "routing/RoutingService.h"

#include <memory>
#include <mutex>
#include <string>

namespace sqlite3pp {
    class database;
}

namespace carto {
    class TileDataSource;
    class ElevationDecoder;

    /**
     * An offline routing service that uses Valhalla routing tiles.
     */
    class ValhallaOfflineRoutingService : public RoutingService {
    public:
        /**
         * Constructs a new ValhallaOfflineRoutingService instance given database file.
         * @param path The full path to the database file.
         * @throws std::runtime_error If the database file could not be opened or read.
         */
        explicit ValhallaOfflineRoutingService(const std::string& path);
        virtual ~ValhallaOfflineRoutingService();

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

        void connectElevationDataSource(const std::shared_ptr<TileDataSource>& dataSource, const std::shared_ptr<ElevationDecoder>& elevationDecoder);

    private:
        std::shared_ptr<sqlite3pp::database> _database;
        std::string _profile;
        Variant _configuration;
        mutable std::mutex _mutex;
        std::shared_ptr<TileDataSource> _elevationDataSource;
        std::shared_ptr<ElevationDecoder> _elevationDecoder;
    };
    
}

#endif

#endif
