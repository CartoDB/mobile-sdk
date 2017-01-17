#include "Address.h"
#include "FeatureReader.h"
#include "StringUtils.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <sqlite3pp.h>

namespace carto { namespace geocoding {
	bool Address::loadFromDB(sqlite3pp::database& db, long long encodedRowId, const std::string& language, const PointConverter& converter) {
		auto findField = [&db, &language](const std::string& type, long long id) -> std::string {
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

		unsigned int rowId = static_cast<unsigned int>(encodedRowId & 0xffffffffU);
		unsigned int elementIndex = static_cast<unsigned int>(encodedRowId >> 32);
		
		sqlite3pp::query query(db, "SELECT country_id, region_id, county_id, locality_id, neighbourhood_id, street_id, postcode_id, name_id, geometry, housenumbers FROM entities WHERE rowid=:rowId");
		query.bind(":rowId", rowId);

		for (auto qit = query.begin(); qit != query.end(); qit++) {
			country       = findField("country",       qit->get<long long>(0));
			region        = findField("region",        qit->get<long long>(1));
			county        = findField("county",        qit->get<long long>(2));
			locality      = findField("locality",      qit->get<long long>(3));
			neighbourhood = findField("neighbourhood", qit->get<long long>(4));
			street        = findField("street",        qit->get<long long>(5));
			postcode      = findField("postcode",      qit->get<long long>(6));
			name          = findField("name",          qit->get<long long>(7));

			id = 0;
			geometry.reset();
			if (auto encodedGeometry = qit->get<const void *>(8)) {
				EncodingStream stream(encodedGeometry, qit->column_bytes(8));
				FeatureReader reader(stream, converter);
				if (elementIndex) {
					for (unsigned int i = 1; i < elementIndex; i++) {
						reader.readFeature();
					}
					Feature feature = reader.readFeature();
					id = feature.getId();
					geometry = feature.getGeometry();
				}
				else {
					std::vector<long long> ids;
					std::vector<std::shared_ptr<Geometry>> geometries;
					while (!stream.eof()) {
						Feature feature = reader.readFeature();
						ids.push_back(feature.getId());
						geometries.push_back(feature.getGeometry());
					}
					if (geometries.size() != 1) {
						id = 0;
						geometry = std::make_shared<MultiGeometry>(geometries);
					}
					else {
						id = ids.front();
						geometry = geometries.front();
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
			return true;
		}
		return false;
	}

	bool Address::merge(const Address& address) {
		if (address.country == country && address.region == region && address.county == county && address.locality == locality && address.neighbourhood == neighbourhood && address.street == street && address.houseNumber.empty() == houseNumber.empty()) {
			if (address.id != id) {
				id = 0;
			}
			if (toLower(toUniString(address.name)) != toLower(toUniString(name))) {
				name.clear();
			}
			if (!houseNumber.empty()) {
				houseNumber += "," + address.houseNumber;
			}
			if (!geometry) {
				geometry = address.geometry;
			}
			else if (address.geometry) {
				std::vector<std::shared_ptr<Geometry>> geometries = { address.geometry };
				if (auto multiGeometry = std::dynamic_pointer_cast<MultiGeometry>(geometry)) {
					geometries.insert(geometries.end(), multiGeometry->getGeometries().begin(), multiGeometry->getGeometries().end());
				}
				else {
					geometries.push_back(geometry);
				}
				geometry = std::make_shared<MultiGeometry>(std::move(geometries));
			}
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
