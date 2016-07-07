#include "Label.h"
#include "styles/LabelStyle.h"
#include "renderers/drawdatas/LabelDrawData.h"

namespace carto {

    Label::Label(const std::shared_ptr<Billboard>& baseBillboard, const std::shared_ptr<LabelStyle>& style) :
        Billboard(baseBillboard),
        _style(style)
    {
        if (!style) {
            throw std::invalid_argument("Null style");
        }
    }
    
    Label::Label(const std::shared_ptr<Geometry>& geometry, const std::shared_ptr<LabelStyle>& style) :
        Billboard(geometry),
        _style(style)
    {
        if (!style) {
            throw std::invalid_argument("Null style");
        }
    }
    
    Label::Label(const MapPos& pos, const std::shared_ptr<LabelStyle>& style) :
        Billboard(pos),
        _style(style)
    {
        if (!style) {
            throw std::invalid_argument("Null style");
        }
    }
        
    Label::~Label() {
    }
    
    std::shared_ptr<LabelStyle> Label::getStyle() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _style;
    }
    
    void Label::setStyle(const std::shared_ptr<LabelStyle>& style) {
        if (!style) {
            throw std::invalid_argument("Null style");
        }

        {
            std::lock_guard<std::mutex> lock(_mutex);
            _style = style;
        }
        notifyElementChanged();
    }
    
}
