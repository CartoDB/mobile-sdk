#include "VariantObjectBuilder.h"

namespace carto {

    VariantObjectBuilder::VariantObjectBuilder() :
        _elementMap(),
        _mutex()
    {
    }

    void VariantObjectBuilder::clear() {
        std::lock_guard<std::mutex> lock(_mutex);
        _elementMap.clear();
    }

    void VariantObjectBuilder::setString(const std::string& key, const std::string& str) {
        std::lock_guard<std::mutex> lock(_mutex);
        _elementMap[key] = Variant(str);
    }

    void VariantObjectBuilder::setBool(const std::string& key, bool val) {
        std::lock_guard<std::mutex> lock(_mutex);
        _elementMap[key] = Variant(val);
    }

    void VariantObjectBuilder::setLong(const std::string& key, long long val) {
        std::lock_guard<std::mutex> lock(_mutex);
        _elementMap[key] = Variant(val);
    }

    void VariantObjectBuilder::setDouble(const std::string& key, double val) {
        std::lock_guard<std::mutex> lock(_mutex);
        _elementMap[key] = Variant(val);
    }

    void VariantObjectBuilder::setVariant(const std::string& key, const Variant& var) {
        std::lock_guard<std::mutex> lock(_mutex);
        _elementMap[key] = var;
    }

    Variant VariantObjectBuilder::buildVariant() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return Variant(_elementMap);
    }

}
