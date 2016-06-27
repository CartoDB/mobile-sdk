/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_LINEDRAWDATA_H_
#define _CARTO_LINEDRAWDATA_H_

#include "renderers/drawdatas/VectorElementDrawData.h"

#include <memory>
#include <vector>

#include <cglib/vec.h>

namespace carto {
    class Bitmap;
    class LineGeometry;
    class LineStyle;
    class MapPos;
    class PolygonGeometry;
    class Projection;
    
    class LineDrawData : public VectorElementDrawData {
    public:
        // Saves a little space compared to MapPos
        LineDrawData(const LineGeometry& geometry, const LineStyle& style, const Projection& projection);
        LineDrawData(const PolygonGeometry& geometry, const std::vector<MapPos>& internalPoses, const LineStyle& style, const Projection& projection);
        LineDrawData(const LineDrawData& lineDrawData);
        virtual ~LineDrawData();
    
        const std::shared_ptr<Bitmap> getBitmap() const;

        float getNormalScale() const;
    
        float getClickScale() const;
    
        const std::vector<std::vector<cglib::vec3<double>*> >& getCoords() const;
    
        const std::vector<std::vector<cglib::vec3<float> > >& getNormals() const;
    
        const std::vector<std::vector<cglib::vec2<float> > >& getTexCoords() const;
    
        const std::vector<std::vector<unsigned int> >& getIndices() const;
    
        virtual void offsetHorizontally(double offset);
    
    private:
        static const float LINE_ENDPOINT_TESSELATION_FACTOR;
        static const float LINE_JOIN_TESSELATION_FACTOR;
        static const float LINE_JOIN_MIN_MITER_DOT;
    
        static const int IDEAL_CLICK_WIDTH = 64;
    
        static const float CLICK_WIDTH_COEF;
        
        void init(const LineStyle& style);
    
        std::shared_ptr<Bitmap> _bitmap;
    
        float _normalScale;

        float _clickScale;
    
        // Actual line coordinates
        std::vector<cglib::vec3<double> > _poses;
    
        // Origin point and normal for each vertex
        std::vector<std::vector<cglib::vec3<double>*> > _coords;
        std::vector<std::vector<cglib::vec3<float> > > _normals;
        std::vector<std::vector<cglib::vec2<float> > > _texCoords;
    
        std::vector<std::vector<unsigned int> > _indices;
    };
    
}

#endif
