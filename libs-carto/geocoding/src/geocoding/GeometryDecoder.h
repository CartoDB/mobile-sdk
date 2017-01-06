/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODING_GEOMETRYDECODER_H_
#define _CARTO_GEOCODING_GEOMETRYDECODER_H_

#include "Geometry.h"

#include <vector>
#include <functional>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace carto { namespace geocoding {
	using PointConverter = std::function<cglib::vec2<double>(const cglib::vec2<double>&)>;

	inline std::vector<cglib::vec2<double>> decodePolyLine(const std::string& encodedPoints, const PointConverter& converter) {
		std::vector<cglib::vec2<double>> points;
		points.reserve(encodedPoints.size() / 4 + 16);

		int index = 0;
		int prev[2] = { 0, 0 };
		for (std::size_t pos = 0; pos < encodedPoints.size(); ) {
			int val = 0, shift = 0, result = 0;
			do {
				if (pos >= encodedPoints.size()) {
					throw std::runtime_error("Bad encoded polyline");
				}
				val = static_cast<int>(encodedPoints[pos++]) - 63;
				result |= (val & 0x1f) << shift;
				shift += 5;
			} while (val >= 0x20);
			prev[index] += (result & 1 ? ~(result >> 1) : (result >> 1));
			index = index ^ 1;
			if (index == 0) {
				points.push_back(converter({ prev[1] * 1.0e-5, prev[0] * 1.0e-5 }));
			}
		}

		points.shrink_to_fit();
		return points;
	}

	inline std::shared_ptr<Geometry> decodeGeometry(const std::string& encodedGeometry, const PointConverter& converter) {
		if (encodedGeometry.empty()) {
			return std::shared_ptr<Geometry>();
		}

		char type = encodedGeometry.front();
		std::string data = encodedGeometry.substr(1);
		if (type == '0') {
			std::vector<cglib::vec2<double>> points = decodePolyLine(data, converter);
			return std::make_shared<PointGeometry>(points.front());
		}
		else if (type == '1') {
			std::vector<cglib::vec2<double>> points = decodePolyLine(data, converter);
			std::vector<std::shared_ptr<Geometry>> geoms;
			geoms.reserve(points.size());
			for (const cglib::vec2<double>& point : points) {
				geoms.push_back(std::make_shared<PointGeometry>(point));
			}
			return std::make_shared<MultiGeometry>(std::move(geoms));
		}
		else if (type == '2') {
			std::vector<cglib::vec2<double>> points = decodePolyLine(data, converter);
			return std::make_shared<LineGeometry>(std::move(points));
		}
		else if (type == '3') {
			std::vector<std::string> parts;
			boost::split(parts, data, boost::is_any_of(","), boost::token_compress_off);
			std::vector<std::shared_ptr<Geometry>> geoms;
			geoms.reserve(parts.size());
			for (const std::string& part : parts) {
				std::vector<cglib::vec2<double>> points = decodePolyLine(part, converter);
				geoms.push_back(std::make_shared<LineGeometry>(std::move(points)));
			}
			return std::make_shared<MultiGeometry>(std::move(geoms));
		}
		else if (type == '4') {
			std::vector<std::vector<cglib::vec2<double>>> pointLists;
			for (auto it = boost::make_split_iterator(data, boost::token_finder(boost::is_any_of(";"))); it != boost::split_iterator<std::string::iterator>(); it++) {
				std::vector<cglib::vec2<double>> points = decodePolyLine(boost::copy_range<std::string>(*it), converter);
				pointLists.push_back(std::move(points));
			}
			if (!pointLists.empty()) {
				return std::make_shared<PolygonGeometry>(std::move(pointLists.front()), std::vector<std::vector<Geometry::Point>>(pointLists.begin() + 1, pointLists.end()));
			}
		}
		else if (type == '5') {
			std::vector<std::string> parts;
			boost::split(parts, data, boost::is_any_of(","), boost::token_compress_off);
			std::vector<std::shared_ptr<Geometry>> geoms;
			geoms.reserve(parts.size());
			for (const std::string& part : parts) {
				std::vector<std::vector<cglib::vec2<double>>> pointLists;
				for (auto it = boost::make_split_iterator(part, boost::token_finder(boost::is_any_of(";"))); it != boost::split_iterator<std::string::const_iterator>(); it++) {
					std::vector<cglib::vec2<double>> points = decodePolyLine(boost::copy_range<std::string>(*it), converter);
					pointLists.push_back(std::move(points));
				}
				if (!pointLists.empty()) {
					geoms.push_back(std::make_shared<PolygonGeometry>(std::move(pointLists.front()), std::vector<std::vector<Geometry::Point>>(pointLists.begin() + 1, pointLists.end())));
				}
			}
			return std::make_shared<MultiGeometry>(std::move(geoms));
		}
		return std::shared_ptr<Geometry>();
	}
} }

#endif
