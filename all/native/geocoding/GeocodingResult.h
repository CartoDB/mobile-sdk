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

    /**
     * A geocoding result object describing address, features (geometry) and rank of the result.
     */
    class GeocodingResult {
    public:
        /**
         * Constructs a GeocodingResult from a projection, address, rank and feature collection.
         * @param projection The projection used for the geometry of the result.
         * @param address The address of the result.
         * @param rank The relative ranking of the result (in range 0..1).
         * @param featureCollection The geometry (features) associated with the result.
         */
        GeocodingResult(const std::shared_ptr<Projection>& projection, const Address& address, float rank, const std::shared_ptr<FeatureCollection>& featureCollection);
        virtual ~GeocodingResult();

        /**
         * Returns the address of the result.
         * @return The address of the result.
         */
        const Address& getAddress() const;

        /**
         * Returns the rank of the result.
         * The rank is a normalized number between 0 and 1, 1 meaning a perfect match.
         * @return The rank of the result.
         */
        float getRank() const;

        /**
         * Returns the feature collection of the result.
         * @return The feature collection of the result.
         */
        const std::shared_ptr<FeatureCollection>& getFeatureCollection() const;

        /**
         * Returns the projection of the geometry in the result.
         * @return The projection of the geometry in the result.
         */
        const std::shared_ptr<Projection>& getProjection() const;

        /**
         * Creates a string representation of this result object, useful for logging.
         * @return The string representation of this result object.
         */
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
