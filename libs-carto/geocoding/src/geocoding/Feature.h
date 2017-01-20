/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODING_FEATURE_H_
#define _CARTO_GEOCODING_FEATURE_H_

#include "Geometry.h"

#include <memory>
#include <cstdint>

namespace carto { namespace geocoding {
	class Feature final {
	public:
		explicit Feature(std::uint64_t id, const std::shared_ptr<Geometry>& geom) : _id(id), _geometry(geom) { }

		std::uint64_t getId() const { return _id; }
		const std::shared_ptr<Geometry>& getGeometry() const { return _geometry; }

	private:
		std::uint64_t _id;
		std::shared_ptr<Geometry> _geometry;
	};
} }

#endif
