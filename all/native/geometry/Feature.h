/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_FEATURE_H_
#define _CARTO_FEATURE_H_

#include "core/Variant.h"

#include <memory>

namespace carto {
    class Geometry;

    /**
     * A feature consisting of geometry and properties
     */
    class Feature {
    public:
        /**
         * Constructs a Feature object from geometry and properties.
         * @param geometry The geometry of the feature.
         * @param properties The properties of the feature.
         */
        Feature(const std::shared_ptr<Geometry>& geometry, const Variant& properties);
        virtual ~Feature();
        
        /**
         * Returns the geometry of the feature.
         * @return The geometry of the feature.
         */
        const std::shared_ptr<Geometry>& getGeometry() const;
    
        /**
         * Returns the properties of the feature.
         * @return The properties of the feature.
         */
        const Variant& getProperties() const;
    
    protected:
        const std::shared_ptr<Geometry> _geometry;
        const Variant _properties;
    };
    
}

#endif
