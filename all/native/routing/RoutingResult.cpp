#ifdef _CARTO_ROUTING_SUPPORT

#include "RoutingResult.h"
#include "components/Exceptions.h"

#include <numeric>
#include <functional>
#include <utility>
#include <iomanip>
#include <sstream>

namespace carto {

    RoutingResult::RoutingResult(const std::shared_ptr<Projection>& projection, std::vector<MapPos> points, std::vector<RoutingInstruction> instructions) :
        _projection(projection),
        _points(std::move(points)),
        _instructions(std::move(instructions))
    {
        if (!projection) {
            throw NullArgumentException("Null projection");
        }
    }

    RoutingResult::~RoutingResult() {
    }

    const std::shared_ptr<Projection>& RoutingResult::getProjection() const {
        return _projection;
    }

    const std::vector<MapPos>& RoutingResult::getPoints() const {
        return _points;
    }

    const std::vector<RoutingInstruction>& RoutingResult::getInstructions() const {
        return _instructions;
    }

    double RoutingResult::getTotalDistance() const {
        return std::accumulate(_instructions.begin(), _instructions.end(), 0.0, [](double dist, const RoutingInstruction& instruction) {
            return dist + instruction.getDistance();
        });
    }
    
    double RoutingResult::getTotalTime() const {
        return std::accumulate(_instructions.begin(), _instructions.end(), 0.0, [](double time, const RoutingInstruction& instruction) {
            return time + instruction.getTime();
        });
    }

    std::string RoutingResult::toString() const {
        std::stringstream ss;
        ss << std::setiosflags(std::ios::fixed);
        ss << "RoutingResult [";
        ss << "instructions=" << _instructions.size() << ", ";
        ss << "totalDistance=" << getTotalDistance() << ", ";
        ss << "totalTime=" << getTotalTime();
        ss << "]";
        return ss.str();
    }

}

#endif
