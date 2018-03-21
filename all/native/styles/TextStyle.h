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
     * Margins info for text.
     */
    class TextMargins {
    public:
        TextMargins(int left, int top, int right, int bottom);
        virtual ~TextMargins();
        
        int getLeft() const;
        int getTop() const;
        int getRight() const;
        int getBottom() const;
        
    private:
        int _left;
        int _top;
        int _right;
        int _bottom;
    };

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
         * @param causesOverlap The causes overlap flag for the text.
         * @param hideIfOverlapped The hide if overlapped flag for the text.
         * @param horizontalOffset The horizontal offset.
         * @param verticalOffset The vertical offset.
         * @param placementPriority The placement priority.
         * @param scaleWithDPI The scale with DPI flag for the text.
         * @param animStyle The animation style for the text.
         * @param anchorPointX The horizontal anchor point.
         * @param anchorPointY The vertical anchor point.
         * @param flippable The fliappble flag.
         * @param orientationMode The orientation mode.
         * @param scalingMode The scaling mode.
         * @param renderScale The relative rendering scale.
         * @param fontName The font's name.
         * @param textField The text field variable to use.
         * @param fontSize The font's size.
         * @param breakLines The 'break lines' flag.
         * @param textMargins The margins for the text.
         * @param strokeColor The color of the stroke.
         * @param strokeWidth The width of the stroke.
         * @param borderColor The color of the border.
         * @param borderWidth The width of the border.
         * @param backgroundColor The color of the background.
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
                  const std::shared_ptr<AnimationStyle>& animStyle,
                  float anchorPointX,
                  float anchorPointY,
                  bool flippable,
                  BillboardOrientation::BillboardOrientation orientationMode,
                  BillboardScaling::BillboardScaling scalingMode,
                  float renderScale,
                  const std::string& fontName,
                  const std::string& textField,
                  float fontSize,
                  bool breakLines,
                  const TextMargins& textMargins,
                  const Color& strokeColor,
                  float strokeWidth,
                  const Color& borderColor,
                  float borderWidth,
                  const Color& backgroundColor);
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
        float getFontSize() const;

        /**
         * Returns the state of the 'break lines' flag.
         * @return The state of the 'break lines' flag.
         */
        bool isBreakLines() const;

        /**
         * Returns the margins for the text.
         * @return The margins for the text.
         */
        const TextMargins& getTextMargins() const;

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

        /**
         * Returns the color of the border.
         * @return The color of the border around the text.
         */
        const Color& getBorderColor() const;

        /**
         * Returns the width of the border.
         * @return The width of the border around the text.
         */
        float getBorderWidth() const;

        /**
         * Returns the background color of the text.
         * @return The background color of the text.
         */
        const Color& getBackgroundColor() const;

    protected:
        Color _fontColor;

        std::string _fontName;
        
        std::string _textField;

        float _fontSize;

        bool _breakLines;

        TextMargins _textMargins;

        Color _strokeColor;

        float _strokeWidth;

        Color _borderColor;

        float _borderWidth;

        Color _backgroundColor;
    };

}

#endif
