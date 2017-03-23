/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODINGPROXY_H_
#define _CARTO_GEOCODINGPROXY_H_

#ifdef _CARTO_GEOCODING_SUPPORT

#include "geocoding/GeocodingService.h"
#include "geocoding/ReverseGeocodingService.h"

#include <memory>
#include <vector>

namespace carto {
    namespace geocoding {
        struct Address;
        class Geometry;
        class Feature;
        class Geocoder;
        class RevGeocoder;
    }

    class Geometry;
    class Feature;
    class FeatureCollection;
    
    class GeocodingProxy {
    public:
        static std::vector<std::shared_ptr<GeocodingResult> > CalculateAddresses(const std::shared_ptr<geocoding::Geocoder>& geocoder, const std::shared_ptr<GeocodingRequest>& request);

        static std::vector<std::shared_ptr<GeocodingResult> > CalculateAddresses(const std::shared_ptr<geocoding::RevGeocoder>& revGeocoder, const std::shared_ptr<ReverseGeocodingRequest>& request);

    private:
        GeocodingProxy();

        static std::shared_ptr<GeocodingResult> TranslateAddress(const std::shared_ptr<Projection>& proj, const geocoding::Address& addr, float rank);

        static std::shared_ptr<Feature> TranslateFeature(const std::shared_ptr<Projection>& proj, const geocoding::Feature& feature);

        static std::shared_ptr<Geometry> TranslateGeometry(const std::shared_ptr<Projection>& proj, const std::shared_ptr<geocoding::Geometry>& geom);
    };
    
}

#endif

#endif
