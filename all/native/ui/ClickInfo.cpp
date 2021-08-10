#include "ClickInfo.h"

#include <iomanip>
#include <sstream>

namespace carto {

    ClickInfo::ClickInfo(ClickType::ClickType clickType, float duration) :
        _clickType(clickType),
        _duration(duration)
    {
    }
    
    ClickType::ClickType ClickInfo::getClickType() const {
        return _clickType;
    }
    
    float ClickInfo::getDuration() const {
        return _duration;
    }
    
    int ClickInfo::hash() const {
        return static_cast<int>(std::hash<int>()(static_cast<int>(_clickType)) ^ std::hash<float>()(_duration));
    }

    bool ClickInfo::operator ==(const ClickInfo& clickInfo) const {
        return _clickType == clickInfo._clickType && _duration == clickInfo._duration;
    }

    bool ClickInfo::operator !=(const ClickInfo& clickInfo) const {
        return !(*this == clickInfo);
    }
    
    std::string ClickInfo::toString() const {
        std::string clickType;
        switch (_clickType) {
        case ClickType::CLICK_TYPE_SINGLE:
            clickType = "CLICK_TYPE_SINGLE";
            break;
        case ClickType::CLICK_TYPE_LONG:
            clickType = "CLICK_TYPE_LONG";
            break;
        case ClickType::CLICK_TYPE_DOUBLE:
            clickType = "CLICK_TYPE_DOUBLE";
            break;
        case ClickType::CLICK_TYPE_DUAL:
            clickType = "CLICK_TYPE_DUAL";
            break;
        }

        std::stringstream ss;
        ss << std::setiosflags(std::ios::fixed);
        ss << "ClickInfo [clickType=" << clickType << ", duration=" << _duration << "]";
        return ss.str();
    }

}
