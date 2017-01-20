/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODING_REVGEOCODER_H_
#define _CARTO_GEOCODING_REVGEOCODER_H_

#include "Address.h"
#include "Geometry.h"
#include "QuadIndex.h"

#include <vector>
#include <memory>
#include <mutex>

#include <boost/optional.hpp>

#include <stdext/lru_cache.h>

#include <cglib/bbox.h>

namespace sqlite3pp {
	class database;
}

namespace carto { namespace geocoding {
	class RevGeocoder final {
	public:
		explicit RevGeocoder(sqlite3pp::database& db) : _queryCache(QUERY_CACHE_SIZE), _db(db) { _bounds = findBounds(); _origin = findOrigin(); }

		float getRadius() const;
		void setRadius(float radius);

		std::string getLanguage() const;
		void setLanguage(const std::string& language);

		boost::optional<Address> findAddress(double lng, double lat) const;

	private:
		cglib::vec2<double> findOrigin() const;
		boost::optional<cglib::bbox2<double>> findBounds() const;

		std::vector<QuadIndex::GeometryInfo> findGeometryInfo(const std::vector<std::uint64_t>& quadIndices, const PointConverter& converter) const;

		static constexpr std::size_t QUERY_CACHE_SIZE = 64;
		
		float _radius = 100.0f;
		std::string _language;

		mutable cache::lru_cache<std::string, std::vector<QuadIndex::GeometryInfo>> _queryCache;

		cglib::vec2<double> _origin;
		boost::optional<cglib::bbox2<double>> _bounds;
		sqlite3pp::database& _db;
		mutable std::recursive_mutex _mutex;
	};
} }

#endif
