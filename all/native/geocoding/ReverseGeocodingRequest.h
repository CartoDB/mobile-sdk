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
#include <mutex>

namespace carto {
    class Projection;

    /**
     * A reverse geocoding request describing search location.
     */
    class ReverseGeocodingRequest {
    public:
        /**
         * Constructs a new ReverseGeocodingRequest object from a projection and location.
         * @param projection The projection to use for the result and location data.
         * @param location The location of the query.
         */
        ReverseGeocodingRequest(const std::shared_ptr<Projection>& projection, const MapPos& location);
        virtual ~ReverseGeocodingRequest();

        /**
         * Returns the location of the query.
         * @return The location of the query.
         */
        const MapPos& getLocation() const;

        /**
         * Returns the search radius (in meters).
         * @return The search radius in meters.
         */
        float getSearchRadius() const;
        /**
         * Sets the search radius (in meters).
         * The default search radius is 100 meters.
         * @param radius The new search radius in meters.
         */
        void setSearchRadius(float radius);

        /**
         * Returns the projection of the query.
         * @return The projection of the query.
         */
        const std::shared_ptr<Projection>& getProjection() const;
        
        /**
         * Creates a string representation of this request object, useful for logging.
         * @return The string representation of this request object.
         */
        std::string toString() const;

    private:
        static const float DEFAULT_SEARCH_RADIUS;

        MapPos _location;
        float _searchRadius;
        std::shared_ptr<Projection> _projection;

        mutable std::mutex _mutex;
    };
    
}

#endif

#endif
