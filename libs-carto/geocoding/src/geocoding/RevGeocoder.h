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
	class RevGeocoder {
	public:
		explicit RevGeocoder(sqlite3pp::database& db) : _queryCache(QUERY_CACHE_SIZE), _db(db) { _bounds = findBounds(); }

		void setRadius(float radius) { _radius = radius; }

		boost::optional<Address> findAddress(double lng, double lat) const;

	private:
		boost::optional<cglib::bbox2<double>> findBounds() const;

		std::vector<QuadIndex::Feature> findFeatures(const std::vector<long long>& quadIndices) const;

		static constexpr std::size_t QUERY_CACHE_SIZE = 64;
		
		float _radius = 100.0f;
		mutable cache::lru_cache<std::string, std::vector<QuadIndex::Feature>> _queryCache;
		boost::optional<cglib::bbox2<double>> _bounds;
		sqlite3pp::database& _db;
		mutable std::recursive_mutex _mutex;
	};
} }

#endif
