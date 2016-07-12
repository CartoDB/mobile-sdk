#include "RoutingResult.h"

#include <numeric>
#include <functional>
#include <utility>

namespace carto {

    RoutingResult::RoutingResult(const std::shared_ptr<Projection>& projection, const std::vector<MapPos>& points, const std::vector<RoutingInstruction>& instructions) :
        _projection(projection),
        _points(points),
        _instructions(instructions)
    {
        if (!projection) {
            throw std::invalid_argument("Null projection");
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
        return std::accumulate(_instructions.begin(), _instructions.end(), 0.0, [](double time, const RoutingInstruction& instruction) {
            return time + instruction.getDistance();
        });
    }
    
    double RoutingResult::getTotalTime() const {
        return std::accumulate(_instructions.begin(), _instructions.end(), 0.0, [](double time, const RoutingInstruction& instruction) {
            return time + instruction.getTime();
        });
    }

}

