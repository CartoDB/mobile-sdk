#include "AddressInterpolator.h"

namespace carto { namespace geocoding {
	AddressInterpolator::AddressInterpolator(EncodingStream& houseNumberStream) {
		while (!houseNumberStream.eof()) {
			_houseNumbers.push_back(houseNumberStream.readNumber<std::uint64_t>());
		}
	}
    
	int AddressInterpolator::findAddress(std::uint64_t id) const {
		auto it = std::find(_houseNumbers.begin(), _houseNumbers.end(), id);
		return (it == _houseNumbers.end() ? -1 : static_cast<int>(it - _houseNumbers.begin()));
    }
    
    std::vector<std::pair<std::uint64_t, std::vector<Feature>>> AddressInterpolator::enumerateAddresses(FeatureReader& featureReader) const {
		std::vector<std::pair<std::uint64_t, std::vector<Feature>>> addresses;
		addresses.reserve(_houseNumbers.size());
		for (std::uint64_t id : _houseNumbers) {
			addresses.emplace_back(id, featureReader.readFeatureCollection());
		}
		return addresses;
    }
} }
