/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_TEXTSTYLE_H_
#define _CARTO_TEXTSTYLE_H_

#include "styles/LabelStyle.h"

#include <string>

namespace carto {

    /**
     * A style for text labels. Contains attributes for configuring how the text label is drawn on the screen.
     */
    class TextStyle : public LabelStyle {
    public:
        /**
         * Constructs a TextStyle object from various parameters. Instantiating the object directly is
         * not recommended, TextStyleBuilder should be used instead.
         * @param color The color for the text.
         * @param attachAnchorPointX The horizontal attaching anchor point.
         * @param attachAnchorPointY The vertical attaching anchor point.
         * @param causesOverlap The causes overlap flag for the billboard.
         * @param hideIfOverlapped The hide if overlapped flag for the billboard.
         * @param horizontalOffset The horizontal offset.
         * @param verticalOffset The vertical offset.
         * @param placementPriority The placement priority.
         * @param scaleWithDPI The scale with DPI flag for the label.
         * @param anchorPointX The horizontal anchor point.
         * @param anchorPointY The vertical anchor point.
         * @param flippable The fliappble flag.
         * @param orientationMode The orientation mode.
         * @param scalingMode The scaling mode.
         * @param fontName The font's name.
         * @param textField The text field variable to use.
         * @param fontSize The font's size.
         * @param strokeColor The width of the color.
         * @param strokeWidth The width of the stroke.
         */
        TextStyle(const Color& color,
                  float attachAnchorPointX,
                  float attachAnchorPointY,
                  bool causesOverlap,
                  bool hideIfOverlapped,
                  float horizontalOffset,
                  float verticalOffset,
                  int placementPriority,
                  bool scaleWithDPI,
                  float anchorPointX,
                  float anchorPointY,
                  bool flippable,
                  BillboardOrientation::BillboardOrientation orientationMode,
                  BillboardScaling::BillboardScaling scalingMode,
                  const std::string& fontName,
                  const std::string& textField,
                  int fontSize,
                  const Color& strokeColor,
                  float strokeWidth);
        virtual ~TextStyle();
        
        /**
         * Returns the font's color.
         * @return The color of the font.
         */
        const Color& getFontColor();

        /**
         * Returns the font's name.
         * @return The platform dependent name of the font.
         */
        const std::string& getFontName() const;
        
        /**
         * Returns the text field variable to use.
         * @return The text field variable.
         */
        const std::string& getTextField() const;

        /**
         * Returns the font's size.
         * @return The size of the font in points.
         */
        int getFontSize() const;

        /**
         * Returns the color of the stroke.
         * @return The color of the stroke around the text.
         */
        const Color& getStrokeColor() const;

        /**
         * Returns the width of the stroke.
         * @return The width of the stroke around the text.
         */
        float getStrokeWidth() const;

    protected:
        Color _fontColor;

        std::string _fontName;
        
        std::string _textField;

        int _fontSize;

        Color _strokeColor;

        float _strokeWidth;
    };

}

#endif
