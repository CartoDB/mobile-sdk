#include "RevGeocoder.h"
#include "FeatureReader.h"
#include "ProjUtils.h"

#include <functional>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <sqlite3pp.h>

namespace carto { namespace geocoding {
    float RevGeocoder::getRadius() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _radius;
    }

    void RevGeocoder::setRadius(float radius) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _radius = radius;
    }

    std::string RevGeocoder::getLanguage() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _language;
    }

    void RevGeocoder::setLanguage(const std::string& language) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _language = language;
        _addressCache.clear();
    }

    std::vector<std::pair<Address, float>> RevGeocoder::findAddresses(double lng, double lat) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        if (_bounds) {
            // TODO: -180/180 wrapping
            cglib::vec2<double> lngLatMeters = wgs84Meters({ lng, lat });
            cglib::vec2<double> point = _bounds->nearest_point({ lng, lat });
            cglib::vec2<double> diff = point - cglib::vec2<double>(lng, lat);
            double dist = cglib::length(cglib::vec2<double>(diff(0) * lngLatMeters(0), diff(1) * lngLatMeters(1)));
            if (dist > _radius) {
                return std::vector<std::pair<Address, float>>();
            }
        }

        _previousEntityQueryCounter = _entityQueryCounter;
        QuadIndex index(std::bind(&RevGeocoder::findGeometryInfo, this, std::placeholders::_1, std::placeholders::_2));
        std::vector<QuadIndex::Result> results = index.findGeometries(lng, lat, _radius);

        std::vector<std::pair<Address, float>> addresses;
        for (const QuadIndex::Result& result : results) {
            float rank = 1.0f - static_cast<float>(result.second) / _radius;
            if (rank > 0) {
                Address address;
                if (!_addressCache.read(result.first, address)) {
                    address.loadFromDB(_db, result.first, _language, [this](const cglib::vec2<double>& pos) {
                        return _origin + pos;
                    });
                    _addressCache.put(result.first, address);
                }
                addresses.emplace_back(address, rank);
            }
        }
        return addresses;
    }

    cglib::vec2<double> RevGeocoder::findOrigin() const {
        sqlite3pp::query query(_db, "SELECT value FROM metadata WHERE name='origin'");
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            std::string value = qit->get<const char*>(0);

            std::vector<std::string> origin;
            boost::split(origin, value, boost::is_any_of(","), boost::token_compress_off);
            return cglib::vec2<double>(boost::lexical_cast<double>(origin.at(0)), boost::lexical_cast<double>(origin.at(1)));
        }
        return cglib::vec2<double>(0, 0);
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

    std::vector<QuadIndex::GeometryInfo> RevGeocoder::findGeometryInfo(const std::vector<std::uint64_t>& quadIndices, const PointConverter& converter) const {
        std::string sql = "SELECT id, features FROM entities WHERE quadindex in (";
        for (std::size_t i = 0; i < quadIndices.size(); i++) {
            if (i > 0) {
                sql += ", ";
            }
            sql += boost::lexical_cast<std::string>(quadIndices[i]);
        }
        sql += ") AND (housenumbers IS NOT NULL OR name_id IS NOT NULL)";

        std::vector<QuadIndex::GeometryInfo> geomInfos;
        if (_queryCache.read(sql, geomInfos)) {
            return geomInfos;
        }

        sqlite3pp::query query(_db, sql.c_str());
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            auto entityId = qit->get<unsigned int>(0);
            if (auto encodedFeatures = qit->get<const void*>(1)) {
                EncodingStream stream(encodedFeatures, qit->column_bytes(1));
                FeatureReader reader(stream, [this, &converter](const cglib::vec2<double>& pos) {
                    return converter(_origin + pos);
                });
                for (unsigned int elementIndex = 1; !stream.eof(); elementIndex++) {
                    std::uint64_t encodedId = (static_cast<std::uint64_t>(elementIndex) << 32) | entityId;

                    std::vector<std::shared_ptr<Geometry>> geometries;
                    for (const Feature& feature : reader.readFeatureCollection()) {
                        if (std::shared_ptr<Geometry> geometry = feature.getGeometry()) {
                            geometries.push_back(geometry);
                        }
                    }
                    if (geometries.size() == 1) {
                        geomInfos.emplace_back(encodedId, geometries.front());
                    }
                    else if (!geometries.empty()) {
                        geomInfos.emplace_back(encodedId, std::make_shared<MultiGeometry>(std::move(geometries)));
                    }
                }
            }
        }

        _entityQueryCounter++;
        _queryCache.put(sql, geomInfos);
        return geomInfos;
    }
} }
