/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODING_GEOMETRYREADER_H_
#define _CARTO_GEOCODING_GEOMETRYREADER_H_

#include "Geometry.h"
#include "EncodingStream.h"

#include <memory>
#include <functional>

namespace carto { namespace geocoding {
	using PointConverter = std::function<cglib::vec2<double>(const cglib::vec2<double>&)>;

	class GeometryReader final {
	public:
		explicit GeometryReader(EncodingStream& stream, const PointConverter& converter) : _stream(stream), _pointConverter(converter) { }

		std::shared_ptr<Geometry> readGeometry() {
			int type = decodeNumber<int>();
			if (type == 0) {
				return std::shared_ptr<Geometry>();
			}
			else if (type == 1) {
				return std::make_shared<PointGeometry>(decodeCoord());
			}
			else if (type == 2) {
				std::vector<cglib::vec2<double>> coords = decodeCoords();
				std::vector<std::shared_ptr<Geometry>> geoms;
				geoms.reserve(coords.size());
				for (const cglib::vec2<double>& coord : coords) {
					geoms.push_back(std::make_shared<PointGeometry>(coord));
				}
				return std::make_shared<MultiGeometry>(std::move(geoms));
			}
			else if (type == 3) {
				std::vector<cglib::vec2<double>> coords = decodeCoords();
				return std::make_shared<LineGeometry>(std::move(coords));
			}
			else if (type == 4) {
				std::size_t n = decodeNumber<std::size_t>();
				std::vector<std::shared_ptr<Geometry>> geoms;
				geoms.reserve(n);
				for (std::size_t i = 0; i < n; i++) {
					std::vector<cglib::vec2<double>> coords = decodeCoords();
					geoms.push_back(std::make_shared<LineGeometry>(std::move(coords)));
				}
				return std::make_shared<MultiGeometry>(std::move(geoms));
			}
			else if (type == 5) {
				std::vector<std::vector<cglib::vec2<double>>> rings = decodeRings();
				if (rings.empty()) {
					return std::shared_ptr<Geometry>();
				}
				return std::make_shared<PolygonGeometry>(rings[0], std::vector<std::vector<cglib::vec2<double>>>(rings.begin() + 1, rings.end()));
			}
			else if (type == 6) {
				std::size_t n = decodeNumber<std::size_t>();
				std::vector<std::shared_ptr<Geometry>> geoms;
				geoms.reserve(n);
				for (std::size_t i = 0; i < n; i++) {
					std::vector<std::vector<cglib::vec2<double>>> rings = decodeRings();
					if (rings.empty()) {
						continue;
					}
					geoms.push_back(std::make_shared<PolygonGeometry>(rings[0], std::vector<std::vector<cglib::vec2<double>>>(rings.begin() + 1, rings.end())));
				}
				return std::make_shared<MultiGeometry>(std::move(geoms));
			}
			else if (type == 7) {
				std::size_t n = decodeNumber<std::size_t>();
				std::vector<std::shared_ptr<Geometry>> geoms;
				geoms.reserve(n);
				for (std::size_t i = 0; i < n; i++) {
					geoms.push_back(decodeGeometry());
				}
				return std::make_shared<MultiGeometry>(std::move(geoms));
			}
			else {
				throw std::runtime_error("Invalid geometry type");
			}
		}

	private:
		template <typename T>
		T decodeNumber() {
			return _stream.decodeNumber<T>();
		}

		cglib::vec2<double> decodeCoord() {
			return _pointConverter(_stream.decodeDeltaCoord(1.0 / PRECISION));
		}

		std::vector<cglib::vec2<double>> decodeCoords() {
			std::size_t count = decodeNumber<std::size_t>();
			std::vector<cglib::vec2<double>> coords;
			coords.reserve(count);
			while (coords.size() < count) {
				coords.push_back(decodeCoord());
			}
			return coords;
		}

		std::vector<std::vector<cglib::vec2<double>>> decodeRings() {
			std::size_t count = decodeNumber<std::size_t>();
			std::vector<std::vector<cglib::vec2<double>>> rings;
			rings.reserve(count);
			while (rings.size() < count) {
				rings.push_back(decodeCoords());
			}
			return rings;
		}

		std::shared_ptr<Geometry> decodeGeometry() {
			int type = decodeNumber<int>();
			if (type == 0) {
				return std::shared_ptr<Geometry>();
			}
			else if (type == 1) {
				return std::make_shared<PointGeometry>(decodeCoord());
			}
			else if (type == 2) {
				std::vector<cglib::vec2<double>> coords = decodeCoords();
				std::vector<std::shared_ptr<Geometry>> geoms;
				geoms.reserve(coords.size());
				for (const cglib::vec2<double>& coord : coords) {
					geoms.push_back(std::make_shared<PointGeometry>(coord));
				}
				return std::make_shared<MultiGeometry>(std::move(geoms));
			}
			else if (type == 3) {
				std::vector<cglib::vec2<double>> coords = decodeCoords();
				return std::make_shared<LineGeometry>(std::move(coords));
			}
			else if (type == 4) {
				std::size_t n = decodeNumber<std::size_t>();
				std::vector<std::shared_ptr<Geometry>> geoms;
				geoms.reserve(n);
				for (std::size_t i = 0; i < n; i++) {
					std::vector<cglib::vec2<double>> coords = decodeCoords();
					geoms.push_back(std::make_shared<LineGeometry>(std::move(coords)));
				}
				return std::make_shared<MultiGeometry>(std::move(geoms));
			}
			else if (type == 5) {
				std::vector<std::vector<cglib::vec2<double>>> rings = decodeRings();
				if (rings.empty()) {
					return std::shared_ptr<Geometry>();
				}
				return std::make_shared<PolygonGeometry>(rings[0], std::vector<std::vector<cglib::vec2<double>>>(rings.begin() + 1, rings.end()));
			}
			else if (type == 6) {
				std::size_t n = decodeNumber<std::size_t>();
				std::vector<std::shared_ptr<Geometry>> geoms;
				geoms.reserve(n);
				for (std::size_t i = 0; i < n; i++) {
					std::vector<std::vector<cglib::vec2<double>>> rings = decodeRings();
					if (rings.empty()) {
						continue;
					}
					geoms.push_back(std::make_shared<PolygonGeometry>(rings[0], std::vector<std::vector<cglib::vec2<double>>>(rings.begin() + 1, rings.end())));
				}
				return std::make_shared<MultiGeometry>(std::move(geoms));
			}
			else if (type == 7) {
				std::size_t n = decodeNumber<std::size_t>();
				std::vector<std::shared_ptr<Geometry>> geoms;
				geoms.reserve(n);
				for (std::size_t i = 0; i < n; i++) {
					geoms.push_back(decodeGeometry());
				}
				return std::make_shared<MultiGeometry>(std::move(geoms));
			}
			else {
				throw std::runtime_error("Invalid geometry type");
			}
		}

		static constexpr double PRECISION = 1.0e5;

		EncodingStream& _stream;
		PointConverter _pointConverter;
	};
} }

#endif
