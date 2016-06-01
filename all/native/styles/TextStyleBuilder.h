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
        int getFontSize() const;
        /**
         * Sets the font size for the text label.
         * @param size The new font size for the text label in points.
         */
        void setFontSize(int size);

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
         * Builds a new instance of the TextStyle object using previously set parameters.
         * @return A new TextStyle object.
         */
        std::shared_ptr<TextStyle> buildStyle();

    protected:
        std::string _fontName;
        
        std::string _textField;

        int _fontSize;
        
        Color _strokeColor;

        float _strokeWidth;
    };

}

#endif
