/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODINGRESULT_H_
#define _CARTO_GEOCODINGRESULT_H_

#ifdef _CARTO_GEOCODING_SUPPORT

#include "geocoding/GeocodingAddress.h"

#include <memory>

namespace carto {
    class Geometry;
    class Projection;

    class GeocodingResult {
    public:
        GeocodingResult(const std::shared_ptr<Projection>& projection, long long id, const GeocodingAddress& address, float rank, const std::shared_ptr<Geometry>& geometry);
        virtual ~GeocodingResult();

        long long getId() const;
        const GeocodingAddress& getAddress() const;
        float getRank() const;
        const std::shared_ptr<Geometry>& getGeometry() const;

        /**
         * Returns the projection of the geometry in the result.
         * @return The projection of the result.
         */
        const std::shared_ptr<Projection>& getProjection() const;

        std::string toString() const;
        
    private:
        long long _id;
        GeocodingAddress _address;
        float _rank;
        std::shared_ptr<Geometry> _geometry;
        std::shared_ptr<Projection> _projection;
    };
    
}

#endif

#endif
