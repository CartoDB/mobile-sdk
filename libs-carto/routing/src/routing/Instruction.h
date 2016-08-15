/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ROUTING_INSTRUCTION_H_
#define _CARTO_ROUTING_INSTRUCTION_H_

#include "Base.h"

#include <cstddef>
#include <string>

namespace carto { namespace routing {
    class Instruction {
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

        Instruction() = default;
        explicit Instruction(Type type, TravelMode travelMode, std::string address, double distance, double time, std::size_t geometryIndex) : _type(type), _travelMode(travelMode), _address(std::move(address)), _distance(distance), _time(time), _geometryIndex(geometryIndex) { }

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
} }

#endif
