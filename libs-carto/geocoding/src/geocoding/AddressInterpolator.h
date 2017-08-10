/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODING_ADDRESSINTERPOLATOR_H_
#define _CARTO_GEOCODING_ADDRESSINTERPOLATOR_H_

#include "Feature.h"
#include "FeatureReader.h"

#include <cstdint>
#include <vector>

namespace carto { namespace geocoding {
    class AddressInterpolator final {
    public:
        explicit AddressInterpolator(EncodingStream& houseNumberStream);

        int findAddress(std::uint64_t id) const;
        std::vector<std::pair<std::uint64_t, std::vector<Feature>>> enumerateAddresses(FeatureReader& featureReader) const;

    private:
        std::vector<std::uint64_t> _houseNumbers;
    };
} }

#endif
