/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_REVERSEGEOCODINGREQUEST_H_
#define _CARTO_REVERSEGEOCODINGREQUEST_H_

#ifdef _CARTO_GEOCODING_SUPPORT

#include "core/MapPos.h"

#include <memory>

namespace carto {
    class Projection;

    class ReverseGeocodingRequest {
    public:
        ReverseGeocodingRequest(const std::shared_ptr<Projection>& projection, const MapPos& location);
        virtual ~ReverseGeocodingRequest();

        const MapPos& getLocation() const;
        const std::shared_ptr<Projection>& getProjection() const;
        
        std::string toString() const;

    private:
        MapPos _location;
        std::shared_ptr<Projection> _projection;
    };
    
}

#endif

#endif
