#include "RevGeocoder.h"
#include "FeatureReader.h"
#include "ProjUtils.h"
#include "AddressInterpolator.h"

#include <functional>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <sqlite3pp.h>

namespace carto { namespace geocoding {
    bool RevGeocoder::import(const std::shared_ptr<sqlite3pp::database>& db) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        Database database;
        database.id = "db" + boost::lexical_cast<std::string>(_databases.size());
        database.db = db;
        database.bounds = getBounds(*db);
        database.origin = getOrigin(*db);
        _databases.push_back(database);
        return true;
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

    unsigned int RevGeocoder::getMaxResults() const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return _maxResults;
    }

    void RevGeocoder::setMaxResults(unsigned int maxResults) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        _maxResults = maxResults;
    }

    bool RevGeocoder::isFilterEnabled(Address::EntityType type) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        return std::find(_enabledFilters.begin(), _enabledFilters.end(), type) != _enabledFilters.end();
    }
    
    void RevGeocoder::setFilterEnabled(Address::EntityType type, bool enabled) {
        std::lock_guard<std::recursive_mutex> lock(_mutex);
        auto it = std::find(_enabledFilters.begin(), _enabledFilters.end(), type);
        if (enabled && it == _enabledFilters.end()) {
            _enabledFilters.push_back(type);
        }
        else if (!enabled && it != _enabledFilters.end()) {
            _enabledFilters.erase(it);
        }
    }

    std::vector<std::pair<Address, float>> RevGeocoder::findAddresses(double lng, double lat, float radius) const {
        std::lock_guard<std::recursive_mutex> lock(_mutex);

        std::vector<std::pair<Address, float>> addresses;
        for (const Database& database : _databases) {
            if (database.bounds) {
                // TODO: -180/180 wrapping
                cglib::vec2<double> lngLatMeters = wgs84Meters({ lng, lat });
                cglib::vec2<double> point = database.bounds->nearest_point({ lng, lat });
                cglib::vec2<double> diff = point - cglib::vec2<double>(lng, lat);
                double dist = cglib::length(cglib::vec2<double>(diff(0) * lngLatMeters(0), diff(1) * lngLatMeters(1)));
                if (dist > radius) {
                    continue;
                }
            }

            _previousEntityQueryCounter = _entityQueryCounter;
            QuadIndex index(std::bind(&RevGeocoder::findGeometryInfo, this, std::cref(database), std::placeholders::_1, std::placeholders::_2));
            std::vector<QuadIndex::Result> results = index.findGeometries(lng, lat, radius);

            for (const QuadIndex::Result& result : results) {
                float rank = 1.0f - static_cast<float>(result.second) / radius;
                if (rank > 0) {
                    Address address;
                    std::string addrKey = database.id + "_" + boost::lexical_cast<std::string>(result.first);
                    if (!_addressCache.read(addrKey, address)) {
                        address.loadFromDB(*database.db, result.first, _language, [&database](const cglib::vec2<double>& pos) {
                            return database.origin + pos;
                        });
                        _addressCache.put(addrKey, address);
                    }
                    addresses.emplace_back(address, rank);
                }
            }
        }

        std::sort(addresses.begin(), addresses.end(), [](const std::pair<Address, float>& addrRank1, const std::pair<Address, float>& addrRank2) {
            return addrRank1.second > addrRank2.second;
        });

        if (addresses.size() > _maxResults) {
            addresses.erase(addresses.begin() + _maxResults, addresses.end());
        }

        return addresses;
    }

    std::vector<QuadIndex::GeometryInfo> RevGeocoder::findGeometryInfo(const Database& database, const std::vector<std::uint64_t>& quadIndices, const PointConverter& converter) const {
        std::string sql = "SELECT id, features, housenumbers FROM entities WHERE quadindex in (";
        for (std::size_t i = 0; i < quadIndices.size(); i++) {
            sql += (i > 0 ? "," : "") + boost::lexical_cast<std::string>(quadIndices[i]);
        }
        sql += ")";
        if (!_enabledFilters.empty()) {
            std::string values;
            for (const Address::EntityType type : _enabledFilters) {
                values += (values.empty() ? "" : ",") + boost::lexical_cast<std::string>(static_cast<int>(type));
            }
            sql += " AND (type IN (" + values + "))";
        }

        std::vector<QuadIndex::GeometryInfo> geomInfos;
        std::string queryKey = database.id + "_" + sql;
        if (_queryCache.read(queryKey, geomInfos)) {
            return geomInfos;
        }

        sqlite3pp::query query(*database.db, sql.c_str());
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            auto entityId = qit->get<unsigned int>(0);

            EncodingStream featureStream(qit->get<const void*>(1), qit->column_bytes(1));
            FeatureReader featureReader(featureStream, [&database, &converter](const cglib::vec2<double>& pos) {
                return converter(database.origin + pos);
            });

            if (qit->get<const void*>(2)) {
                EncodingStream houseNumberStream(qit->get<const void*>(2), qit->column_bytes(2));
                AddressInterpolator interpolator(houseNumberStream);

                std::vector<std::pair<std::uint64_t, std::vector<Feature>>> results = interpolator.enumerateAddresses(featureReader);
                for (std::size_t i = 0; i < results.size(); i++) {
                    std::uint64_t encodedId = (results[i].first ? static_cast<std::uint64_t>(i + 1) << 32 : 0) | entityId;
                    std::vector<std::shared_ptr<Geometry>> geometries;
                    for (const Feature& feature : results[i].second) {
                        if (feature.getGeometry()) {
                            geometries.push_back(feature.getGeometry());
                        }
                    }
                    geomInfos.emplace_back(encodedId, std::make_shared<MultiGeometry>(std::move(geometries)));
                }
            }
            else {
                std::vector<std::shared_ptr<Geometry>> geometries;
                for (const Feature& feature : featureReader.readFeatureCollection()) {
                    if (feature.getGeometry()) {
                        geometries.push_back(feature.getGeometry());
                    }
                }
                geomInfos.emplace_back(entityId, std::make_shared<MultiGeometry>(std::move(geometries)));
            }
        }

        _entityQueryCounter++;
        _queryCache.put(queryKey, geomInfos);
        return geomInfos;
    }

    cglib::vec2<double> RevGeocoder::getOrigin(sqlite3pp::database& db) {
        sqlite3pp::query query(db, "SELECT value FROM metadata WHERE name='origin'");
        for (auto qit = query.begin(); qit != query.end(); qit++) {
            std::string value = qit->get<const char*>(0);

            std::vector<std::string> origin;
            boost::split(origin, value, boost::is_any_of(","), boost::token_compress_off);
            return cglib::vec2<double>(boost::lexical_cast<double>(origin.at(0)), boost::lexical_cast<double>(origin.at(1)));
        }
        return cglib::vec2<double>(0, 0);
    }

    boost::optional<cglib::bbox2<double>> RevGeocoder::getBounds(sqlite3pp::database& db) {
        sqlite3pp::query query(db, "SELECT value FROM metadata WHERE name='bounds'");
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
} }
