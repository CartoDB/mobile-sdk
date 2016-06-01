#include "VectorElementDrawData.h"

namespace carto {

    VectorElementDrawData::~VectorElementDrawData() {
    }
    
    const Color& VectorElementDrawData::getColor() const {
        return _color;
    }
    
    bool VectorElementDrawData::isOffset() const {
        return _isOffset;
    }
    
    void VectorElementDrawData::setIsOffset(bool isOffset) {
        _isOffset = isOffset;
    }
    
    VectorElementDrawData::VectorElementDrawData(const Color& color) :
        _color(static_cast<unsigned char>(static_cast<unsigned int>(color.getR()) * color.getA() / 255),
               static_cast<unsigned char>(static_cast<unsigned int>(color.getG()) * color.getA() / 255),
               static_cast<unsigned char>(static_cast<unsigned int>(color.getB()) * color.getA() / 255),
               color.getA()),
        _isOffset(false)
    {
    }
    
}
