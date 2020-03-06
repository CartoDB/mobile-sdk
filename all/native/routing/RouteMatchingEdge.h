/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_ROUTEMATCHINGEDGE_H_
#define _CARTO_ROUTEMATCHINGEDGE_H_

#ifdef _CARTO_ROUTING_SUPPORT

#include "core/Variant.h"

#include <string>
#include <map>

namespace carto {

    /**
     * A class that defines a route matching result edge.
     */
    class RouteMatchingEdge {
    public:
        /**
         * Constructs a new RouteMatchingEdge instance with no attributes.
         */
        RouteMatchingEdge();
        /**
         * Constructs a new RouteMatchingEdge from an attributes map.
         * @param attributes The attributes map.
         */
        RouteMatchingEdge(const std::map<std::string, Variant>& attributes);
        virtual ~RouteMatchingEdge();

        /**
         * Returns true if edge contains the specified attribute.
         * @param name The name of the attribute to check.
         * @return True if the specified attribute exists in the edge.
         */
        bool containsAttribute(const std::string& name) const;
        /**
         * Returns the value of the specified attribute.
         * @param name The name of the attribute to return.
         * @return The attribute value, if attribute exists. Null variant is returned otherwise.
         */
        Variant getAttribute(const std::string& name) const;

        /**
         * Creates a string representation of this object, useful for logging.
         * @return The string representation of this object.
         */
        std::string toString() const;
        
    private:
        std::map<std::string, Variant> _attributes;
    };
    
}

#endif

#endif
