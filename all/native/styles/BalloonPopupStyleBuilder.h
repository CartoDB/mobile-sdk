/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BALLOONPOPUPSTYLEBUILDER_H_
#define _CARTO_BALLOONPOPUPSTYLEBUILDER_H_

#include "styles/PopupStyleBuilder.h"
#include "styles/BalloonPopupStyle.h"

#include <memory>

namespace carto {
    class Bitmap;

    /**
     * A builder class for BalloonPopupStyle.
     */
    class BalloonPopupStyleBuilder : public PopupStyleBuilder {
    public:
        /**
         * Constructs a BalloonPopupStyleBuilder object with all parameters set to defaults.
         */
        BalloonPopupStyleBuilder();
        virtual ~BalloonPopupStyleBuilder();
        
        /**
         * Returns the corner radius of the popup.
         * @return The corner radius of the popup in dp.
         */
        int getCornerRadius() const;
        /**
         * Sets the corner radius of the popup. Bigger values mean rounder corners, 0 creates
         * a rectangular popup. The default is 3.
         * @param cornerRadius The new corner radius in dp.
         */
        void setCornerRadius(int cornerRadius);
        
        /**
         * Returns the background color of the left part of the popup.
         * @return The background color of the left part of the popup.
         */
        Color getLeftColor() const;
        /**
         * Sets the background color of the left part of the popup. The size of the colored area
         * is determined by the left margins and the left image. The default is 0xFF00B483.
         * @param leftColor The new color for the left part of the popup.
         */
        void setLeftColor(const Color& leftColor);
        /**
         * Returns the image of the left part of the popup.
         * @return The image of the left part of the popup.
         */
        std::shared_ptr<Bitmap> getLeftImage() const;
        /**
         * Sets the image for the left part of the popup. If null is passed no image will be drawn.
         * The default is null.
         * @param leftImage The new image for the left part of the popup.
         */
        void setLeftImage(const std::shared_ptr<Bitmap>& leftImage);
        /**
         * Returns the margins of the left part of the popup.
         * @return The margins of the left part of the popup.
         */
        BalloonPopupMargins getLeftMargins() const;
        /**
         * Sets the margins for the left part of the popup. The margins will determine how much
         * empty space should surround the left image. The default is BalloonPopupMargins(5, 0, 5, 0).
         * @param leftMargins The new margins for the left part of the popup in dp.
         */
        void setLeftMargins(const BalloonPopupMargins& leftMargins);
        
        /**
         * Returns the background color of the right part of the popup.
         * @return The background color of the right part of the popup.
         */
        Color getRightColor() const;
        /**
         * Sets the background color of the right part of the popup. The size of the colored area
         * is determined by the right margins and the right image. The default is 0xFF00B483.
         * @param rightColor The new color for the right part of the popup.
         */
        void setRightColor(const Color& rightColor);
        /**
         * Returns the image of the right part of the popup.
         * @return The image of the right part of the popup.
         */
        std::shared_ptr<Bitmap> getRightImage() const;
        /**
         * Sets the image for the right part of the popup. If null is passed no image will be drawn.
         * The default is null.
         * @param rightImage The new image for the right part of the popup.
         */
        void setRightImage(const std::shared_ptr<Bitmap>& rightImage);
        /**
         * Returns the margins of the right part of the popup.
         * @return The margins of the right part of the popup.
         */
        BalloonPopupMargins getRightMargins() const;
        /**
         * Sets the margins for the right part of the popup. The margins will determine how much
         * empty space should surround the right image. The default is BalloonPopupMargins(5, 0, 5, 0).
         * @param rightMargins The new margins for the right part of the popup in dp.
         */
        void setRightMargins(const BalloonPopupMargins& rightMargins);
        
        /**
         * Returns the color of the title.
         * @return The color of the title.
         */
        Color getTitleColor() const;
        /**
         * Sets the color of the title. The default is 0xFF000000.
         * @param titleColor The new color for the title.
         */
        void setTitleColor(const Color& titleColor);
        /**
         * Returns the name of the title font.
         * @return The name of the title font.
         */
        std::string getTitleFontName() const;
        /**
         * Sets the name of the title font. It must be one of the fonts bundled with the platform.
         * The default is HelveticaNeue-Light.
         * @param titleFontName The new name for the title font.
         */
        void setTitleFontName(const std::string& titleFontName);
        /**
         * Returns the title field variable. If not empty, this variable is used to read actual text string from object meta info.
         * @return The current title variable.
         */
        std::string getTitleField() const;
        /**
         * Sets the title field variable. If not empty, this variable is used to read actual text string from object meta info.
         * @param field The text field to use for displaying title from metainfo.
         */
        void setTitleField(const std::string& field);
        /**
         * Returns the size of the title font.
         * @return The size of the title font.
         */
        int getTitleFontSize() const;
        /**
         * Sets the size of the title font. The default is 16.
         * @param titleFontSize The new size for the title font in pts.
         */
        void setTitleFontSize(int titleFontSize);
        /**
         * Returns the margins of the title.
         * @return The margins of the title.
         */
        BalloonPopupMargins getTitleMargins() const;
        /**
         * Sets the margins for the title. The margins will determine how much
         * empty space should surround the title. The default is BalloonPopupMargins(8, 4, 8, 0).
         * @param titleMargins The new margins for the title in dp.
         */
        void setTitleMargins(const BalloonPopupMargins& titleMargins);
        /**
         * Returns the state of the title wrap parameter.
         * @return True if long titles are wrapped, false if it is truncated.
         */
        bool isTitleWrap() const;
        /**
         * Sets the state of the title wrap parameter. If set to true, long titles that are
         * wider than the sreen width will be wrapped to multiple lines, increasing the height of the popup. 
         * If set to false, long titles are truncated with three dots.
         * @param titleWrap The new state of the title wrap parameter.
         */
        void setTitleWrap(bool titleWrap);
        
        /**
         * Returns the color of the description.
         * @return The color of the description.
         */
        Color getDescriptionColor() const;
        /**
         * Sets the color of the description. The default is 0xFF000000.
         * @param descColor The new color for the description.
         */
        void setDescriptionColor(const Color& descColor);
        /**
         * Returns the name of the description font.
         * @return The name of the description font.
         */
        std::string getDescriptionFontName() const;
        /**
         * Sets the name of the description font. It must be one of the fonts bundled with the platform.
         * The default is HelveticaNeue-Light.
         * @param descFontName The new name for the description font.
         */
        void setDescriptionFontName(const std::string& descFontName);
        /**
         * Returns the description field variable. If not empty, this variable is used to read actual text string from object meta info.
         * @return The current description variable.
         */
        std::string getDescriptionField() const;
        /**
         * Sets the description field variable. If not empty, this variable is used to read actual text string from object meta info.
         * @param field The description field to use for displaying text from metainfo.
         */
        void setDescriptionField(const std::string& field);
        /**
         * Returns the size of the description font.
         * @return The size of the description font.
         */
        int getDescriptionFontSize() const;
        /**
         * Sets the size of the description font. The default is 14.
         * @param descFontSize The new size for the description font in pts.
         */
        void setDescriptionFontSize(int descFontSize);
        /**
         * Returns the margins of the description.
         * @return The margins of the description.
         */
        BalloonPopupMargins getDescriptionMargins() const;
        /**
         * Sets the margins for the description. The margins will determine how much
         * empty space should surround the description. The default is BalloonPopupMargins(8, 4, 8, 0).
         * @param descMargins The new margins for the description in dp.
         */
        void setDescriptionMargins(const BalloonPopupMargins& descMargins);
        /**
         * Returns the state of the description wrap parameter.
         * @return True if long descriptions are wrapped, false if it is truncated.
         */
        bool isDescriptionWrap() const;
        /**
         * Sets the state of the description wrap parameter. If set to true, long descriptions that are
         * wider than the sreen width will be wrapped to multiple lines, increasing the height of the popup.
         * If set to false, long descriptions are truncated with three dots.
         * @param descWrap The new state of the description wrap parameter.
         */
        void setDescriptionWrap(bool descWrap);
        
        /**
         * Returns the color of the stroke surrounding the popup.
         * @return The color of the stroke surrounding the popup.
         */
        Color getStrokeColor() const;
        /**
         * Sets the color of the stroke surrounding the popup. The default is 0xFF000000.
         * @param strokeColor The new color of the stroke surrounding the popup.
         */
        void setStrokeColor(const Color& strokeColor);
        /**
         * Returns the width of the stroke surrounding the popup.
         * @return The width of the stroke surrounding the popup in dp.
         */
        int getStrokeWidth() const;
        /**
         * Sets the width of the stroke surrounding the popup. The default is 1.
         * @param strokeWidth The new width of the stroke surrounding the popup.
         */
        void setStrokeWidth(int strokeWidth);
        
        /**
         * Returns the width of the triangle at the bottom of the popup.
         * @return The width of the triangle at the bottom of the popup in dp.
         */
        int getTriangleWidth() const;
        /**
         * Sets the width of the triangle at the bottom of the popup. The default is 20.
         * @param triangleWidth The new width of the triangle at the bottom of the popup.
         */
        void setTriangleWidth(int triangleWidth);
        /**
         * Returns the height of the triangle at the bottom of the popup.
         * @return The height of the triangle at the bottom of the popup in dp.
         */
        int getTriangleHeight() const;
        /**
         * Sets the height of the triangle at the bottom of the popup. The default is 10.
         * @param triangleHeight The new height of the triangle at the bottom of the popup.
         */
        void setTriangleHeight(int triangleHeight);

        /**
         * Builds a new instance of the BalloonPopupStyle object using previously set parameters.
         * @return A new BalloonPopupStyle object.
         */
        std::shared_ptr<BalloonPopupStyle> buildStyle();

    protected:
        int _cornerRadius;
        
        Color _leftColor;
        std::shared_ptr<Bitmap> _leftImage;
        BalloonPopupMargins _leftMargins;
        
        Color _rightColor;
        std::shared_ptr<Bitmap> _rightImage;
        BalloonPopupMargins _rightMargins;
        
        Color _titleColor;
        std::string _titleFontName;
        std::string _titleField;
        int _titleFontSize;
        BalloonPopupMargins _titleMargins;
        bool _titleWrap;
        
        Color _descColor;
        std::string _descFontName;
        std::string _descField;
        int _descFontSize;
        BalloonPopupMargins _descMargins;
        bool _descWrap;
        
        Color _strokeColor;
        int _strokeWidth;
        
        int _triangleWidth;
        int _triangleHeight;
    };

}

#endif
