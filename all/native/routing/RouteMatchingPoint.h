/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ROUTEMATCHINGPOINT_H_
#define _CARTO_ROUTEMATCHINGPOINT_H_

#ifdef _CARTO_ROUTING_SUPPORT

#include "core/MapPos.h"

#include <string>

namespace carto {

    namespace RouteMatchingPointType {
        /**
         * Route matching point type.
         */
        enum RouteMatchingPointType {
            /**
             * The point was unmatched.
             */
            ROUTE_MATCHING_POINT_UNMATCHED,
            /**
             * The point was interpolated.
             */
            ROUTE_MATCHING_POINT_INTERPOLATED,
            /**
             * The point was matched.
             */
            ROUTE_MATCHING_POINT_MATCHED
        };
    }

    /**
     * A class that defines a route matching result point.
     */
    class RouteMatchingPoint {
    public:
        /**
         * Constructs a new RouteMatchingPoint with default values.
         */
        RouteMatchingPoint();
        /**
         * Constructs a new RouteMatchingPoint from position, type and edge index.
         * @param pos The position of the point.
         * @param type The type of the matching point.
         * @param edgeIndex The index of the corresponding RouteMatchingEdge instance.
         */
        RouteMatchingPoint(const MapPos& pos, RouteMatchingPointType::RouteMatchingPointType type, int edgeIndex);
        virtual ~RouteMatchingPoint();

        /**
         * Returns the position of the matching point.
         * @return The position of the matching point.
         */
        const MapPos& getPos() const;

        /**
         * Returns the type of the matching point.
         * @return The type of the matching point.
         */
        RouteMatchingPointType::RouteMatchingPointType getType() const;

        /**
         * Returns the corresponding matching edge index in the matching result.
         * @return The corresponding matching edge index in the matching result.
         */
        int getEdgeIndex() const;

        /**
         * Creates a string representation of this object, useful for logging.
         * @return The string representation of this object.
         */
        std::string toString() const;
        
    private:
        MapPos _pos;
        RouteMatchingPointType::RouteMatchingPointType _type;
        int _edgeIndex;
    };
    
}

#endif

#endif
