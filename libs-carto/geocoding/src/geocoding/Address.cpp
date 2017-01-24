#include "Address.h"
#include "FeatureReader.h"
#include "StringUtils.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <sqlite3pp.h>

namespace carto { namespace geocoding {
	bool Address::loadFromDB(sqlite3pp::database& db, std::uint64_t encodedId, const std::string& language, const PointConverter& converter) {
		auto findField = [&db, &language](const std::string& type, std::uint64_t id) -> std::string {
			if (id == 0) {
				return std::string();
			}
			
			std::string sql = "SELECT name, lang FROM " + type + "names WHERE id=:id";
			sqlite3pp::query query(db, sql.c_str());
			query.bind(":id", id);
			
			std::string defaultValue;
			for (auto qit = query.begin(); qit != query.end(); qit++) {
				auto value = qit->get<const char*>(0);
				auto lang = qit->get<const char*>(1);
				if (!lang) {
					defaultValue = value;
				}
				else if (lang == language) {
					return value;
				}
			}
			return defaultValue;
		};

		unsigned int entityId = static_cast<unsigned int>(encodedId & 0xffffffffU);
		unsigned int elementIndex = static_cast<unsigned int>(encodedId >> 32);
		
		sqlite3pp::query query(db, "SELECT country_id, region_id, county_id, locality_id, neighbourhood_id, street_id, postcode_id, name_id, features, housenumbers FROM entities WHERE id=:id");
		query.bind(":id", entityId);

		for (auto qit = query.begin(); qit != query.end(); qit++) {
			country       = findField("country",       qit->get<std::uint64_t>(0));
			region        = findField("region",        qit->get<std::uint64_t>(1));
			county        = findField("county",        qit->get<std::uint64_t>(2));
			locality      = findField("locality",      qit->get<std::uint64_t>(3));
			neighbourhood = findField("neighbourhood", qit->get<std::uint64_t>(4));
			street        = findField("street",        qit->get<std::uint64_t>(5));
			postcode      = findField("postcode",      qit->get<std::uint64_t>(6));
			name          = findField("name",          qit->get<std::uint64_t>(7));

			features.clear();
			if (auto encodedFeatures = qit->get<const void *>(8)) {
				EncodingStream stream(encodedFeatures, qit->column_bytes(8));
				FeatureReader reader(stream, converter);
				if (elementIndex) {
					for (unsigned int i = 1; i < elementIndex; i++) {
						reader.readFeatureCollection();
					}
					std::vector<Feature> featureCollection = reader.readFeatureCollection();
					features.insert(features.end(), featureCollection.begin(), featureCollection.end());
				}
				else {
					while (!stream.eof()) {
						std::vector<Feature> featureCollection = reader.readFeatureCollection();
						features.insert(features.end(), featureCollection.begin(), featureCollection.end());
					}
				}
			}

			houseNumber.clear();
			if (auto houseNumbers = qit->get<const char*>(9)) {
				std::vector<std::string> houseNumberVector;
				boost::split(houseNumberVector, houseNumbers, boost::is_any_of("|"), boost::token_compress_off);
				if (elementIndex) {
					houseNumber = houseNumberVector.at(elementIndex - 1);
				}
				else {
					houseNumber = (houseNumberVector.size() == 1 ? houseNumberVector.front() : std::string());
				}
			}

				
			categories.clear();
			sqlite3pp::query query2(db, "SELECT c.category FROM entitycategories ec, categories c WHERE ec.entity_id=:id AND ec.category_id=c.id");
			query2.bind(":id", entityId);
			for (auto qit2 = query2.begin(); qit2 != query2.end(); qit2++) {
				categories.insert(qit2->get<const char*>(0));
			}

			return true;
		}
		return false;
	}

	bool Address::merge(const Address& address) {
		if (address.country == country && address.region == region && address.county == county && address.locality == locality && address.neighbourhood == neighbourhood && address.street == street && address.name == name && address.houseNumber.empty() == houseNumber.empty()) {
			if (!houseNumber.empty()) {
				houseNumber += "," + address.houseNumber;
			}

			features.insert(features.end(), address.features.begin(), address.features.end());
			categories.insert(address.categories.begin(), address.categories.end());

			return true;
		}
		return false;
	}

	std::string Address::toString() const {
		std::string str;
		if (!name.empty()) {
			str += name;
		}
		if (!houseNumber.empty()) {
			str += (str.empty() ? "" : ", ") + houseNumber;
		}
		if (!street.empty()) {
			str += (str.empty() ? "" : (houseNumber.empty() ? ", " : " ")) + street;
		}
		if (!neighbourhood.empty()) {
			str += (str.empty() ? "" : ", ") + neighbourhood;
		}
		if (!locality.empty()) {
			str += (str.empty() ? "" : ", ") + locality;
		}
		if (!county.empty()) {
			str += (str.empty() ? "" : ", ") + county;
		}
		if (!region.empty()) {
			str += (str.empty() ? "" : ", ") + region;
		}
		if (!country.empty()) {
			str += (str.empty() ? "" : ", ") + country;
		}
		if (!postcode.empty()) {
			str += (str.empty() ? "" : ", ") + postcode;
		}
		return str;
	}
} }
