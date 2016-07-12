#include "Marker.h"
#include "components/Exceptions.h"
#include "styles/MarkerStyle.h"
#include "renderers/drawdatas/MarkerDrawData.h"

namespace carto {
    
    Marker::Marker(const std::shared_ptr<Billboard>& baseBillboard,
                   const std::shared_ptr<MarkerStyle>& style) :
        Billboard(baseBillboard),
        _style(style)
    {
        if (!style) {
            throw NullArgumentException("Null style");
        }
    }
    
    Marker::Marker(const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<MarkerStyle>& style) :
        Billboard(geometry),
        _style(style)
    {
        if (!style) {
            throw NullArgumentException("Null style");
        }
    }
        
    Marker::Marker(const MapPos& pos, const std::shared_ptr<MarkerStyle>& style) :
        Billboard(pos),
        _style(style)
    {
        if (!style) {
            throw NullArgumentException("Null style");
        }
    }
    
    Marker::~Marker() {
    }
    
    std::shared_ptr<MarkerStyle> Marker::getStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _style;
    }
    
    void Marker::setStyle(const std::shared_ptr<MarkerStyle>& style) {
        if (!style) {
            throw NullArgumentException("Null style");
        }

        {
            std::lock_guard<std::mutex> lock(_mutex);
            _style = style;
        }
        notifyElementChanged();
    }
    
}
