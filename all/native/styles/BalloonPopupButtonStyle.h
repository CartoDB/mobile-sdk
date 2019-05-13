/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BALLOONPOPUPBUTTONSTYLE_H_
#define _CARTO_BALLOONPOPUPBUTTONSTYLE_H_

#include "styles/BalloonPopupStyle.h"

#include <string>

namespace carto {

    /**
     * A style for balloon popup buttons. Contains attributes for configuring how the balloon popup is drawn on the screen.
     */
    class BalloonPopupButtonStyle : public Style {
    public:
        /**
         * Constructs a BalloonPopupButtonStyle object from various parameters. Instantiating the object directly is
         * not recommended, BalloonPopupButtonStyleBuilder should be used instead.
         * @param color The color for the button.
         * @param buttonWidth The width of the button in dp.
         * @param cornerRadius The corner radius of the button in dp.
         * @param textColor The color of the text.
         * @param textFontName The name of the text font.
         * @param textFontSize The size of the text font.
         * @param textMargins The margins of the text.
         * @param strokeColor The color of the stroke surrounding the button.
         * @param strokeWidth The width of the stroke surrounding the button in dp.
         */
        BalloonPopupButtonStyle(const Color& color,
                                int buttonWidth,
                                int cornerRadius,
                                const Color& textColor,
                                const std::string& textFontName,
                                int textFontSize,
                                const BalloonPopupMargins& textMargins,
                                const Color& strokeColor,
                                int strokeWidth);
        virtual ~BalloonPopupButtonStyle();

        /**
         * Returns the background color of the button.
         * @return The background color of the button.
         */
        const Color& getBackgroundColor() const;
        /**
         * Returns the width of the button.
         * If this value is -1, then button width is calculated automatically based on button text.
         * @return The width of the button in dp.
         */
        int getButtonWidth() const;
        /**
         * Returns the corner radius of the button.
         * @return The corner radius of the button.
         */
        int getCornerRadius() const;
        
        /**
         * Returns the color of the text.
         * @return The color of the text.
         */
        const Color& getTextColor() const;
        /**
         * Returns the name of the text font.
         * @return The name of the text font.
         */
        const std::string& getTextFontName() const;
        /**
         * Returns the size of the text font.
         * @return The size of the text font.
         */
        int getTextFontSize() const;
        /**
         * Returns the margins of the text.
         * @return The margins of the text.
         */
        const BalloonPopupMargins& getTextMargins() const;
        
        /**
         * Returns the color of the stroke surrounding the button.
         * @return The color of the stroke surrounding the button.
         */
        const Color& getStrokeColor() const;
        /**
         * Returns the width of the stroke surrounding the button.
         * @return The width of the stroke surrounding the button in dp.
         */
        int getStrokeWidth() const;
        
    protected:
        Color _backgroundColor;
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
