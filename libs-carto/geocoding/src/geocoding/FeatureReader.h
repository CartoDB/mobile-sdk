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
#include <vector>

namespace carto { namespace geocoding {
    class FeatureReader final {
    public:
        explicit FeatureReader(EncodingStream& stream, const PointConverter& converter) : _stream(stream), _geometryReader(stream, converter) { }

        Feature readFeature() {
            std::uint64_t id = _stream.readDeltaNumber<std::uint64_t>();
            std::shared_ptr<Geometry> geometry = _geometryReader.readGeometry();
            std::size_t n = _stream.readNumber<std::size_t>();
            std::map<std::string, Value> properties;
            for (std::size_t i = 0; i < n; i++) {
                std::string name = _stream.readString();
                Value value = readValue();
                properties[name] = value;
            }
            return Feature(id, std::move(geometry), std::move(properties));
        }

        std::vector<Feature> readFeatureCollection() {
            std::size_t n = _stream.readNumber<std::size_t>();
            std::vector<Feature> features;
            for (std::size_t i = 0; i < n; i++) {
                features.push_back(readFeature());
            }
            return features;
        }

    private:
        enum class ValueType : int {
            NONE = 0,
            BOOLEAN = 1,
            INTEGER = 2,
            FLOAT = 3,
            STRING = 4
        };

        Value readValue() {
            ValueType type = _stream.readNumber<ValueType>();
            if (type == ValueType::NONE) {
                return Value();
            }
            else if (type == ValueType::BOOLEAN) {
                return Value(_stream.readNumber<int>() != 0);
            }
            else if (type == ValueType::INTEGER) {
                return Value(_stream.readNumber<long long>());
            }
            else if (type == ValueType::FLOAT) {
                return Value(static_cast<double>(_stream.readFloat()));
            }
            else if (type == ValueType::STRING) {
                return Value(_stream.readString());
            }
            else {
                throw std::runtime_error("Invalid value type");
            }
        }

        EncodingStream& _stream;
        GeometryReader _geometryReader;
    };
} }

#endif
