/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_GEOMETRYCOLLECTIONDRAWDATA_H_
#define _CARTO_GEOMETRYCOLLECTIONDRAWDATA_H_

#include "renderers/drawdatas/VectorElementDrawData.h"

#include <memory>
#include <vector>

namespace carto {
    class Bitmap;
    class Geometry;
    class MultiGeometry;
    class GeometryCollection;
    class GeometryCollectionStyle;
    class Projection;

    class GeometryCollectionDrawData : public VectorElementDrawData {
    public:
        GeometryCollectionDrawData(const MultiGeometry& geometry, const GeometryCollectionStyle& style, const Projection& projection);
        virtual ~GeometryCollectionDrawData();

        const std::vector<std::shared_ptr<VectorElementDrawData> >& getDrawDatas() const;

        virtual void offsetHorizontally(double offset);

    private:
        void addDrawData(const Geometry& geometry, const GeometryCollectionStyle& style, const Projection& projection);

        std::vector<std::shared_ptr<VectorElementDrawData> > _drawDatas;
    };

}

#endif
