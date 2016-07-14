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
         * Returns the API key used when accessing the online service.
         * @return The API key used when accessing the online service.
         */
        std::string getAPIKey() const;
        /**
         * Sets the optional API key used when accessing the online service.
         * Usually this is not needed and can be left blank.
         * @param apiKey The optional API key used when accessing the online service.
         */
        void setAPIKey(const std::string& apiKey);

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
         * Connects to the online service and performs the specified query
         * The resulting JSON is deserialized into a Variant that is returned.
         * @param sql The SQL query to use.
         * @return The query result. If query fails, null variant is returned.
         * @throws std::runtime_error If IO error occured during the operation.
         */
        Variant queryData(const std::string& sql) const;

    private:
        static const std::string DEFAULT_API_TEMPLATE;

        std::string _username;
        std::string _apiKey;
        std::string _apiTemplate;

        mutable std::recursive_mutex _mutex;
    };

}

#endif
