#include "RevGeocoder.h"
#include "GeometryDecoder.h"
#include "ProjUtils.h"

#include <functional>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <sqlite3pp.h>

namespace carto { namespace geocoding {
	boost::optional<Address> RevGeocoder::findAddress(double lng, double lat) const {
		std::lock_guard<std::recursive_mutex> lock(_mutex);

		if (_bounds) {
			// TODO: -180/180 wrapping
			cglib::vec2<double> lngLatMeters = wgs84Meters({ lng, lat });
			cglib::vec2<double> point = _bounds->nearest_point({ lng, lat });
			double dist = cglib::length(cglib::vec2<double>(lngLatMeters(0) * (point(0) - lng), lngLatMeters(1) * (point(1) - lat)));
			if (dist > _radius) {
				return boost::optional<Address>();
			}
		}

		QuadIndex index(std::bind(&RevGeocoder::findFeatures, this, std::placeholders::_1));
		std::vector<QuadIndex::Result> results = index.findGeometries(lng, lat, _radius);
		if (results.empty()) {
			return boost::optional<Address>();
		}

		Address address;
		address.loadFromDB(_db, results.front().first);
		return address;
	}

	boost::optional<cglib::bbox2<double>> RevGeocoder::findBounds() const {
		sqlite3pp::query query(_db, "SELECT value FROM metadata WHERE name='bounds'");
		for (auto qit = query.begin(); qit != query.end(); qit++) {
			std::string value = qit->get<const char*>(0);
			
			std::vector<std::string> bounds;
			boost::split(bounds, value, boost::is_any_of(","), boost::token_compress_off);
			cglib::vec2<double> min(boost::lexical_cast<double>(bounds.at(0)), boost::lexical_cast<double>(bounds.at(1)));
			cglib::vec2<double> max(boost::lexical_cast<double>(bounds.at(2)), boost::lexical_cast<double>(bounds.at(3)));
			return cglib::bbox2<double>(min, max);
		}
		return boost::optional<cglib::bbox2<double>>();
	}

	std::vector<QuadIndex::Feature> RevGeocoder::findFeatures(const std::vector<long long>& quadIndices) const {
		std::string sql = "SELECT rowid, geometry, housenums FROM entities WHERE quadindex in (";
		for (std::size_t i = 0; i < quadIndices.size(); i++) {
			if (i > 0) {
				sql += ", ";
			}
			sql += boost::lexical_cast<std::string>(quadIndices[i]);
		}
		sql += ") AND (housenums IS NOT NULL OR name IS NOT NULL)";

		std::vector<QuadIndex::Feature> features;
		if (_queryCache.read(sql, features)) {
			return features;
		}

		sqlite3pp::query query(_db, sql.c_str());
		for (auto qit = query.begin(); qit != query.end(); qit++) {
			long long rowId = qit->get<long long>(0);
			const char* encodedGeometry = qit->get<const char*>(1);
			std::shared_ptr<Geometry> geometry = decodeGeometry(encodedGeometry);
			if (const char* houseNums = qit->get<const char*>(1)) {
				if (auto multiGeometry = std::dynamic_pointer_cast<MultiGeometry>(geometry)) {
					for (std::size_t i = 0; i < multiGeometry->getGeometries().size(); i++) {
						features.emplace_back((rowId << 32) | (i + 1), multiGeometry->getGeometries()[i]);
					}
					geometry.reset();
				}
			}
			if (geometry) {
				features.emplace_back((rowId << 32) | 0, geometry);
			}
		}

		_queryCache.put(sql, features);
		return features;
	}
} }
