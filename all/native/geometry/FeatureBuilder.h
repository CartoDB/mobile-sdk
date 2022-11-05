/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_FEATUREBUILDER_H_
#define _CARTO_FEATUREBUILDER_H_

#include "core/Variant.h"

#include <string>
#include <memory>
#include <mutex>

namespace carto {
    class Geometry;
    class Feature;

    /**
     * A feature builder for constructing individual features.
     */
    class FeatureBuilder {
    public:
        /**
         * Constructs a FeatureBuilder with empty state.
         */
        FeatureBuilder();
        virtual ~FeatureBuilder();
        
        /**
         * Returns the geometry of the builder.
         * @return The geometry of the builder.
         */
        std::shared_ptr<Geometry> getGeometry() const;
        /**
         * Sets the geometry of the builder.
         * @param geometry The new geometry to use for the builder.
         */
        void setGeometry(const std::shared_ptr<Geometry>& geometry);
    
        /**
         * Returns a specified property value of the builder.
         * @param key The name of the property to return.
         * @return The value of the property. If the key does not exist an empty variant is returned.
         */
        Variant getPropertyValue(const std::string& key) const;
        /**
         * Sets an individual property value of the builder.
         * @param key The name of the property to set.
         * @param value The value of the property.
         */
        void setPropertyValue(const std::string& key, const Variant& value);

        /**
         * Builds a new feature from the state.
         * @return A new feature based on the builder state. Can be null if the geometry is not specified.
         */
        std::shared_ptr<Feature> buildFeature() const;
    
    protected:
        std::shared_ptr<Geometry> _geometry;
        std::map<std::string, Variant> _propertyMap;

        mutable std::mutex _mutex;
    };
    
}

#endif
