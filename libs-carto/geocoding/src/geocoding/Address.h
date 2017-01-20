/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODING_ADDRESS_H_
#define _CARTO_GEOCODING_ADDRESS_H_

#include "Geometry.h"
#include "GeometryReader.h"

#include <memory>
#include <string>
#include <cstdint>

namespace sqlite3pp {
	class database;
}

namespace carto { namespace geocoding {
	struct Address final {
		std::uint64_t id = 0;
		std::string country;
		std::string region;
		std::string county;
		std::string locality;
		std::string neighbourhood;
		std::string street;
		std::string postcode;
		std::string houseNumber;
		std::string name;
		std::shared_ptr<Geometry> geometry;

		bool loadFromDB(sqlite3pp::database& db, std::uint64_t encodedRowId, const std::string& language, const PointConverter& converter);

		bool merge(const Address& address);

		std::string toString() const;
	};
} }

#endif
