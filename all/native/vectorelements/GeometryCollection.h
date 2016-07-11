/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOMETRYCOLLECTION_H_
#define _CARTO_GEOMETRYCOLLECTION_H_

#include "vectorelements/VectorElement.h"

#include <vector>

namespace carto {
    class MultiGeometry;
    class GeometryCollectionDrawData;
    class GeometryCollectionStyle;

    /**
     * A collection of multiple geometries that can be displayed on the map.
     */
    class GeometryCollection : public VectorElement {
    public:
        /**
         * Constructs a new GeometryCollection object from multigeometry and style.
         * @param geometry The multigeometry containing possibly points, lines, polygons.
         * @param style The style defining multigeometry rendering.
         */
        GeometryCollection(const std::shared_ptr<MultiGeometry>& geometry, const std::shared_ptr<GeometryCollectionStyle>& style);
        virtual ~GeometryCollection();

        std::shared_ptr<MultiGeometry> getGeometry() const;
        /**
         * Sets the geometry for this geometry collection.
         * @param geometry The new geometry object.
         */
        void setGeometry(const std::shared_ptr<MultiGeometry>& geometry);

        /**
         * Returns the style of this geometry collection.
         * @return The style that defines multigeometry rendering.
         */
        std::shared_ptr<GeometryCollectionStyle> getStyle() const;
        /**
         * Sets the style for this geometry collection.
         * @param style The new style that defines defines multigeometry rendering.
         */
        void setStyle(const std::shared_ptr<GeometryCollectionStyle>& style);

        std::shared_ptr<GeometryCollectionDrawData> getDrawData() const;
        void setDrawData(const std::shared_ptr<GeometryCollectionDrawData>& drawData);
        
    protected:
        friend class GeometryCollectionRenderer;
        friend class VectorLayer;

    private:
        std::shared_ptr<GeometryCollectionDrawData> _drawData;

        std::shared_ptr<GeometryCollectionStyle> _style;
    };

}

#endif
