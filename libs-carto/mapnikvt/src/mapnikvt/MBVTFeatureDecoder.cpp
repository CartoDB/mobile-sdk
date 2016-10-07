#include "MBVTFeatureDecoder.h"
#include "Logger.h"

#include "mbvtpackage/MBVTPackage.pb.h"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <atomic>
#include <list>
#include <vector>
#include <map>
#include <unordered_map>
#include <utility>
#include <algorithm>
#include <limits>

#define MINIZ_HEADER_FILE_ONLY
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#include <miniz.c>

namespace carto { namespace mvt {
    class MBVTFeatureDecoder::MBVTFeatureIterator : public carto::mvt::FeatureDecoder::FeatureIterator {
    public:
        explicit MBVTFeatureIterator(const vector_tile::Tile& tile, const vector_tile::Tile::Layer& layer, const std::unordered_set<std::string>* fields, const cglib::mat3x3<float>& transform, const cglib::bbox2<float>& clipBox, float buffer, bool idOverride, std::map<std::vector<int>, std::shared_ptr<FeatureData>>& featureDataCache) :
            _tile(tile), _layer(layer), _transform(transform), _clipBox(clipBox), _buffer(buffer), _idOverride(idOverride), _featureDataCache(featureDataCache)
        {
            for (int i = 0; i < tile.layers_size(); i++) {
                if (&tile.layers(i) == &layer) {
                    _tileIndexBase = static_cast<long long>(i) << 32;
                    break;
                }
            }

            for (int i = 0; i < layer.keys_size(); i++) {
                if (layer.keys(i) == "id" || layer.keys(i) == "cartodb_id") {
                    _idKey = i;
                }
                if (fields) {
                    auto it = fields->find(layer.keys(i));
                    if (it != fields->end()) {
                        _fieldKeys.push_back(i);
                    }
                }
                else {
                    _fieldKeys.push_back(i);
                }
            }
        }

        bool findByLocalId(long long localId) {
            if (localId >= _tileIndexBase && localId < _tileIndexBase + _layer.features_size()) {
                _index = static_cast<std::size_t>(localId - _tileIndexBase);
                return true;
            }
            return false;
        }

        virtual bool valid() const override {
            return _index < _layer.features_size();
        }

        virtual void advance() override {
            _index++;
        }

        virtual long long getLocalId() const override {
            return _tileIndexBase + _index;
        }

        virtual long long getGlobalId() const override {
            if (_idOverride) {
                return _idCounter++;
            }

            const vector_tile::Tile::Feature& feature = _layer.features(_index);
            if (feature.id() != 0) {
                return feature.id();
            }
            for (int i = 0; i + 1 < feature.tags_size(); i += 2) {
                if (feature.tags(i) == _idKey) {
                    int valueIdx = feature.tags(i + 1);
                    if (valueIdx >= 0 && valueIdx < _layer.values_size()) {
                        const vector_tile::Tile::Value& value = _layer.values(valueIdx);
                        if (value.has_int_value()) {
                            return static_cast<long long>(value.int_value());
                        }
                        else if (value.has_sint_value()) {
                            return static_cast<long long>(value.sint_value());
                        }
                        else if (value.has_uint_value()) {
                            return static_cast<long long>(value.uint_value());
                        }
                        return 0;
                    }
                }
            }
            return 0;
        }

        virtual std::shared_ptr<const FeatureData> getFeatureData() const override {
            const vector_tile::Tile::Feature& feature = _layer.features(_index);
            std::vector<int> tags(_fieldKeys.size() + 1, -1);
            tags.back() = static_cast<int>(feature.type());
            for (int i = 0; i + 1 < feature.tags_size(); i += 2) {
                auto it = std::find(_fieldKeys.begin(), _fieldKeys.end(), feature.tags(i));
                if (it != _fieldKeys.end()) {
                    tags[it - _fieldKeys.begin()] = feature.tags(i + 1);
                }
            }

            auto it = _featureDataCache.find(tags);
            if (it != _featureDataCache.end()) {
                return it->second;
            }

            FeatureData::GeometryType geomType = convertGeometryType(feature.type());
            std::vector<std::pair<std::string, Value>> dataMap;
            dataMap.reserve(tags.size());
            for (std::size_t i = 0; i < _fieldKeys.size(); i++) {
                if (tags[i] >= 0 && tags[i] < _layer.values_size()) {
                    dataMap.emplace_back(_layer.keys(_fieldKeys[i]), convertValue(_layer.values(tags[i])));
                }
            }

            auto featureData = std::make_shared<FeatureData>(geomType, std::move(dataMap));
            _featureDataCache.emplace(std::move(tags), featureData);
            return featureData;
        }

        virtual std::shared_ptr<const Geometry> getGeometry() const override {
            std::vector<std::vector<cglib::vec2<float>>> verticesList;
            decodeGeometry(_layer.features(_index), verticesList, 1.0f / _layer.extent());
            if (_buffer > 0 && _layer.features(_index).type() == vector_tile::Tile::LINESTRING) {
                bufferGeometry(verticesList, _buffer);
            }

            cglib::bbox2<float> bbox = cglib::bbox2<float>::smallest();
            for (std::vector<cglib::vec2<float>>& vertices : verticesList) {
                for (cglib::vec2<float>& p : vertices) {
                    p = cglib::transform_point(p, _transform);
                    bbox.add(p);
                }
            }
            if (!bbox.inside(_clipBox)) {
                return std::shared_ptr<Geometry>();
            }

            switch (_layer.features(_index).type()) {
            case vector_tile::Tile::POINT:
                if (!verticesList.empty()) {
                    return std::make_shared<PointGeometry>(std::move(verticesList.front()));
                }
                return std::shared_ptr<Geometry>();
            case vector_tile::Tile::LINESTRING:
                return std::make_shared<LineGeometry>(std::move(verticesList));
            case vector_tile::Tile::POLYGON: {
                PolygonGeometry::PolygonList polygons;
                if (_layer.has_version() && _layer.version() > 1) {
                    auto it = std::find_if(verticesList.begin(), verticesList.end(), isRingCCW); // find first outer ring
                    while (it != verticesList.end()) {
                        auto it0 = it++;
                        it = std::find_if(it, verticesList.end(), isRingCCW); // find next outer ring
                        polygons.emplace_back(it0, it);
                    }
                }
                else {
                    polygons.push_back(std::move(verticesList));
                }
                return std::make_shared<PolygonGeometry>(std::move(polygons));
            }
            default:
                return std::shared_ptr<Geometry>();
            }
        }

    private:
        static FeatureData::GeometryType convertGeometryType(vector_tile::Tile::GeomType geomType) {
            switch (geomType) {
            case vector_tile::Tile::POINT:
                return FeatureData::GeometryType::POINT_GEOMETRY;
            case vector_tile::Tile::LINESTRING:
                return FeatureData::GeometryType::LINE_GEOMETRY;
            case vector_tile::Tile::POLYGON:
                return FeatureData::GeometryType::POLYGON_GEOMETRY;
            default:
                return FeatureData::GeometryType::NULL_GEOMETRY;
            }
        }

        static Value convertValue(const vector_tile::Tile::Value& val) {
            if (val.has_bool_value()) {
                return Value(val.bool_value());
            }
            else if (val.has_int_value()) {
                return Value(static_cast<long long>(val.int_value()));
            }
            else if (val.has_sint_value()) {
                return Value(static_cast<long long>(val.sint_value()));
            }
            else if (val.has_uint_value()) {
                return Value(static_cast<long long>(val.uint_value()));
            }
            else if (val.has_float_value()) {
                return Value(static_cast<double>(val.float_value()));
            }
            else if (val.has_double_value()) {
                return Value(val.double_value());
            }
            else if (val.has_string_value()) {
                return Value(val.string_value());
            }
            return Value();
        }

        static void decodeGeometry(const vector_tile::Tile::Feature& feature, std::vector<std::vector<cglib::vec2<float>>>& verticesList, float scale) {
            int cx = 0, cy = 0;
            int cmd = 0, length = 0;
            std::vector<cglib::vec2<float>> vertices;
            vertices.reserve(feature.geometry_size());
            for (int i = 0; i < feature.geometry_size(); ) {
                if (length == 0) {
                    int cmdLength = feature.geometry(i++);
                    length = cmdLength >> 3;
                    cmd = cmdLength & 7;
                    if (length == 0) {
                        continue;
                    }
                }

                length--;
                if ((cmd == 1 || cmd == 2) && i + 2 <= feature.geometry_size()) {
                    if (cmd == 1) {
                        if (!vertices.empty()) {
                            verticesList.emplace_back();
                            std::swap(verticesList.back(), vertices);
                        }
                    }
                    int dx = feature.geometry(i++);
                    int dy = feature.geometry(i++);
                    dx = ((dx >> 1) ^ (-(dx & 1)));
                    dy = ((dy >> 1) ^ (-(dy & 1)));
                    cx += dx;
                    cy += dy;
                    vertices.emplace_back(static_cast<float>(cx) * scale, static_cast<float>(cy) * scale);
                }
                else if (cmd == 7) {
                    if (!vertices.empty()) {
                        if (vertices.front() != vertices.back()) {
                            cglib::vec2<float> p = vertices.front();
                            vertices.emplace_back(p);
                        }
                    }
                }
            }
            if (!vertices.empty()) {
                verticesList.emplace_back();
                std::swap(verticesList.back(), vertices);
            }
        }

        static void bufferGeometry(std::vector<std::vector<cglib::vec2<float>>>& verticesList, float buffer) {
            for (std::vector<cglib::vec2<float>>& vertices : verticesList) {
                if (vertices.size() < 2) {
                    continue;
                }

                {
                    cglib::vec2<float>& p0 = vertices[0];
                    const cglib::vec2<float>& p1 = vertices[1];
                    if (p0(0) <= 0 || p0(0) >= 1 || p0(1) <= 0 || p0(1) >= 1) {
                        if (p1 != p0) {
                            p0 = p0 + cglib::unit(p0 - p1) * buffer;
                        }
                    }
                }

                {
                    const cglib::vec2<float>& p0 = vertices[vertices.size() - 2];
                    cglib::vec2<float>& p1 = vertices[vertices.size() - 1];
                    if (p1(0) <= 0 || p1(0) >= 1 || p1(1) <= 0 || p1(1) >= 1) {
                        if (p1 != p0) {
                            p1 = p1 + cglib::unit(p1 - p0) * buffer;
                        }
                    }
                }
            }
        }

        static bool isRingCCW(const std::vector<cglib::vec2<float>>& vertices) {
            double area = 0;
            if (!vertices.empty()) {
                for (std::size_t i = 1; i < vertices.size(); i++) {
                    area += vertices[i - 1](0) * vertices[i](1) - vertices[i](0) * vertices[i - 1](1);
                }
                area += vertices.back()(0) * vertices.front()(1) - vertices.front()(0) * vertices.back()(1);
            }
            return area > 0;
        }

        int _index = 0;
        int _idKey = -1;
        long long _tileIndexBase = 0;
        std::vector<int> _fieldKeys;
        const vector_tile::Tile& _tile;
        const vector_tile::Tile::Layer& _layer;
        const cglib::mat3x3<float> _transform;
        const cglib::bbox2<float> _clipBox;
        const float _buffer;
        const bool _idOverride;
        std::map<std::vector<int>, std::shared_ptr<FeatureData>>& _featureDataCache;

        static std::atomic<long long> _idCounter;
    };

    std::atomic<long long> MBVTFeatureDecoder::MBVTFeatureIterator::_idCounter = ATOMIC_VAR_INIT(1);

    MBVTFeatureDecoder::MBVTFeatureDecoder(const std::vector<unsigned char>& data, std::shared_ptr<Logger> logger) :
        _transform(cglib::mat3x3<float>::identity()), _clipBox(cglib::vec2<float>(-0.1f, -0.1f), cglib::vec2<float>(1.1f, 1.1f)), _buffer(0), _idOverride(false), _tile(), _layerMap(), _logger(std::move(logger))
    {
        std::vector<unsigned char> pbfData;
        pbfData.reserve(data.size() * 3);
        if (inflate(data, pbfData)) {
            protobuf::message tileMsg(pbfData.data(), pbfData.size());
            _tile = std::make_shared<vector_tile::Tile>(tileMsg);
        }
        else {
            protobuf::message tileMsg(data.data(), data.size());
            _tile = std::make_shared<vector_tile::Tile>(tileMsg);
        }

        for (int i = 0; i < _tile->layers_size(); i++) {
            const std::string& name = _tile->layers(i).name();
            if (_layerMap.find(name) != _layerMap.end()) {
                _logger->write(Logger::Severity::ERROR, "Duplicate layer name: " + name);
            }
            else {
                _layerMap[name] = i;
            }
        }
    }

    void MBVTFeatureDecoder::setTransform(const cglib::mat3x3<float>& transform) {
        _transform = transform;
    }

    void MBVTFeatureDecoder::setClipBox(const cglib::bbox2<float>& clipBox) {
        _clipBox = clipBox;
    }

    void MBVTFeatureDecoder::setBuffer(float buffer) {
        _buffer = buffer;
    }

    void MBVTFeatureDecoder::setFeatureIdOverride(bool idOverride) {
        _idOverride = idOverride;
    }

    std::shared_ptr<Feature> MBVTFeatureDecoder::getFeature(long long localId, std::string& layerName) const {
        for (int i = 0; i < _tile->layers_size(); i++) {
            std::map<std::vector<int>, std::shared_ptr<FeatureData>> featureDataCache;
            MBVTFeatureIterator it(*_tile, _tile->layers(i), nullptr, _transform, _clipBox, _buffer, _idOverride, featureDataCache);
            if (it.findByLocalId(localId)) {
                 layerName = _tile->layers(i).name();
                 return std::make_shared<Feature>(it.getGlobalId(), it.getGeometry(), it.getFeatureData());
            }
        }
        return std::shared_ptr<Feature>();
    }

    std::shared_ptr<FeatureDecoder::FeatureIterator> MBVTFeatureDecoder::createLayerFeatureIterator(const std::string& name, const std::unordered_set<std::string>& fields) const {
        auto layerIt = _layerMap.find(name);
        if (layerIt == _layerMap.end()) {
            return std::shared_ptr<FeatureIterator>();
        }
        if (_layerFeatureDataCache.find(name) == _layerFeatureDataCache.end()) { // flush the cache if previous layer was different
            _layerFeatureDataCache.clear();
        }
        const vector_tile::Tile::Layer& layer = _tile->layers(layerIt->second);
        std::map<std::vector<int>, std::shared_ptr<FeatureData>>& featureDataCache = _layerFeatureDataCache[name];
        return std::make_shared<MBVTFeatureIterator>(*_tile, layer, &fields, _transform, _clipBox, _buffer, _idOverride, featureDataCache);
    }

    bool MBVTFeatureDecoder::inflate(const std::vector<unsigned char>& in, std::vector<unsigned char>& out) {
        if (in.size() < 14) {
            return false;
        }

        std::size_t offset = 0;
        if (in[0] != 0x1f || in[1] != 0x8b) {
            return false;
        }
        if (in[2] != 8) {
            return false;
        }
        int flags = in[3];
        offset += 10;
        if (flags & (1 << 2)) { // FEXTRA
            int n = static_cast<int>(in[offset + 0]) | (static_cast<int>(in[offset + 1]) << 8);
            offset += n + 2;
        }
        if (flags & (1 << 3)) { // FNAME
            while (offset < in.size()) {
                if (in[offset++] == 0) {
                    break;
                }
            }
        }
        if (flags & (1 << 4)) { // FCOMMENT
            while (offset < in.size()) {
                if (in[offset++] == 0) {
                    break;
                }
            }
        }
        if (flags & (1 << 1)) { // FCRC
            offset += 2;
        }

        unsigned char buf[4096];
        ::mz_stream infstream;
        std::memset(&infstream, 0, sizeof(infstream));
        infstream.zalloc = NULL;
        infstream.zfree = NULL;
        infstream.opaque = NULL;
        int err = MZ_OK;
        infstream.avail_in = static_cast<unsigned int>(in.size() - offset - 4); // size of input
        infstream.next_in = reinterpret_cast<const unsigned char *>(&in[offset]); // input char array
        infstream.avail_out = sizeof(buf); // size of output
        infstream.next_out = buf; // output char array
        ::mz_inflateInit2(&infstream, -MZ_DEFAULT_WINDOW_BITS);
        do {
            infstream.avail_out = sizeof(buf); // size of output
            infstream.next_out = buf; // output char array
            err = ::mz_inflate(&infstream, infstream.avail_in > 0 ? MZ_NO_FLUSH : MZ_FINISH);
            if (err != MZ_OK && err != MZ_STREAM_END) {
                break;
            }
            out.insert(out.end(), buf, buf + sizeof(buf) - infstream.avail_out);
        } while (err != MZ_STREAM_END);
        ::mz_inflateEnd(&infstream);
        return err == MZ_OK || err == MZ_STREAM_END;
    }
} }
