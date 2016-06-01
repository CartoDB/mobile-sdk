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

#include <picojson/picojson.h>

namespace carto {

    class CartoSQLService {
    public:
        CartoSQLService();
        virtual ~CartoSQLService();
        
        std::string getUsername() const;
        void setUsername(const std::string& username);

        std::string getAPIKey() const;
        void setAPIKey(const std::string& apiKey);

        std::string getAPITemplate() const;
        void setAPITemplate(const std::string& apiTemplate);

        Variant queryData(const std::string& sql) const;
        // TODO: implement
        // FeatureCollection queryFeatures(const std::string& sql) const;

    private:
        static const std::string DEFAULT_API_TEMPLATE;

        std::string _username;
        std::string _apiKey;
        std::string _apiTemplate;

        mutable std::recursive_mutex _mutex;
    };

}

#endif
