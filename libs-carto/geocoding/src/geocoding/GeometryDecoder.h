/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOCODING_GEOMETRYDECODER_H_
#define _CARTO_GEOCODING_GEOMETRYDECODER_H_

#include "Geometry.h"

#include <vector>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace carto { namespace geocoding {
	inline std::vector<cglib::vec2<double>> decodePolyLine(const std::string& str) {
		std::vector<cglib::vec2<double>> points;
		points.reserve(str.size() / 4 + 16);

		int index = 0;
		int prev[2] = { 0, 0 };
		for (std::size_t pos = 0; pos < str.size(); ) {
			int val = 0, shift = 0, result = 0;
			do {
				if (pos >= str.size()) {
					throw std::runtime_error("Bad encoded polyline");
				}
				val = static_cast<int>(str[pos++]) - 63;
				result |= (val & 0x1f) << shift;
				shift += 5;
			} while (val >= 0x20);
			prev[index] += (result & 1 ? ~(result >> 1) : (result >> 1));
			index = index ^ 1;
			if (index == 0) {
				points.emplace_back(prev[0] * 1.0e-5, prev[1] * 1.0e-5);
			}
		}

		return points;
	}

	inline std::shared_ptr<Geometry> decodeGeometry(const std::string& str) {
		if (str.empty()) {
			return std::shared_ptr<Geometry>();
		}

		char type = str.front();
		std::string payload = str.substr(1);
		if (type == '0') {
			std::vector<cglib::vec2<double>> points = decodePolyLine(payload);
			return std::make_shared<PointGeometry>(points.front());
		}
		else if (type == '1') {
			std::vector<cglib::vec2<double>> points = decodePolyLine(payload);
			std::vector<std::shared_ptr<Geometry>> geoms;
			geoms.reserve(points.size());
			for (const cglib::vec2<double>& point : points) {
				geoms.push_back(std::make_shared<PointGeometry>(point));
			}
			return std::make_shared<MultiGeometry>(std::move(geoms));
		}
		else if (type == '2') {
			std::vector<cglib::vec2<double>> points = decodePolyLine(payload);
			return std::make_shared<LineGeometry>(std::move(points));
		}
		else if (type == '3') {
			std::vector<std::string> parts;
			boost::split(parts, payload, boost::is_any_of(","), boost::token_compress_off);
			std::vector<std::shared_ptr<Geometry>> geoms;
			geoms.reserve(parts.size());
			for (const std::string& part : parts) {
				std::vector<cglib::vec2<double>> points = decodePolyLine(part);
				geoms.push_back(std::make_shared<LineGeometry>(std::move(points)));
			}
			return std::make_shared<MultiGeometry>(std::move(geoms));
		}
		else if (type == '4') {
			std::vector<std::vector<cglib::vec2<double>>> pointLists;
			for (auto it = boost::make_split_iterator(payload, boost::token_finder(boost::is_any_of(";"))); it != boost::split_iterator<std::string::iterator>(); it++) {
				std::vector<cglib::vec2<double>> points = decodePolyLine(boost::copy_range<std::string>(*it));
				pointLists.push_back(std::move(points));
			}
			if (!pointLists.empty()) {
				return std::make_shared<PolygonGeometry>(std::move(pointLists.front()), std::vector<std::vector<Geometry::Point>>(pointLists.begin() + 1, pointLists.end()));
			}
		}
		else if (type == '5') {
			std::vector<std::string> parts;
			boost::split(parts, payload, boost::is_any_of(","), boost::token_compress_off);
			std::vector<std::shared_ptr<Geometry>> geoms;
			geoms.reserve(parts.size());
			for (const std::string& part : parts) {
				std::vector<std::vector<cglib::vec2<double>>> pointLists;
				for (auto it = boost::make_split_iterator(part, boost::token_finder(boost::is_any_of(";"))); it != boost::split_iterator<std::string::const_iterator>(); it++) {
					std::vector<cglib::vec2<double>> points = decodePolyLine(boost::copy_range<std::string>(*it));
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
