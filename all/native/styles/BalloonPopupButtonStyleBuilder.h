/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BALLOONPOPUPBUTTONSTYLEBUILDER_H_
#define _CARTO_BALLOONPOPUPBUTTONSTYLEBUILDER_H_

#include "styles/StyleBuilder.h"
#include "styles/BalloonPopupButtonStyle.h"

#include <memory>

namespace carto {

    /**
     * A builder class for BalloonPopupButtonStyle.
     */
    class BalloonPopupButtonStyleBuilder : public StyleBuilder {
    public:
        /**
         * Constructs a BalloonPopupButtonStyleBuilder object with all parameters set to defaults.
         */
        BalloonPopupButtonStyleBuilder();
        virtual ~BalloonPopupButtonStyleBuilder();
        
        /**
         * Returns the width of the button.
         * If this value is -1, then button width is calculated automatically based on button text.
         * @return The width of the button in dp.
         */
        int getButtonWidth() const;
        /**
         * Sets the width of the button.
         * The default is -1, which means that button width is calculated automatically based on button text.
         * @param buttonWidth The button width in dp, or -1.
         */
        void setButtonWidth(int buttonWidth);

        /**
         * Returns the corner radius of the button.
         * @return The corner radius of the button in dp.
         */
        int getCornerRadius() const;
        /**
         * Sets the corner radius of the button. Bigger values mean rounder corners, 0 creates
         * a rectangular button. The default is 3.
         * @param cornerRadius The new corner radius in dp.
         */
        void setCornerRadius(int cornerRadius);
        
        /**
         * Returns the color of the text.
         * @return The color of the text.
         */
        Color getTextColor() const;
        /**
         * Sets the color of the text. The default is 0xFF000000.
         * @param textColor The new color for the text.
         */
        void setTextColor(const Color& textColor);
        /**
         * Returns the name of the text font.
         * @return The name of the text font.
         */
        std::string getTextFontName() const;
        /**
         * Sets the name of the text font. It must be one of the fonts bundled with the platform.
         * The default is HelveticaNeue-Light.
         * @param textFontName The new name for the text font.
         */
        void setTextFontName(const std::string& textFontName);
        /**
         * Returns the size of the text font.
         * @return The size of the text font.
         */
        int getTextFontSize() const;
        /**
         * Sets the size of the text font. The default is 16.
         * @param textFontSize The new size for the text font in pts.
         */
        void setTextFontSize(int textFontSize);
        /**
         * Returns the margins of the text.
         * @return The margins of the text.
         */
        BalloonPopupMargins getTextMargins() const;
        /**
         * Sets the margins for the text. The margins will determine how much
         * empty space should surround the text. The default is BalloonPopupMargins(8, 4, 8, 4).
         * @param textMargins The new margins for the text in dp.
         */
        void setTextMargins(const BalloonPopupMargins& textMargins);
        
        /**
         * Returns the color of the stroke surrounding the button.
         * @return The color of the stroke surrounding the button.
         */
        Color getStrokeColor() const;
        /**
         * Sets the color of the stroke surrounding the button. The default is 0xFF000000.
         * @param strokeColor The new color of the stroke surrounding the button.
         */
        void setStrokeColor(const Color& strokeColor);
        /**
         * Returns the width of the stroke surrounding the button.
         * @return The width of the stroke surrounding the button in dp.
         */
        int getStrokeWidth() const;
        /**
         * Sets the width of the stroke surrounding the button. The default is 1.
         * @param strokeWidth The new width of the stroke surrounding the button.
         */
        void setStrokeWidth(int strokeWidth);
        
        /**
         * Builds a new instance of the BalloonPopupButtonStyle object using previously set parameters.
         * @return A new BalloonPopupButtonStyle object.
         */
        std::shared_ptr<BalloonPopupButtonStyle> buildStyle();

    protected:
        int _buttonWidth;
        int _cornerRadius;
        
        Color _textColor;
        std::string _textFontName;
        int _textFontSize;
        BalloonPopupMargins _textMargins;
        
        Color _strokeColor;
        int _strokeWidth;
    };

}

#endif
