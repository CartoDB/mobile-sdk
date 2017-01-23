/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODINGRESULT_H_
#define _CARTO_GEOCODINGRESULT_H_

#ifdef _CARTO_GEOCODING_SUPPORT

#include "core/Address.h"

#include <memory>

namespace carto {
    class FeatureCollection;
    class Projection;

    class GeocodingResult {
    public:
        GeocodingResult(const std::shared_ptr<Projection>& projection, const Address& address, float rank, const std::shared_ptr<FeatureCollection>& featureCollection);
        virtual ~GeocodingResult();

        const Address& getAddress() const;
        float getRank() const;
        const std::shared_ptr<FeatureCollection>& getFeatureCollection() const;

        /**
         * Returns the projection of the geometry in the result.
         * @return The projection of the result.
         */
        const std::shared_ptr<Projection>& getProjection() const;

        std::string toString() const;
        
    private:
        Address _address;
        float _rank;
        std::shared_ptr<FeatureCollection> _featureCollection;
        std::shared_ptr<Projection> _projection;
    };
    
}

#endif

#endif
