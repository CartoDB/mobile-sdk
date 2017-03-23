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
            GeometryType type = readNumber<GeometryType>();
            if (type == GeometryType::NONE) {
                return std::shared_ptr<Geometry>();
            }
            else if (type == GeometryType::POINT) {
                return std::make_shared<PointGeometry>(readCoord());
            }
            else if (type == GeometryType::MULTIPOINT) {
                std::vector<cglib::vec2<double>> coords = readCoords();
                std::vector<std::shared_ptr<Geometry>> geoms;
                geoms.reserve(coords.size());
                for (const cglib::vec2<double>& coord : coords) {
                    geoms.push_back(std::make_shared<PointGeometry>(coord));
                }
                return std::make_shared<MultiGeometry>(std::move(geoms));
            }
            else if (type == GeometryType::LINESTRING) {
                std::vector<cglib::vec2<double>> coords = readCoords();
                return std::make_shared<LineGeometry>(std::move(coords));
            }
            else if (type == GeometryType::MULTILINESTRING) {
                std::size_t n = readNumber<std::size_t>();
                std::vector<std::shared_ptr<Geometry>> geoms;
                geoms.reserve(n);
                for (std::size_t i = 0; i < n; i++) {
                    std::vector<cglib::vec2<double>> coords = readCoords();
                    geoms.push_back(std::make_shared<LineGeometry>(std::move(coords)));
                }
                return std::make_shared<MultiGeometry>(std::move(geoms));
            }
            else if (type == GeometryType::POLYGON) {
                std::vector<std::vector<cglib::vec2<double>>> rings = readRings();
                if (rings.empty()) {
                    return std::shared_ptr<Geometry>();
                }
                return std::make_shared<PolygonGeometry>(rings[0], std::vector<std::vector<cglib::vec2<double>>>(rings.begin() + 1, rings.end()));
            }
            else if (type == GeometryType::MULTIPOLYGON) {
                std::size_t n = readNumber<std::size_t>();
                std::vector<std::shared_ptr<Geometry>> geoms;
                geoms.reserve(n);
                for (std::size_t i = 0; i < n; i++) {
                    std::vector<std::vector<cglib::vec2<double>>> rings = readRings();
                    if (rings.empty()) {
                        continue;
                    }
                    geoms.push_back(std::make_shared<PolygonGeometry>(rings[0], std::vector<std::vector<cglib::vec2<double>>>(rings.begin() + 1, rings.end())));
                }
                return std::make_shared<MultiGeometry>(std::move(geoms));
            }
            else if (type == GeometryType::GEOMETRYCOLLECTION) {
                std::size_t n = readNumber<std::size_t>();
                std::vector<std::shared_ptr<Geometry>> geoms;
                geoms.reserve(n);
                for (std::size_t i = 0; i < n; i++) {
                    geoms.push_back(readGeometry());
                }
                return std::make_shared<MultiGeometry>(std::move(geoms));
            }
            else {
                throw std::runtime_error("Invalid geometry type");
            }
        }

    private:
        enum class GeometryType : int {
            NONE               = 0,
            POINT              = 1,
            MULTIPOINT         = 2,
            LINESTRING         = 3,
            MULTILINESTRING    = 4,
            POLYGON            = 5,
            MULTIPOLYGON       = 6,
            GEOMETRYCOLLECTION = 7
        };

        template <typename T>
        T readNumber() {
            return _stream.readNumber<T>();
        }

        cglib::vec2<double> readCoord() {
            return _pointConverter(_stream.readDeltaCoord(1.0 / PRECISION));
        }

        std::vector<cglib::vec2<double>> readCoords() {
            std::size_t count = readNumber<std::size_t>();
            std::vector<cglib::vec2<double>> coords;
            coords.reserve(count);
            while (coords.size() < count) {
                coords.push_back(readCoord());
            }
            return coords;
        }

        std::vector<std::vector<cglib::vec2<double>>> readRings() {
            std::size_t count = readNumber<std::size_t>();
            std::vector<std::vector<cglib::vec2<double>>> rings;
            rings.reserve(count);
            while (rings.size() < count) {
                rings.push_back(readCoords());
            }
            return rings;
        }

        static constexpr double PRECISION = 1.0e6;

        EncodingStream& _stream;
        PointConverter _pointConverter;
    };
} }

#endif
