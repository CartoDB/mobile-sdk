/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ROUTING_ROUTINGOBJECTS_H_
#define _CARTO_ROUTING_ROUTINGOBJECTS_H_

#include <cstdint>
#include <cstddef>
#include <vector>
#include <memory>
#include <array>
#include <numeric>
#include <functional>
#include <utility>

#include <cglib/vec.h>
#include <cglib/bbox.h>

namespace carto { namespace Routing {
    using WGSPos = cglib::vec2<double>;
    using WGSBounds = cglib::bbox2<double>;

    class RoutingQuery {
    public:
        RoutingQuery() = delete;
        explicit RoutingQuery(const WGSPos& pos0, const WGSPos& pos1) {
            _points[0] = pos0;
            _points[1] = pos1;
        }

        WGSPos getPos(int index) const {
            return _points[index];
        }

    private:
        std::array<WGSPos, 2> _points;
    };

    class RoutingInstruction {
    public:
        enum class Type : unsigned char {
            NO_TURN = 0,
            GO_STRAIGHT,
            TURN_SLIGHT_RIGHT,
            TURN_RIGHT,
            TURN_SHARP_RIGHT,
            UTURN,
            TURN_SHARP_LEFT,
            TURN_LEFT,
            TURN_SLIGHT_LEFT,
            REACH_VIA_LOCATION,
            HEAD_ON,
            ENTER_ROUNDABOUT,
            LEAVE_ROUNDABOUT,
            STAY_ON_ROUNDABOUT,
            START_AT_END_OF_STREET,
            REACHED_YOUR_DESTINATION,
            ENTER_AGAINST_ALLOWED_DIRECTION,
            LEAVE_AGAINST_ALLOWED_DIRECTION,
            INVERSE_ACCESS_RESTRICTION_FLAG = 127,
            ACCESS_RESTRICTION_FLAG = 128,
            ACCESS_RESTRICTION_PENALTY = 129
        };

        enum class TravelMode : unsigned char {
            DEFAULT = 0, // Note: DEFAULT/INACCESSIBLE encodings are inverted vs OSRM
            INACCESSIBLE
        };

        RoutingInstruction() = default;
        explicit RoutingInstruction(Type type, TravelMode travelMode, std::string address, double distance, double time, std::size_t geometryIndex) : _type(type), _travelMode(travelMode), _address(std::move(address)), _distance(distance), _time(time), _geometryIndex(geometryIndex) { }

        Type getType() const {
            return _type;
        }

        TravelMode getTravelMode() const {
            return _travelMode;
        }

        const std::string getAddress() const {
            return _address;
        }

        double getDistance() const {
            return _distance;
        }

        double getTime() const {
            return _time;
        }

        std::size_t getGeometryIndex() const {
            return _geometryIndex;
        }

    private:
        Type _type = Type::NO_TURN;
        TravelMode _travelMode = TravelMode::DEFAULT;
        std::string _address;
        double _distance = 0.0;
        double _time = 0.0;
        std::size_t _geometryIndex = 0;
    };

    class RoutingResult {
    public:
        enum class Status {
            FAILED,
            SUCCESS
        };

        RoutingResult() = default;
        explicit RoutingResult(std::vector<RoutingInstruction> instructions, std::vector<WGSPos> geometry) : _status(Status::SUCCESS), _instructions(std::move(instructions)), _geometry(std::move(geometry)) { }

        Status getStatus() const {
            return _status;
        }

        const std::vector<RoutingInstruction>& getInstructions() const {
            return _instructions;
        }

        const std::vector<WGSPos>& getGeometry() const {
            return _geometry;
        }

        double getTotalDistance() const {
            return std::accumulate(_instructions.begin(), _instructions.end(), 0.0, [](double dist, const RoutingInstruction& instruction) {
                return dist + instruction.getDistance();
            });
        }

        double getTotalTime() const {
            return std::accumulate(_instructions.begin(), _instructions.end(), 0.0, [](double time, const RoutingInstruction& instruction) {
                return time + instruction.getTime();
            });
        }

    private:
        Status _status = Status::FAILED;
        std::vector<RoutingInstruction> _instructions;
        std::vector<WGSPos> _geometry;
    };
} }

#endif
