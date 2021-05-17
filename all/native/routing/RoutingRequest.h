/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ROUTINGREQUEST_H_
#define _CARTO_ROUTINGREQUEST_H_

#ifdef _CARTO_ROUTING_SUPPORT

#include "core/MapPos.h"
#include "core/Variant.h"

#include <memory>
#include <mutex>
#include <vector>
#include <map>

namespace carto {
    class Projection;

    /**
     * A class that defines required attributes for routing (end points, etc).
     */
    class RoutingRequest {
    public:
        /**
         * Constructs a new RoutingRequest instance from projection and via points.
         * @param projection The projection of the points.
         * @param points The list of points that the route must pass. Must contains at least 2 elements.
         */
        RoutingRequest(const std::shared_ptr<Projection>& projection, const std::vector<MapPos>& points);
        virtual ~RoutingRequest();

        /**
         * Returns the projection of the points in the request.
         * @return The projection of the request.
         */
        const std::shared_ptr<Projection>& getProjection() const;
        /**
         * Returns the point list of the request.
         * @return The point list of the request.
         */
        const std::vector<MapPos>& getPoints() const;

        /**
         * Returns the set of given point parameters as a variant.
         * @return The set of given point parameters as a variant. Can be empty.
         */
        Variant getPointParameters(int index) const;
        /**
         * Returns the parameter value for the given routing point.
         * @param index The routing point index.
         * @param param The name of the parameter.
         * @return The value of the specified parameter of the given routing point. If the parameter does not exist, empty variant is returned.
         */
        Variant getPointParameter(int index, const std::string& param) const;
        /**
         * Sets the parameter value for the given routing point.
         * This is currently supported by Valhalla routing engine and can be used to specify initial or final heading, for example.
         * In addition to Valhalla, SGRE routing engine supports 'geometry_tag_filter' parameter for specifying how to filter matching edges.
         * @param index The routing point index.
         * @param param The name of the parameter to set.
         * @param value The new value for the parameter of the given routing point.
         */
        void setPointParameter(int index, const std::string& param, const Variant& value);

        /**
         * Returns the set of custom parameters of the request as a variant.
         * @return The set of custom parameters as a variant. Can be empty.
         */
        Variant getCustomParameters() const;
        /**
         * Returns the custom parameter value of the request.
         * @param param The name of the parameter to return.
         * @return The value of the parameter. If the parameter does not exist, empty variant is returned.
         */
        Variant getCustomParameter(const std::string& param) const;
        /**
         * Sets a custom parameter value for the the request.
         * @param param The name of the parameter. For example, "trace_options.search_radius".
         * @param value The new value for the parameter.
         */
        void setCustomParameter(const std::string& param, const Variant& value);

        /**
         * Creates a string representation of this request object, useful for logging.
         * @return The string representation of this request object.
         */
        std::string toString() const;
        
    private:
        const std::shared_ptr<Projection> _projection;
        const std::vector<MapPos> _points;
        std::map<int, Variant> _pointParams;
        Variant _customParams;

        mutable std::mutex _mutex;
    };
    
}

#endif

#endif
