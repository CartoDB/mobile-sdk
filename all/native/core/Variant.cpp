#include "Variant.h"
#include "utils/Log.h"

namespace carto {

    Variant::Variant() :
        _value()
    {
    }

    Variant::Variant(bool boolVal) :
        _value(boolVal)
    {
    }

    Variant::Variant(long long longVal) :
        _value(static_cast<std::int64_t>(longVal))
    {
    }

    Variant::Variant(double doubleVal) :
        _value(doubleVal)
    {
    }

    Variant::Variant(const char* str) :
        _value(std::string(str))
    {
    }

    Variant::Variant(const std::string& string) :
        _value(string)
    {
    }

    Variant::Variant(const std::vector<Variant>& array) :
        _value()
    {
        picojson::value::array valArr;
        for (auto it = array.begin(); it != array.end(); it++) {
            valArr.push_back(it->toPicoJSON());
        }
        _value = picojson::value(valArr);
    }

    Variant::Variant(const std::map<std::string, Variant>& object) :
        _value()
    {
        picojson::value::object valObj;
        for (auto it = object.begin(); it != object.end(); it++) {
            valObj[it->first] = it->second.toPicoJSON();
        }
        _value = picojson::value(valObj);
    }

    bool Variant::operator == (const Variant& var) const {
        return toPicoJSON() == var.toPicoJSON();
    }

    bool Variant::operator != (const Variant& var) const {
        return !(*this == var);
    }

    VariantType::VariantType Variant::getType() const {
        const picojson::value& val = toPicoJSON();
        if (val.is<std::string>()) {
            return VariantType::VARIANT_TYPE_STRING;
        }
        if (val.is<bool>()) {
            return VariantType::VARIANT_TYPE_BOOL;
        }
        if (val.is<std::int64_t>()) {
            return VariantType::VARIANT_TYPE_INTEGER;
        }
        if (val.is<double>()) {
            return VariantType::VARIANT_TYPE_DOUBLE;
        }
        if (val.is<picojson::value::array>()) {
            return VariantType::VARIANT_TYPE_ARRAY;
        }
        if (val.is<picojson::value::object>()) {
            return VariantType::VARIANT_TYPE_OBJECT;
        }
        return VariantType::VARIANT_TYPE_NULL;
    }

    std::string Variant::getString() const {
        return toPicoJSON().to_str();
    }

    bool Variant::getBool() const {
        const picojson::value& val = toPicoJSON();
        if (val.is<bool>()) {
            return val.get<bool>();
        }
        return false;
    }

    long long Variant::getLong() const {
        const picojson::value& val = toPicoJSON();
        if (val.is<std::int64_t>()) {
            return val.get<std::int64_t>();
        }
        return 0;
    }

    double Variant::getDouble() const {
        const picojson::value& val = toPicoJSON();
        if (val.is<double>()) {
            return val.get<double>();
        }
        return 0.0;
    }

    int Variant::getArraySize() const {
        const picojson::value& val = toPicoJSON();
        if (val.is<picojson::value::array>()) {
            return static_cast<int>(val.get<picojson::value::array>().size());
        }
        return 0;
    }

    Variant Variant::getArrayElement(int idx) const {
        const picojson::value& val = toPicoJSON();
        if (val.is<picojson::value::array>()) {
            const picojson::array& valArr = val.get<picojson::value::array>();
            if (idx >= 0 && idx < static_cast<int>(valArr.size())) {
                return FromPicoJSON(valArr[idx]);
            }
        }
        return Variant();
    }
    
    std::vector<std::string> Variant::getObjectKeys() const {
        const picojson::value& val = toPicoJSON();
        std::vector<std::string> keys;
        if (val.is<picojson::value::object>()) {
            const picojson::object& valObj = val.get<picojson::value::object>();
            for (auto it = valObj.begin(); it != valObj.end(); it++) {
                keys.push_back(it->first);
            }
        }
        return keys;
    }
    
    Variant Variant::getObjectElement(const std::string& key) const {
        const picojson::value& val = toPicoJSON();
        std::vector<std::string> keys;
        if (val.is<picojson::value::object>()) {
            const picojson::object& valObj = val.get<picojson::value::object>();
            auto it = valObj.find(key);
            if (it != valObj.end()) {
                return FromPicoJSON(it->second);
            }
        }
        return Variant();
    }

    int Variant::hash() const {
        return static_cast<int>(std::hash<std::string>()(toString()));
    }

    std::string Variant::toString() const {
        return toPicoJSON().serialize();
    }

    const picojson::value& Variant::toPicoJSON() const {
        return _value;
    }

    Variant Variant::FromString(const std::string& str) {
        picojson::value val;
        std::string err = picojson::parse(val, str);
        if (!err.empty()) {
            Log::Errorf("Variant::FromString: Failed to read JSON value: %s", str.c_str());
        }
        return FromPicoJSON(val);
    }

    Variant Variant::FromPicoJSON(const picojson::value& val) {
        Variant var;
        var._value = val;
        return var;
    }

}
