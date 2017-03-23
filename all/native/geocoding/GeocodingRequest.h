/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODINGREQUEST_H_
#define _CARTO_GEOCODINGREQUEST_H_

#ifdef _CARTO_GEOCODING_SUPPORT

#include <memory>
#include <mutex>
#include <string>

#include "core/MapPos.h"

namespace carto {
    class Projection;

    class GeocodingRequest {
    public:
        GeocodingRequest(const std::shared_ptr<Projection>& projection, const std::string& query);
        virtual ~GeocodingRequest();

        const std::string& getQuery() const;
        const std::shared_ptr<Projection>& getProjection() const;

        MapPos getLocation() const;
        void setLocation(const MapPos& pos);

        float getLocationRadius() const;
        void setLocationRadius(float radius);
        
        std::string toString() const;

    private:
        static const int DEFAULT_LOCATION_RADIUS = 100000; // in meters

        std::string _query;
        std::shared_ptr<Projection> _projection;
        MapPos _location;
        float _locationRadius;

        mutable std::mutex _mutex;
    };
    
}

#endif

#endif
