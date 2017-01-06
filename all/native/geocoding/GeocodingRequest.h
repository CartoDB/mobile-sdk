/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODINGREQUEST_H_
#define _CARTO_GEOCODINGREQUEST_H_

#ifdef _CARTO_GEOCODING_SUPPORT

#include <memory>
#include <string>

namespace carto {
    class Projection;

    class GeocodingRequest {
    public:
        GeocodingRequest(const std::shared_ptr<Projection>& projection, const std::string& query);
        virtual ~GeocodingRequest();

        const std::string& getQuery() const;
        const std::shared_ptr<Projection>& getProjection() const;
        
        std::string toString() const;

    private:
        std::string _query;
        std::shared_ptr<Projection> _projection;
    };
    
}

#endif

#endif
