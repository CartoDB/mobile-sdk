/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_POLYGONDRAWDATA_H_
#define _CARTO_POLYGONDRAWDATA_H_

#include "renderers/drawdatas/VectorElementDrawData.h"

#include <memory>
#include <vector>

#include <cglib/vec.h>
#include <cglib/bbox.h>

namespace carto {
    class Bitmap;
    class LineDrawData;
    class MapPos;
    class PolygonGeometry;
    class PolygonStyle;
    class Projection;
    
    class PolygonDrawData : public VectorElementDrawData {
    public:
        PolygonDrawData(const PolygonGeometry& geometry, const PolygonStyle& style, const Projection& projection);
        PolygonDrawData(const PolygonDrawData& drawData);
        virtual ~PolygonDrawData();
    
        const std::shared_ptr<Bitmap> getBitmap() const;
    
        const cglib::bbox3<double>& getBoundingBox() const;
    
        const std::vector<std::vector<cglib::vec3<double> > >& getCoords() const;
    
        const std::vector<std::vector<unsigned int> >& getIndices() const;
    
        const std::vector<std::shared_ptr<LineDrawData> >& getLineDrawDatas() const;
    
        virtual void offsetHorizontally(double offset);
    
    private:
        static const int MAX_INDICES_PER_ELEMENT = 3;
    
        std::shared_ptr<Bitmap> _bitmap;
    
        cglib::bbox3<double> _boundingBox;
    
        std::vector<std::vector<cglib::vec3<double> > > _coords;

        std::vector<std::vector<unsigned int> > _indices;
    
        std::vector<std::shared_ptr<LineDrawData> > _lineDrawDatas;
    };
    
}

#endif
