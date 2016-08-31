/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ROUTINGINSTRUCTION_H_
#define _CARTO_ROUTINGINSTRUCTION_H_

#ifdef _CARTO_ROUTING_SUPPORT

#include "core/MapPos.h"

#include <memory>
#include <string>

namespace carto {

    namespace RoutingAction {
        /**
         * Routing action type.
         */
        enum RoutingAction {
            /**
             * Head on, start the route.
             */
            ROUTING_ACTION_HEAD_ON,
            /**
             * Finish the route.
             */
            ROUTING_ACTION_FINISH,
            /**
             * Continue along the given street, do not turn.
             */
            ROUTING_ACTION_NO_TURN,
            /**
             * Go straight.
             */
            ROUTING_ACTION_GO_STRAIGHT,
            /**
             * Turn right.
             */
            ROUTING_ACTION_TURN_RIGHT,
            /**
             * Do an u-turn.
             */
            ROUTING_ACTION_UTURN,
            /**
             * Turn left.
             */
            ROUTING_ACTION_TURN_LEFT,
            /**
             * Reached given via point. If this is the final point, FINISH action is used instead.
             */
            ROUTING_ACTION_REACH_VIA_LOCATION,
            /**
             * Enter roundabout.
             */
            ROUTING_ACTION_ENTER_ROUNDABOUT,
            /**
             * Leave roundabout.
             */
            ROUTING_ACTION_LEAVE_ROUNDABOUT,
            /**
             * Continue along the roundabout.
             */
            ROUTING_ACTION_STAY_ON_ROUNDABOUT,
            /**
             * Start at the end of the street.
             */
            ROUTING_ACTION_START_AT_END_OF_STREET,
            /**
             * Enter street while moving against the allowed direction.
             */
            ROUTING_ACTION_ENTER_AGAINST_ALLOWED_DIRECTION,
            /**
             * Leave the street while moving aginst the allowed direction.
             */
            ROUTING_ACTION_LEAVE_AGAINST_ALLOWED_DIRECTION
        };
    }

    /**
     * A class that defines a routing instruction at specified position in the path.
     */
    class RoutingInstruction {
    public:
        /**
         * Constructs a new RoutingInstruction instance with all values set to default.
         */
        RoutingInstruction();
        /**
         * Constructs a new RoutingInstruction instance given all instruction attributes.
         * @param action Action to take.
         * @param pointIndex Instruction starting point index in the point list.
         * @param streetName Street name.
         * @param turnAngle Turn angle in degrees.
         * @param azimuth Azimuth in degrees.
         * @param distance The distance to move along the given street in meters.
         * @param time The approximate duration of the instruction in seconds.
         */
        RoutingInstruction(RoutingAction::RoutingAction action, int pointIndex, const std::string& streetName, float turnAngle, float azimuth, double distance, double time);
        virtual ~RoutingInstruction();
        
        /**
         * Returns the action of the instruction.
         * @return The action to take.
         */
        RoutingAction::RoutingAction getAction() const;
        /**
         * Returns the index of the first geometry point in external point array.
         * @return The point index.
         */
        int getPointIndex() const;
        /**
         * Returns the name of street.
         * @return The name of the street.
         */
        const std::string& getStreetName() const;
        /**
         * Returns the turn angle of the action.
         * @return The turn angle in degrees.
         */
        float getTurnAngle() const;
        /**
         * Returns the azimuth of the initial position.
         * @return The azimuth in degrees.
         */
        float getAzimuth() const;
        /**
         * Returns the distance to move along the given street.
         * @return The distance to move in meters.
         */
        double getDistance() const;
        /**
         * Sets the distance to move along the given street.
         * @param distance The new distance in meters.
         */
        void setDistance(double distance);
        /**
         * Returns the time approximate duration of the instruction.
         * @return The approximate duration of the instruction in seconds.
         */
        double getTime() const;
        /**
         * Sets the approximate duration of the instruction.
         * @param time The duration of the instruction in seconds.
         */
        void setTime(double time);

        /**
         * Creates a string representation of this instruction, useful for logging.
         * @return The string representation of this instruction.
         */
        std::string toString() const;

    private:
        RoutingAction::RoutingAction _action;
        int _pointIndex;
        std::string _streetName;
        float _turnAngle;
        float _azimuth;
        double _distance;
        double _time;
    };
    
}

#endif

#endif
