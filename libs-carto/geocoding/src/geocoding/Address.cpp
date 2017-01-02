#include "Address.h"
#include "GeometryDecoder.h"

#include <boost/algorithm/string/split.hpp>

#include <sqlite3pp.h>

namespace carto { namespace geocoding {
	bool Address::loadFromDB(sqlite3pp::database& db, long long encodedRowId) {
		auto findField = [&db](const std::string& type, long long id) -> std::string {
			if (id == 0) {
				return std::string();
			}
			
			std::string sql = "SELECT name FROM " + type + "names WHERE id=:id";
			sqlite3pp::query query(db, sql.c_str());
			query.bind(":id", id);
			
			for (auto qit = query.begin(); qit != query.end(); qit++) {
				return qit->get<const char*>(0);
			}
			return std::string();
		};

		sqlite3pp::query query(db, "SELECT id, country_id, region_id, county_id, locality_id, neighbourhood_id, street_id, postcode_id, name, geometry, housenums FROM entities WHERE rowid=:rowId");
		query.bind(":rowId", encodedRowId >> 32);

		for (auto qit = query.begin(); qit != query.end(); qit++) {
			id = qit->get<long long>(0);
			country = findField("country", qit->get<long long>(1));
			region = findField("region", qit->get<long long>(2));
			county = findField("county", qit->get<long long>(3));
			locality = findField("locality", qit->get<long long>(4));
			neighbourhood = findField("neighbourhood", qit->get<long long>(5));
			street = findField("street", qit->get<long long>(6));
			postcode = findField("postcode", qit->get<long long>(7));
			name = (qit->get<const char*>(8) ? qit->get<const char*>(8) : "");
			if (const char* encodedGeometry = qit->get<const char*>(9)) {
				geometry = decodeGeometry(encodedGeometry);
			}
			else {
				geometry.reset();
			}
			if (const char* houseNums = qit->get<const char*>(10)) {
				std::size_t index = static_cast<std::size_t>((encodedRowId & 0xffffffffU) - 1);
				std::vector<std::string> houseNumsVector;
				boost::split(houseNumsVector, houseNums, boost::is_any_of("|"), boost::token_compress_off);
				houseNumber = houseNumsVector.at(index);
				if (auto multiGeometry = std::dynamic_pointer_cast<MultiGeometry>(geometry)) {
					geometry = multiGeometry->getGeometries().at(index);
				}
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
