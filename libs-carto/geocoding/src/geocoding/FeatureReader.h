/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODING_FEATUREREADER_H_
#define _CARTO_GEOCODING_FEATUREREADER_H_

#include "Feature.h"
#include "GeometryReader.h"
#include "EncodingStream.h"

#include <memory>
#include <string>

namespace carto { namespace geocoding {
	class FeatureReader final {
	public:
		explicit FeatureReader(EncodingStream& stream, const PointConverter& converter) : _stream(stream), _geometryReader(stream, converter) { }

		Feature readFeature() {
			std::uint64_t id = _stream.decodeDeltaNumber<std::uint64_t>();
			std::shared_ptr<Geometry> geometry = _geometryReader.readGeometry();
			std::string extra = _stream.decodeString();
			return Feature(id, geometry);
		}

	private:
		EncodingStream& _stream;
		GeometryReader _geometryReader;
	};
} }

#endif
