#ifdef _CARTO_ROUTING_SUPPORT

#include "RoutingResultBuilder.h"
#include "projections/Projection.h"
#include "projections/EPSG3857.h"
#include "utils/Const.h"

#include <utility>
#include <cmath>

namespace carto {

    RoutingResultBuilder::RoutingResultBuilder(const std::shared_ptr<Projection>& proj) :
        _projection(proj),
        _points(),
        _instructions()
    {
    }

    int RoutingResultBuilder::addPoints(const std::vector<MapPos>& points) {
        int pointIndex0 = static_cast<int>(_points.size());
        for (std::size_t i = 0; i < points.size(); i++) {
            if (i != 0 || _points.empty() || _points.back() != points[i]) {
                _points.push_back(points[i]);
            }
            if (i == 0) {
                pointIndex0 = static_cast<int>(_points.size() - 1);
            }
        }
        return pointIndex0;
    }

    RoutingInstructionBuilder& RoutingResultBuilder::addInstruction(RoutingAction::RoutingAction action, int pointIndex) {
        _instructions.emplace_back();
        RoutingInstructionBuilder& instrBuilder = _instructions.back();
        instrBuilder.setAction(action);
        instrBuilder.setPointIndex(pointIndex);
        instrBuilder.setAzimuth(std::numeric_limits<float>::quiet_NaN());
        instrBuilder.setTurnAngle(std::numeric_limits<float>::quiet_NaN());
        return instrBuilder;
    }

    std::shared_ptr<RoutingResult> RoutingResultBuilder::buildRoutingResult() const {
        std::vector<RoutingInstruction> instructions;
        instructions.reserve(_instructions.size());
        for (RoutingInstructionBuilder instrBuilder : _instructions) {
            if (std::isnan(instrBuilder.getTurnAngle())) {
                instrBuilder.setTurnAngle(calculateTurnAngle(instrBuilder.getPointIndex()));
            }
            if (std::isnan(instrBuilder.getAzimuth())) {
                instrBuilder.setAzimuth(calculateAzimuth(instrBuilder.getPointIndex()));
            }
            instructions.push_back(instrBuilder.buildRoutingInstruction());
        }
        return std::make_shared<RoutingResult>(_projection, _points, std::move(instructions));
    }

    float RoutingResultBuilder::calculateTurnAngle(int pointIndex) const {
        EPSG3857 epsg3857;

        if (pointIndex < 0 || pointIndex >= static_cast<int>(_points.size())) {
            return 0.0f;
        }

        MapPos p1 = epsg3857.fromInternal(_projection->toInternal(_points[pointIndex]));
        MapPos p0 = p1;
        MapPos p2 = p1;

        int pointIndex0 = pointIndex;
        while (--pointIndex0 >= 0) {
            p0 = epsg3857.fromInternal(_projection->toInternal(_points[pointIndex0]));
            if (p0 != p1) {
                break;
            }
        }
        int pointIndex2 = pointIndex;
        while (++pointIndex2 < static_cast<int>(_points.size())) {
            p2 = epsg3857.fromInternal(_projection->toInternal(_points[pointIndex2]));
            if (p2 != p1) {
                break;
            }
        }

        if (pointIndex0 >= 0 && pointIndex2 < static_cast<int>(_points.size())) {
            MapVec v10 = p1 - p0;
            MapVec v21 = p2 - p1;
            double dot = v10.dotProduct(v21) / (v10.length() * v21.length());
            return static_cast<float>(std::acos(std::max(-1.0, std::min(1.0, dot))) * Const::RAD_TO_DEG);
        }
        return 0.0f;
    }

    float RoutingResultBuilder::calculateAzimuth(int pointIndex) const {
        EPSG3857 epsg3857;

        int step = 1;
        for (int i = pointIndex; i >= 0; i += step) {
            if (i + 1 >= static_cast<int>(_points.size())) {
                step = -1;
                continue;
            }

            MapPos p0 = epsg3857.fromInternal(_projection->toInternal(_points[i + 0]));
            MapPos p1 = epsg3857.fromInternal(_projection->toInternal(_points[i + 1]));
            MapVec v10 = p1 - p0;
            if (v10.length() > 0) {
                float angle = static_cast<float>(std::atan2(v10.getY(), v10.getX()) * Const::RAD_TO_DEG);
                float azimuth = 90.0f - angle;
                return (azimuth < 0 ? azimuth + 360.0f : azimuth);
            }
        }
        return std::numeric_limits<float>::quiet_NaN();
    }

}

#endif
