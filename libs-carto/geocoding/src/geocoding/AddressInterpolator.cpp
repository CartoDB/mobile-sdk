#include "AddressInterpolator.h"
#include "StringUtils.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace carto { namespace geocoding {
    AddressInterpolator::AddressInterpolator(const std::string& houseNumberPatterns) {
        boost::split(_houseNumberPatterns, houseNumberPatterns, boost::is_any_of("|"), boost::token_compress_off);
    }

    int AddressInterpolator::findAddress(const std::string& houseNumber) const {
        int index = 0;
        for (const std::string& houseNumberPattern : _houseNumberPatterns) {
            std::vector<std::string> houseNumbers = enumerateHouseNumbers(houseNumberPattern);
            auto it = std::find_if(houseNumbers.begin(), houseNumbers.end(), [&houseNumber](const std::string& houseNumber2) {
                return toLower(toUniString(houseNumber)) == toLower(toUniString(houseNumber2));
            });
            index += static_cast<int>(it - houseNumbers.begin());
            if (it != houseNumbers.end()) {
                return index;
            }
        }
        return -1;
    }
    
    std::vector<std::pair<std::string, std::vector<Feature>>> AddressInterpolator::enumerateAddresses(FeatureReader& featureReader) const {
        std::vector<std::pair<std::string, std::vector<Feature>>> addresses;
        addresses.reserve(_houseNumberPatterns.size());
        for (const std::string& houseNumberPattern : _houseNumberPatterns) {
            std::vector<std::string> houseNumbers = enumerateHouseNumbers(houseNumberPattern);
            std::vector<Feature> features = featureReader.readFeatureCollection();

            if (features.size() == 1 && houseNumbers.size() > 1) {
                const Feature& feature = features.front();
                if (auto lineGeometry = std::dynamic_pointer_cast<LineGeometry>(feature.getGeometry())) {
                    double len = 0;
                    for (std::size_t i = 1; i < lineGeometry->getPoints().size(); i++) {
                        len += cglib::length(lineGeometry->getPoints()[i] - lineGeometry->getPoints()[i - 1]);
                    }
                    for (std::size_t j = 0; j < houseNumbers.size(); j++) {
                        double pos = static_cast<double>(j) / static_cast<double>(houseNumbers.size() - 1) * len;
                        double t = 0;
                        for (std::size_t i = 1; i < lineGeometry->getPoints().size(); i++) {
                            cglib::vec2<double> delta = lineGeometry->getPoints()[i] - lineGeometry->getPoints()[i - 1];
                            double dt = cglib::length(delta);
                            if (t + dt >= pos) {
                                LineGeometry::Point p = lineGeometry->getPoints()[i - 1] + delta * ((pos - t) / dt);
                                addresses.emplace_back(houseNumbers[j], std::vector<Feature> { Feature(feature.getId(), std::make_shared<PointGeometry>(p), feature.getProperties()) });
                                break;
                            }
                            t += dt;
                        }
                    }
                }
            }

            addresses.emplace_back(houseNumbers.front(), std::move(features));
        }
        return addresses;
    }

    std::vector<std::string> AddressInterpolator::enumerateHouseNumbers(const std::string& houseNumberPattern) {
        return std::vector<std::string> { houseNumberPattern };
    }
} }
