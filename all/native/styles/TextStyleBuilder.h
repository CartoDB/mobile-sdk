/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */


#ifndef _CARTO_TEXTSTYLEBUILDER_H_
#define _CARTO_TEXTSTYLEBUILDER_H_

#include "styles/LabelStyleBuilder.h"
#include "styles/TextStyle.h"

#include <memory>

namespace carto {

    /**
     * A builder class for TextStyle.
     */
    class TextStyleBuilder : public LabelStyleBuilder {
    public:
        /**
         * Constructs a TextStyleBuilder object with all parameters set to defaults.
         */
        TextStyleBuilder();
        virtual ~TextStyleBuilder();

        /**
         * Returns the font name for the text label.
         * @return The platform dependent font name for the text label.
         */
        std::string getFontName() const;
        /**
         * Sets the font name for the text label.
         * @param fontName The new platform dependent font name for the text label.
         */
        void setFontName(const std::string& fontName);
        
        /**
         * Returns the text field variable. If not empty, this variable is used to read actual text string from object meta info.
         * @return The current text variable.
         */
        std::string getTextField() const;
        /**
         * Sets the text field variable. If not empty, this variable is used to read actual text string from object meta info.
         * @param field The text field to use for displaying text from metainfo.
         */
        void setTextField(const std::string& field);

        /**
         * Returns the font size for the text label.
         * @return The font size for the text label in points.
         */
        float getFontSize() const;
        /**
         * Sets the font size for the text label.
         * @param size The new font size for the text label in points.
         */
        void setFontSize(float size);

        /**
         * Returns the state of the 'break lines' flag.
         * @return The state of the 'break lines' flag.
         */
        bool isBreakLines() const;
        /**
         * Sets the state of the 'break lines' flag.
         * If enabled, texts containing CR or NL characters are split into multiple lines.
         * By default the flag is false, due to backward compatibility reasons.
         */
        void setBreakLines(bool enable);

        /**
         * Returns the margins for the text.
         * @return The margins for the text.
         */
        TextMargins getTextMargins() const;
        /**
         * Sets the margins for the text. The margins will determine how much
         * empty space should surround the text. The default is TextMargins(0, 0, 0, 0).
         * @param textMargins The new margins for the text in dp.
         */
        void setTextMargins(const TextMargins& textMargins);

        /**
         * Returns the stroke color for the text label.
         * @return The stroke color for the text label.
         */
        Color getStrokeColor() const;
        /**
         * Sets the stroke color for the text label.
         * @param strokeColor The new stroke color for the text label.
         */
        void setStrokeColor(const Color& strokeColor);

        /**
         * Returns the stroke width for the text label.
         * @return The stroke width for the text label in screen density independent pixels.
         */
        float getStrokeWidth() const;
        /**
         * Sets the stroke width for the text label.
         * @param strokeWidth The new stroke width for the text label in screen density independent pixels.
         */
        void setStrokeWidth(float strokeWidth);

        /**
         * Returns the border color for the text label.
         * @return The border color for the text label.
         */
        Color getBorderColor() const;
        /**
         * Sets the border color for the text label.
         * @param borderColor The new border color for the text label. By default the border color is transparent.
         */
        void setBorderColor(const Color& borderColor);

        /**
         * Returns the border width for the text label.
         * @return The border width for the text label in screen density independent pixels.
         */
        float getBorderWidth() const;
        /**
         * Sets the border width for the text label.
         * @param borderWidth The new border width for the text label in screen density independent pixels. By default the border width is 0.
         */
        void setBorderWidth(float borderWidth);

        /**
         * Returns the background color for the text label.
         * @return The background color for the text label.
         */
        Color getBackgroundColor() const;
        /**
         * Sets the background color for the text label.
         * @param backgroundColor The new background color for the text label. By default the background color is transparent.
         */
        void setBackgroundColor(const Color& backgroundColor);

        /**
         * Builds a new instance of the TextStyle object using previously set parameters.
         * @return A new TextStyle object.
         */
        std::shared_ptr<TextStyle> buildStyle();

    protected:
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
