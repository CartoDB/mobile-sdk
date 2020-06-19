/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_LINESTYLE_H_
#define _CARTO_LINESTYLE_H_

#include "styles/Style.h"

#include <memory>

namespace carto {
    
    namespace LineEndType {
        /**
         * Possible styles in which line end points are drawn.
         */
        enum LineEndType {
            /**
             * No line end points are drawn.
             */
            LINE_END_TYPE_NONE,
            /**
             * Line end points are drawn as squares.
             */
            LINE_END_TYPE_SQUARE,
            /**
             * Line end points are drawn as half circles.
             */
            LINE_END_TYPE_ROUND
        };
    }
    
    namespace LineJoinType {
        /**
         * Possible styles in which line segments are connected with each other.
         */
        enum LineJoinType {
            /**
             * Line segments are not connected with each other. The fastest and ugliest.
             */
            LINE_JOIN_TYPE_NONE,
            /**
             * Line segments are connected with each other using miter connections. This is the preferred mode (fast and good looking generally).
             */
            LINE_JOIN_TYPE_MITER,
            /**
             * Line segments are connected with each other using bevel (straight line) connectors. Fast
             * but results in an unnatural line.
             */
            LINE_JOIN_TYPE_BEVEL,
            /**
             * Line segments are connected with each other using circle sectors resulting in 
             * round corners. Slowest and prettiest.
             */
            LINE_JOIN_TYPE_ROUND
        };
    }
    
    class Bitmap;
    
    /**
     * A style for lines. Contains attributes for configuring how the line is drawn on the screen.
     */
    class LineStyle : public Style {
    public:
        /**
         * Constructs a LineStyle object from various parameters. Instantiating the object directly is
         * not recommended, LineStyleBuilder should be used instead.
         * @param color The color for the line.
         * @param bitmap The bitmap for the line.
         * @param clickWidth The width of the line used for click detection.
         * @param lineEndType The line's end point type.
         * @param lineJoinType The join type for the line.
         * @param stretchFactor The color for the line.
         * @param width The width for the line.
         */
        LineStyle(const Color& color, const std::shared_ptr<Bitmap>& bitmap, float clickWidth,
                LineEndType::LineEndType lineEndType, LineJoinType::LineJoinType lineJoinType,
                float stretchFactor, float width);
        virtual ~LineStyle();
    
        /**
         * Returns the bitmap of the line.
         * @return The bitmap of the line.
         */
        std::shared_ptr<Bitmap> getBitmap() const;
        
        /**
         * Returns the width of the line used for click detection.
         * @return The width of the line used for click detection.
         */
        float getClickWidth() const;
    
        /**
         * Returns the end point type of the line.
         * @return The end point type of the line.
         */
        LineEndType::LineEndType getLineEndType() const;
    
        /**
         * Returns the join type of the line.
         * @return The join type of the line.
         */
        LineJoinType::LineJoinType getLineJoinType() const;
    
        /**
         * Returns the stretching factor of the line.
         * @return The stretching factor of the line.
         */
        float getStretchFactor() const;
    
        /**
         * Returns the width of the line.
         * @return The width of the line.
         */
        float getWidth() const;
    
    protected:
        std::shared_ptr<Bitmap> _bitmap;
        
        float _clickWidth;
    
        LineEndType::LineEndType _lineEndType;
    
        LineJoinType::LineJoinType _lineJoinType;
    
        float _stretchFactor;
    
        float _width;
    
    };
    
}

#endif
