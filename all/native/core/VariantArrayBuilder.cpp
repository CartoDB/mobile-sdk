#include "VariantArrayBuilder.h"

namespace carto {

    VariantArrayBuilder::VariantArrayBuilder() :
        _elements(),
        _mutex()
    {
    }

    void VariantArrayBuilder::addString(const std::string& str) {
        std::lock_guard<std::mutex> lock(_mutex);
        _elements.emplace_back(str);
    }

    void VariantArrayBuilder::addBool(bool val) {
        std::lock_guard<std::mutex> lock(_mutex);
        _elements.emplace_back(val);
    }

    void VariantArrayBuilder::addLong(long long val) {
        std::lock_guard<std::mutex> lock(_mutex);
        _elements.emplace_back(val);
    }

    void VariantArrayBuilder::addDouble(double val) {
        std::lock_guard<std::mutex> lock(_mutex);
        _elements.emplace_back(val);
    }

    void VariantArrayBuilder::addVariant(const Variant& var) {
        std::lock_guard<std::mutex> lock(_mutex);
        _elements.emplace_back(var);
    }

    Variant VariantArrayBuilder::buildVariant() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return Variant(_elements);
    }

}
