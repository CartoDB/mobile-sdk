#include "CartoCSSParser.h"
#include "PointStyleBuilder.h"
#include "LineStyleBuilder.h"
#include "PolygonStyleBuilder.h"
#include "MarkerStyleBuilder.h"
#include "graphics/Bitmap.h"
#include "utils/AssetUtils.h"
#include "utils/Log.h"

#include <regex>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/repository/include/qi_distinct.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

namespace carto {

    namespace cssparserimpl {
        namespace phx = boost::phoenix;
        namespace qi = boost::spirit::qi;
        namespace encoding = boost::spirit::iso8859_1;

        typedef std::pair<std::string, std::string> KeyValuePair;

        template <typename Iterator>
        struct Skipper : public qi::grammar<Iterator> {
            Skipper() : Skipper::base_type(skip_, "PL/0") {
                skip_ = qi::iso8859_1::space | ("/*" >> *(qi::char_ - "*/") >> "*/");
            }
            qi::rule<Iterator> skip_;
        };

        template <typename Iterator>
        struct Grammar : qi::grammar<Iterator, std::vector<KeyValuePair>(), Skipper<Iterator> > {
            Grammar() : Grammar::base_type(keyvalues) {
                using qi::_val;
                using qi::_1;
                using qi::_2;
                using qi::_3;

                unesc_char.add("\\a", '\a')("\\b", '\b')("\\f", '\f')("\\n", '\n')
                    ("\\r", '\r')("\\t", '\t')("\\v", '\v')("\\\\", '\\')
                    ("\\\'", '\'')("\\\"", '\"');

                nonascii_ = qi::char_("\xA0-\xFF");
                nmstart_ = qi::char_("_a-zA-Z") | nonascii_;
                nmchar_ = qi::char_("_a-zA-Z0-9-") | nonascii_;
                unescaped_ = qi::char_("_a-zA-Z0-9-#().,%");

                string_ = '\'' >> *(unesc_char | "\\x" >> qi::hex | (qi::print - '\'')) >> '\''
                        | '\"' >> *(unesc_char | "\\x" >> qi::hex | (qi::print - '\"')) >> '\"';                
                url_ = qi::lit("url") >> "(" >> string_ >> ")";

                key = nmstart_ > *nmchar_;
                value = url_ | string_ | *unescaped_;

                keyvalue = (key >> ":" >> value) [_val = phx::construct<KeyValuePair>(_1, _2)];
                keyvalues = -keyvalue % ';';
            }

            qi::symbols<char const, char const> unesc_char;
            qi::rule<Iterator, char()> nonascii_, nmstart_, nmchar_, unescaped_;
            qi::rule<Iterator, std::string()> string_, url_;
            qi::rule<Iterator, std::string()> key, value;
            qi::rule<Iterator, KeyValuePair(), Skipper<Iterator> > keyvalue;
            qi::rule<Iterator, std::vector<KeyValuePair>(), Skipper<Iterator> > keyvalues;
        };
    }

    CartoCSSParser::CartoCSSKeyValues::CartoCSSKeyValues(const std::string& method, const std::map<std::string, std::string>& keyValueMap) :
        _method(method),
        _keyValueMap(keyValueMap),
        _usedKeys()
    {
    }

    CartoCSSParser::CartoCSSKeyValues::~CartoCSSKeyValues() {
        for (auto it = _keyValueMap.begin(); it != _keyValueMap.end(); it++) {
            if (_usedKeys.find(it->first) == _usedKeys.end()) {
                Log::Warnf("CartoCSSParser::%s: Unused parameter: %s", _method.c_str(), it->first.c_str());
            }
        }
    }

    std::string CartoCSSParser::CartoCSSKeyValues::getString(const std::string& key, const std::string& defaultValue) const {
        auto it = _keyValueMap.find(key);
        if (it == _keyValueMap.end()) {
            return defaultValue;
        }
        _usedKeys.insert(key);
        return it->second;
    }
    
    std::string CartoCSSParser::CartoCSSKeyValues::getFileName(const std::string& key, const std::string& defaultValue) const {
        auto it = _keyValueMap.find(key);
        if (it == _keyValueMap.end()) {
            return defaultValue;
        }
        _usedKeys.insert(key);

        const std::string& value = it->second;
        if (value.substr(0, 9) == "//assets/") {
            return value.substr(9);
        }
        Log::Errorf("CartoCSSParser::%s: Unsupported URL scheme: %s; should begin with '//assets/'", _method.c_str(), value.c_str());
        return defaultValue;
    }

    Color CartoCSSParser::CartoCSSKeyValues::getColor(const std::string& key, const Color& defaultValue) const {
        auto it = _keyValueMap.find(key);
        if (it == _keyValueMap.end()) {
            return defaultValue;
        }
        _usedKeys.insert(key);

        const std::string& value = it->second;
        if (value.substr(0, 1) == "#") {
            unsigned int val = 0;
            std::stringstream(value.substr(1)) >> std::hex >> val;
            if (value.length() == 4) {
                val = ((((val >> 8) & 15) * 17) << 16) + ((((val >> 4) & 15) * 17) << 8) + ((((val >> 0) & 15) * 17) << 0);
            }
            return val | 0xff000000;
        }

        static const std::regex RGBRe("\\s*rgb\\s*\\(\\s*(\\S+)\\s*,\\s*(\\S+)\\s*,\\s*(\\S+)\\s*\\)\\s*", std::regex_constants::icase);
        static const std::regex RGBARe("\\s*rgba\\s*\\(\\s*(\\S+)\\s*,\\s*(\\S+)\\s*,\\s*(\\S+)\\s*,\\s*(\\S+)\\s*\\)\\s*", std::regex_constants::icase);
        
        std::smatch sm;
        if (std::regex_match(value, sm, RGBRe) || std::regex_match(value, sm, RGBARe)) {
            unsigned int components[4] = { 0, 0, 0, 0 };
            for (std::size_t i = 1; i <= 4; i++) {
                unsigned int val = 255;
                if (i < sm.size()) {
                    try {
                        if (sm[i].str().find('.') != std::string::npos) {
                            val = static_cast<unsigned int>(boost::lexical_cast<float>(sm[i].str()) * 255);
                        } else {
                            val = boost::lexical_cast<unsigned int>(sm[i].str());
                        }
                    } catch (const boost::bad_lexical_cast&) {
                        Log::Errorf("CartoCSSParser::%s: Unsupported color component value: %s", _method.c_str(), sm[i].str().c_str());
                        val = 0;
                    }
                }
                components[i - 1] = val;
            }
            return Color(components[0], components[1], components[2], components[3]);
        }

        Log::Errorf("CartoCSSParser::%s: Unsupported color value: %s", _method.c_str(), value.c_str());
        return defaultValue;
    }
    
    Color CartoCSSParser::CartoCSSKeyValues::getColor(const std::string& key, const Color& defaultValue, float opacity) const {
        Color color = getColor(key, defaultValue);
        return Color(color.getR(), color.getG(), color.getB(), static_cast<unsigned char>(color.getA() * opacity));
    }

    bool CartoCSSParser::CartoCSSKeyValues::getBool(const std::string& key, bool defaultValue) const {
        auto it = _keyValueMap.find(key);
        if (it == _keyValueMap.end()) {
            return defaultValue;
        }
        _usedKeys.insert(key);

        const std::string& value = it->second;
        try {
            if (value == "true") {
                return true;
            } else if (value == "false") {
                return false;
            }
            return boost::lexical_cast<bool>(value);
        } catch (const std::exception& ex) {
            Log::Errorf("CartoCSSParser::%s: Exception while reading boolean value: %s", _method.c_str(), ex.what());
            return defaultValue;
        }
    }

    float CartoCSSParser::CartoCSSKeyValues::getFloat(const std::string& key, float defaultValue) const {
        auto it = _keyValueMap.find(key);
        if (it == _keyValueMap.end()) {
            return defaultValue;
        }
        _usedKeys.insert(key);

        const std::string& value = it->second;
        try {
            if (!value.empty() && value[value.size() - 1] == '%') {
                return boost::lexical_cast<float>(value.substr(0, value.size() - 1)) / 100.0f;
            }
            return boost::lexical_cast<float>(value);
        } catch (const std::exception& ex) {
            Log::Errorf("CartoCSSParser::%s: Exception while reading float value: %s", _method.c_str(), ex.what());
            return defaultValue;
        }
    }

    std::shared_ptr<PointStyleBuilder> CartoCSSParser::CreatePointStyleBuilder(const std::string& cartoCSS) {
        return CreatePointStyleBuilder(CartoCSSKeyValues("CreatePointStyleBuilder", ParseCSSKeyValues(cartoCSS)));
    }

    std::shared_ptr<LineStyleBuilder> CartoCSSParser::CreateLineStyleBuilder(const std::string& cartoCSS) {
        return CreateLineStyleBuilder(CartoCSSKeyValues("CreateLineStyleBuilder", ParseCSSKeyValues(cartoCSS)));
    }

    std::shared_ptr<PolygonStyleBuilder> CartoCSSParser::CreatePolygonStyleBuilder(const std::string& cartoCSS) {
        CartoCSSKeyValues keyValues("CreatePolygonStyleBuilder", ParseCSSKeyValues(cartoCSS));
        std::shared_ptr<PolygonStyleBuilder> styleBuilder = CreatePolygonStyleBuilder(keyValues);
        if (keyValues.getFloat("line-width", 0) > 0) {
            styleBuilder->setLineStyle(CreateLineStyleBuilder(keyValues)->buildStyle());
        }
        return styleBuilder;
    }

    std::shared_ptr<MarkerStyleBuilder> CartoCSSParser::CreateMarkerStyleBuilder(const std::string& cartoCSS) {
        return CreateMarkerStyleBuilder(CartoCSSKeyValues("CreateMarkerStyleBuilder", ParseCSSKeyValues(cartoCSS)));
    }

    std::map<std::string, std::string> CartoCSSParser::ParseCSSKeyValues(const std::string& cartoCSS) {
        std::string::const_iterator it = cartoCSS.begin();
        std::string::const_iterator end = cartoCSS.end();
        cssparserimpl::Grammar<std::string::const_iterator> grammar;
        cssparserimpl::Skipper<std::string::const_iterator> skipper;
        std::vector<cssparserimpl::KeyValuePair> keyValues;
        bool result = boost::spirit::qi::phrase_parse(it, end, grammar, skipper, keyValues);
        if (!result) {
            Log::Error("CartoCSSParser: Failed to parse CartoCSS.");
        } else if (it != cartoCSS.end()) {
            Log::Error("CartoCSSParser: Could not parse to the end of CartoCSS.");
        }
        std::map<std::string, std::string> keyValueMap;
        std::for_each(keyValues.begin(), keyValues.end(), [&keyValueMap](const cssparserimpl::KeyValuePair& keyValue) { keyValueMap.insert(keyValue); });
        return keyValueMap;
    }

    std::shared_ptr<PointStyleBuilder> CartoCSSParser::CreatePointStyleBuilder(const CartoCSSKeyValues& keyValues) {
        auto styleBuilder = std::make_shared<PointStyleBuilder>();
        styleBuilder->setSize(keyValues.getFloat("point-width", keyValues.getFloat("point-size", 10)));
        styleBuilder->setColor(keyValues.getColor("point-fill", Color(255, 255, 255, 255), keyValues.getFloat("point-opacity", 1)));

        std::string pointFile = keyValues.getFileName("point-file", "");
        if (!pointFile.empty()) {
            std::shared_ptr<BinaryData> bitmapData = AssetUtils::LoadAsset(pointFile);
            if (bitmapData) {
                std::shared_ptr<Bitmap> bitmap = Bitmap::CreateFromCompressed(bitmapData);
                styleBuilder->setBitmap(bitmap);
            }
        }
        return styleBuilder;
    }

    std::shared_ptr<LineStyleBuilder> CartoCSSParser::CreateLineStyleBuilder(const CartoCSSKeyValues& keyValues) {
        auto styleBuilder = std::make_shared<LineStyleBuilder>();
        styleBuilder->setWidth(keyValues.getFloat("line-width", 1));
        styleBuilder->setColor(keyValues.getColor("line-fill", Color(0, 0, 0, 255), keyValues.getFloat("line-opacity", 1)));

        std::string lineJoin = keyValues.getString("line-join", "miter");
        if (lineJoin == "miter") {
            styleBuilder->setLineJointType(LineJointType::LINE_JOINT_TYPE_STRAIGHT); // TODO: wrong, miter currently not supported
        } else if (lineJoin == "round") {
            styleBuilder->setLineJointType(LineJointType::LINE_JOINT_TYPE_ROUND);
        } else if (lineJoin == "bevel") {
            styleBuilder->setLineJointType(LineJointType::LINE_JOINT_TYPE_STRAIGHT);
        } else {
            Log::Errorf("CartoCSSParser::CreateLineStyle: Unsupported line join mode: %s", lineJoin.c_str());
        }

        std::string lineCap = keyValues.getString("line-cap", "butt");
        if (lineCap == "butt") {
            styleBuilder->setLineEndType(LineEndType::LINE_END_TYPE_STRAIGHT);
        } else if (lineCap == "round") {
            styleBuilder->setLineEndType(LineEndType::LINE_END_TYPE_ROUND);
        } else if (lineCap == "square") {
            styleBuilder->setLineEndType(LineEndType::LINE_END_TYPE_STRAIGHT); // TODO: wrong, square currently not supported
        } else {
            Log::Errorf("CartoCSSParser::CreateLineStyle: Unsupported line cap mode: %s", lineCap.c_str());
        }

        std::string lineDashArray = keyValues.getString("line-dasharray", "");
        if (!lineDashArray.empty()) {
            std::vector<std::string> dashList;
            boost::split(dashList, lineDashArray, boost::is_any_of(","));
            std::vector<float> strokeDashArray;
            for (const std::string& dash : dashList) {
                try {
                    strokeDashArray.push_back(boost::lexical_cast<float>(boost::trim_copy(dash)));
                } catch (const boost::bad_lexical_cast& ex) {
                    Log::Errorf("CartoCSSParser::CreateLineStyle: Illegal dash value: %s", ex.what());
                }
            }
            if (strokeDashArray.empty()) {
                strokeDashArray.push_back(1);
            }
            float size = 0;
            int superSample = 1;
            for (float dash : strokeDashArray) {
                size += dash;
                int factor = 1;
                while (factor * superSample < 16) { // increase resolution when fractional dashes are used
                    float dashFract = dash * superSample * factor;
                    dashFract -= std::floor(dashFract);
                    if (dashFract < 0.1f || dashFract > 0.9f) {
                        break;
                    }
                    factor++;
                }
                superSample *= factor;
            }
            int pow2Size = 1;
            while (pow2Size < size * superSample) {
                pow2Size *= 2;
            }
            if (pow2Size < 16) {
                pow2Size = 16;
            }
            std::vector<uint32_t> data(pow2Size);
            float pos = 0;
            for (size_t n = 0; n < strokeDashArray.size(); n++) {
                float dash = strokeDashArray[n];
                int x0 = static_cast<int>(pos * pow2Size / size);
                int x1 = static_cast<int>((pos + dash) * pow2Size / size);
                for (int x = x0; x < x1; x++) {
                    data[x] = n % 2 == 0 ? 0xffffffff : 0;
                }
                pos += dash;
            }

            auto bitmap = std::make_shared<Bitmap>(reinterpret_cast<const unsigned char*>(data.data()), 1, pow2Size, ColorFormat::COLOR_FORMAT_RGBA, 4);
            styleBuilder->setBitmap(bitmap);
            styleBuilder->setStretchFactor(size / pow2Size);
        }
        return styleBuilder;
    }

    std::shared_ptr<PolygonStyleBuilder> CartoCSSParser::CreatePolygonStyleBuilder(const CartoCSSKeyValues& keyValues) {
        auto styleBuilder = std::make_shared<PolygonStyleBuilder>();
        styleBuilder->setColor(keyValues.getColor("polygon-fill", Color(128, 128, 128, 255), keyValues.getFloat("polygon-opacity", 1)));
        return styleBuilder;
    }

    std::shared_ptr<MarkerStyleBuilder> CartoCSSParser::CreateMarkerStyleBuilder(const CartoCSSKeyValues& keyValues) {
        auto styleBuilder = std::make_shared<MarkerStyleBuilder>();
        styleBuilder->setSize(keyValues.getFloat("marker-width", keyValues.getFloat("marker-size", 10)));
        styleBuilder->setColor(keyValues.getColor("marker-fill", Color(255, 255, 255, 255), keyValues.getFloat("marker-opacity", 1)));

        std::string markerFile = keyValues.getFileName("marker-file", "");
        if (!markerFile.empty()) {
            std::shared_ptr<BinaryData> bitmapData = AssetUtils::LoadAsset(markerFile);
            if (bitmapData) {
                std::shared_ptr<Bitmap> bitmap = Bitmap::CreateFromCompressed(bitmapData);
                styleBuilder->setBitmap(bitmap);
            }
        }

        bool allowOverlap = keyValues.getBool("allow-overlap", false);
        styleBuilder->setCausesOverlap(true);
        styleBuilder->setHideIfOverlapped(allowOverlap);
        return styleBuilder;
    }

}
