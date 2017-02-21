/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODING_ADDRESSINTERPOLATOR_H_
#define _CARTO_GEOCODING_ADDRESSINTERPOLATOR_H_

#include "Feature.h"
#include "FeatureReader.h"

#include <string>
#include <vector>

namespace carto { namespace geocoding {
    class AddressInterpolator final {
    public:
        explicit AddressInterpolator(const std::string& houseNumberPatterns);

        int findAddress(const std::string& houseNumber) const;
        std::vector<std::pair<std::string, std::vector<Feature>>> enumerateAddresses(FeatureReader& featureReader) const;

    private:
        static std::vector<std::string> enumerateHouseNumbers(const std::string& houseNumberPattern);

        std::vector<std::string> _houseNumberPatterns;
    };
} }

#endif
