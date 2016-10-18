/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_CARTOSQLSERVICE_H_
#define _CARTO_CARTOSQLSERVICE_H_

#include "core/Variant.h"

#include <memory>
#include <mutex>
#include <string>

namespace carto {
    class FeatureCollection;
    class Projection;

    /**
     * A high-level interface for Carto SQL Service.
     * The service can be used to query data from Carto databases
     * using explicit SQL queries.
     */
    class CartoSQLService {
    public:
        /**
         * Constructs a new CartoSQLService service instance with default settings.
         */
        CartoSQLService();
        virtual ~CartoSQLService();
        
        /**
         * Returns the user account name used when accessing the online service.
         * @return The user account name used when accessing the online service.
         */
        std::string getUsername() const;
        /**
         * Sets the user account name used when accessing the online service.
         * @param username The user account name used when accessing the online service.
         */
        void setUsername(const std::string& username);

        /**
         * Returns the API endpoint template of the online service.
         * @return The API endpoint template of the online service.
         */
        std::string getAPITemplate() const;
        /**
         * Sets the API endpoint template of the online service.
         * By default cartodb.com endpoint is used. This needs to be redefined only for on-premise services.
         * @param apiTemplate The API endpoint template of the online service. For example, "http://{user}.cartodb.com"
         */
        void setAPITemplate(const std::string& apiTemplate);

        /**
         * Connects to the online service and performs the specified query,
         * The resulting JSON is deserialized into a Variant that is returned.
         * @param sql The SQL query to use.
         * @return The query result. If query fails, null variant is returned.
         * @throws std::runtime_error If IO error occured during the operation.
         */
        Variant queryData(const std::string& sql) const;

        /**
         * Connects to the online service and performs the specified query.
         * The resulting JSON is deserialized into a FeatureCollection that is returned.
         * @param sql The SQL query to use.
         * @param proj The projection to use for transforming feature coordinates. Can be null for WGS84 coordinates.
         * @return The query result as feature collection. If query fails, null feature collection is returned.
         * @throws std::runtime_error If IO error occured during the operation.
         */
        std::shared_ptr<FeatureCollection> queryFeatures(const std::string& sql, const std::shared_ptr<Projection>& proj) const;

    private:
        std::string executeQuery(const std::map<std::string, std::string>& urlParams) const;

        static const std::string DEFAULT_API_TEMPLATE;

        std::string _username;
        std::string _apiTemplate;

        mutable std::recursive_mutex _mutex;
    };

}

#endif
