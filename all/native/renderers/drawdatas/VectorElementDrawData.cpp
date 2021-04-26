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

    Color VectorElementDrawData::GetPremultipliedColor(const Color& color) {
        unsigned int alpha = color.getA();
        if (alpha == 255) {
            return color;
        }
        return Color(static_cast<unsigned char>(color.getR() * alpha / 255),
                     static_cast<unsigned char>(color.getG() * alpha / 255),
                     static_cast<unsigned char>(color.getB() * alpha / 255),
                     color.getA());
    }
    
    VectorElementDrawData::VectorElementDrawData(const Color& color, const std::shared_ptr<ProjectionSurface>& projectionSurface) :
        _projectionSurface(projectionSurface),
        _color(GetPremultipliedColor(color)),
        _isOffset(false)
    {
    }

    void VectorElementDrawData::setIsOffset(bool isOffset) {
        _isOffset = isOffset;
    }
    
    const std::shared_ptr<ProjectionSurface>& VectorElementDrawData::getProjectionSurface() const {
        return _projectionSurface;
    }
}
