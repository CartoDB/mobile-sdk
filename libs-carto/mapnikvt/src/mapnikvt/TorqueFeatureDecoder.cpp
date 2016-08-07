#include "TorqueFeatureDecoder.h"
#include "Logger.h"

#include <boost/lexical_cast.hpp>

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

namespace {
    int readRapidJSONInt(const rapidjson::Value& value) {
        if (value.IsInt()) {
            return value.GetInt();
        } else if (value.IsUint()) {
            return value.GetUint();
        } else if (value.IsInt64()) {
            return static_cast<int>(value.GetInt64());
        } else if (value.IsUint64()) {
            return static_cast<int>(value.GetUint64());
        } else if (value.IsNumber()) {
            return static_cast<int>(value.GetDouble());
        }
        return 0;
    }

    int readRapidJSONDouble(const rapidjson::Value& value) {
        if (value.IsInt()) {
            return value.GetInt();
        } else if (value.IsUint()) {
            return value.GetUint();
        } else if (value.IsInt64()) {
            return value.GetInt64();
        } else if (value.IsUint64()) {
            return value.GetUint64();
        } else if (value.IsNumber()) {
            return value.GetDouble();
        }
        return 0.0;
    }
}

namespace carto { namespace mvt {
    class TorqueFeatureDecoder::TorqueFeatureIterator : public carto::mvt::FeatureDecoder::FeatureIterator {
    public:
        explicit TorqueFeatureIterator(const std::vector<TorqueFeatureDecoder::Element>& elements, int resolution, const cglib::mat3x3<float>& transform, const cglib::bbox2<float>& clipBox) : _elements(elements), _resolution(resolution), _transform(transform), _clipBox(clipBox) {
            while (++_index1 < _elements.size()) {
                if (_elements[_index0].value != _elements[_index1].value) {
                    break;
                }
            }
        }

        virtual bool valid() const override {
            return _index0 < _elements.size();
        }

        virtual void advance() override {
            _index0 = _index1;
            while (++_index1 < _elements.size()) {
                if (_elements[_index0].value != _elements[_index1].value) {
                    break;
                }
            }
        }

        virtual long long getFeatureId() const override {
            return 0;
        }

        virtual std::shared_ptr<const FeatureData> getFeatureData() const override {
            const TorqueFeatureDecoder::Element& element = _elements[_index0];
            auto it = _featureDataCache.find(element.value);
            if (it != _featureDataCache.end()) {
                return it->second;
            }

            std::vector<std::pair<std::string, Value>> dataMap(1);
            dataMap[0] = std::make_pair(std::string("value"), Value(element.value));
            auto featureData = std::make_shared<FeatureData>(FeatureData::GeometryType::POINT_GEOMETRY, std::move(dataMap));
            _featureDataCache.emplace(element.value, featureData);
            return featureData;
        }

        virtual std::shared_ptr<const Geometry> getGeometry() const override {
            float scale = 1.0f / _resolution;
            std::vector<cglib::vec2<float>> vertices;
            for (std::size_t i = _index0; i < _index1; i++) {
                const TorqueFeatureDecoder::Element& element = _elements[i];
                cglib::vec2<float> p = cglib::transform_point(cglib::vec2<float>(element.x * scale, 1.0f - element.y * scale), _transform);
                if (_clipBox.inside(p)) {
                    vertices.push_back(p);
                }
            }
            return std::make_shared<PointGeometry>(std::move(vertices));
        }

    private:
        std::size_t _index0 = 0;
        std::size_t _index1 = 0;
        const std::vector<TorqueFeatureDecoder::Element>& _elements;
        const int _resolution;
        const cglib::mat3x3<float> _transform;
        const cglib::bbox2<float> _clipBox;
        mutable std::unordered_map<double, std::shared_ptr<FeatureData>> _featureDataCache;
    };

    TorqueFeatureDecoder::TorqueFeatureDecoder(const std::vector<unsigned char>& data, int resolution, const std::shared_ptr<Logger>& logger) :
        _resolution(resolution), _transform(cglib::mat3x3<float>::identity()), _clipBox(cglib::vec2<float>(-0.1f, -0.1f), cglib::vec2<float>(1.1f, 1.1f)), _logger(logger)
    {
        rapidjson::Document torqueDoc;
        std::string torqueJson(reinterpret_cast<const char*>(data.data()), reinterpret_cast<const char*>(data.data() + data.size()));
        if (torqueDoc.Parse<rapidjson::kParseDefaultFlags>(torqueJson.c_str()).HasParseError()) {
            std::string msg = "Error while parsing Torque JSON, error at position " + boost::lexical_cast<std::string>(torqueDoc.GetErrorOffset() + 1);
            _logger->write(Logger::Severity::ERROR, msg);
            return;
        }

        std::string xField;
        std::string yField;
        std::string valueField;
        std::string timeField;
        const rapidjson::Value* rows = nullptr;
        if (torqueDoc.IsArray()) {
            if (torqueDoc.Begin() == torqueDoc.End()) {
                _logger->write(Logger::Severity::INFO, "Empty Torque JSON");
                return;
            }
            rapidjson::Value::MemberIterator mit = (*torqueDoc.Begin()).MemberBegin();
            xField = (mit++)->name.GetString();
            yField = (mit++)->name.GetString();
            valueField = (mit++)->name.GetString();
            timeField = (mit++)->name.GetString();
            rows = &torqueDoc;
        }
        else if (torqueDoc.IsObject()) {
            if (!torqueDoc["fields"].IsObject() || torqueDoc["fields"].MemberCount() < 4) {
                _logger->write(Logger::Severity::ERROR, "Torque JSON 'fields' missing");
                return;
            }
            rapidjson::Value::MemberIterator mit = torqueDoc["fields"].MemberBegin();
            xField = (mit++)->name.GetString();
            yField = (mit++)->name.GetString();
            valueField = (mit++)->name.GetString();
            timeField = (mit++)->name.GetString();
            rows = &torqueDoc["rows"];
        } else {
            _logger->write(Logger::Severity::ERROR, "Unexpected Torque JSON type");
            return;
        }

        for (rapidjson::Value::ConstValueIterator jit = rows->Begin(); jit != rows->End(); jit++) {
            const rapidjson::Value& rowValue = *jit;
            int x = readRapidJSONInt(rowValue[xField.c_str()]);
            int y = readRapidJSONInt(rowValue[yField.c_str()]);
            unsigned int valueCount = rowValue[valueField.c_str()].Size();
            unsigned int timeCount = rowValue[timeField.c_str()].Size();
            if (valueCount != timeCount) {
                _logger->write(Logger::Severity::ERROR, "Value/time array mismatch");
            }
            for (unsigned int i = 0; i < valueCount; i++) {
                int time = (i < timeCount ? readRapidJSONInt(rowValue[timeField.c_str()][i]) : -1);
                double value = readRapidJSONDouble(rowValue[valueField.c_str()][i]);
                _timeValueMap[time].emplace_back(x, y, value);
            }
        }
    }

    void TorqueFeatureDecoder::setTransform(const cglib::mat3x3<float>& transform) {
        _transform = transform;
    }

    void TorqueFeatureDecoder::setClipBox(const cglib::bbox2<float>& clipBox) {
        _clipBox = clipBox;
    }

    std::shared_ptr<FeatureDecoder::FeatureIterator> TorqueFeatureDecoder::createFrameFeatureIterator(int frame) const {
        auto it = _timeValueMap.find(frame);
        if (it == _timeValueMap.end()) {
            return std::shared_ptr<FeatureIterator>();
        }

        return std::make_shared<TorqueFeatureIterator>(it->second, _resolution, _transform, _clipBox);
    }
} }
