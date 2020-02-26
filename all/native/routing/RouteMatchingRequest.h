/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ROUTEMATCHINGREQUEST_H_
#define _CARTO_ROUTEMATCHINGREQUEST_H_

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
     * A class that defines required attributes for route matching.
     */
    class RouteMatchingRequest {
    public:
        /**
         * Constructs a new RouteMatchingRequest instance from projection, measured points and accuracy.
         * @param projection The projection of the points.
         * @param points The list of points to match. Must contains at least 1 element.
         * @param accuracy Accuracy of the points in meters.
         */
        RouteMatchingRequest(const std::shared_ptr<Projection>& projection, const std::vector<MapPos>& points, float accuracy);
        virtual ~RouteMatchingRequest();

        /**
         * Returns the projection of the points in the request.
         * @return The projection of the request.
         */
        const std::shared_ptr<Projection>& getProjection() const;
        /**
         * Returns the measured points of the request.
         * @return The measured points of the request.
         */
        const std::vector<MapPos>& getPoints() const;
        /**
         * Returns the accuracy of the points in the request.
         * @return The accuracy of the points in the request.
         */
        float getAccuracy() const;

        /**
         * Returns the custom parameters of the request.
         * @return The custom parameters of the request.
         */
        std::map<std::string, Variant> getCustomParameters() const;
        /**
         * Sets the custom parameters for the the request.
         * @param customParams The map of custom parameters to set.
         */
        void setCustomParameters(const std::map<std::string, Variant>& customParams);

        /**
         * Creates a string representation of this request object, useful for logging.
         * @return The string representation of this request object.
         */
        std::string toString() const;
        
    private:
        std::shared_ptr<Projection> _projection;
        std::vector<MapPos> _points;
        float _accuracy;

        std::map<std::string, Variant> _customParams;
        mutable std::mutex _mutex;
    };
    
}

#endif

#endif
