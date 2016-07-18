/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BALLOONPOPUPSTYLE_H_
#define _CARTO_BALLOONPOPUPSTYLE_H_

#include "styles/PopupStyle.h"

#include <string>

namespace carto {
    class Bitmap;

    /**
     * Margins info for balloon popup.
     */
    class BalloonPopupMargins {
    public:
        BalloonPopupMargins(int left, int top, int right, int bottom);
        virtual ~BalloonPopupMargins();
        
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
     * A style for balloon popups. Contains attributes for configuring how the balloon popup is drawn on the screen.
     */
    class BalloonPopupStyle : public PopupStyle {
    public:
        /**
         * Constructs a BalloonPopupStyle object from various parameters. Instantiating the object directly is
         * not recommended, BalloonPopupStyleBuilder should be used instead.
         * @param color The color for the popup.
         * @param attachAnchorPointX The horizontal attaching anchor point for the popup.
         * @param attachAnchorPointY The vertical attaching anchor point for the popup.
         * @param causesOverlap The causes overlap flag for the billboard.
         * @param hideIfOverlapped The hide if overlapped flag for the billboard.
         * @param horizontalOffset The horizontal offset for the popup.
         * @param verticalOffset The vertical offset for the popup.
         * @param placementPriority The placement priority for the popup.
         * @param scaleWithDPI The scale with DPI flag for the popup.
         * @param cornerRadius The corner radius of the popup in dp.
         * @param leftColor The background color of the left part of the popup.
         * @param leftImage The image of the left part of the popup.
         * @param leftMargins The margins of the left part of the popup.
         * @param rightColor The background color of the right part of the popup.
         * @param rightImage The margins of the right part of the popup.
         * @param rightMargins The margins of the right part of the popup.
         * @param titleColor The color of the title.
         * @param titleFontName The name of the title font.
         * @param titleField The metadata field name for title.
         * @param titleFontSize The size of the title font.
         * @param titleMargins The margins of the title.
         * @param titleWrap True if long titles are wrapped, false if it is truncated.
         * @param descColor The color of the description.
         * @param descFontName The name of the description font.
         * @param descField The metadata field for description.
         * @param descFontSize The size of the description font.
         * @param descMargins The margins of the description.
         * @param descWrap True if long descriptions are wrapped, false if it is truncated.
         * @param strokeColor The color of the stroke surrounding the popup.
         * @param strokeWidth The width of the stroke surrounding the popup in dp.
         * @param triangleWidth The width of the triangle at the bottom of the popup in dp.
         * @param triangleHeight The height of the triangle at the bottom of the popup in dp.
         */
        BalloonPopupStyle(const Color& color,
                          float attachAnchorPointX,
                          float attachAnchorPointY,
                          bool causesOverlap,
                          bool hideIfOverlapped,
                          float horizontalOffset,
                          float verticalOffset,
                          int placementPriority,
                          bool scaleWithDPI,
                          int cornerRadius,
                          const Color& leftColor,
                          const std::shared_ptr<Bitmap>& leftImage,
                          const BalloonPopupMargins& leftMargins,
                          const Color& rightColor,
                          const std::shared_ptr<Bitmap>& rightImage,
                          BalloonPopupMargins& rightMargins,
                          const Color& titleColor,
                          const std::string& titleFontName,
                          const std::string& titleField,
                          int titleFontSize,
                          const BalloonPopupMargins& titleMargins,
                          bool titleWrap,
                          const Color& descColor,
                          const std::string& descFontName,
                          const std::string& descField,
                          int descFontSize,
                          const BalloonPopupMargins& descMargins,
                          bool descWrap,
                          const Color& strokeColor,
                          int strokeWidth,
                          int triangleWidth,
                          int triangleHeight);
    	virtual ~BalloonPopupStyle();

        /**
         * Returns the background color of the popup.
         * @return The background color of the popup.
         */
        const Color& getBackgroundColor() const;
        /**
         * Returns the corner radius of the popup.
         * @return The corner radius of the popup.
         */
        int getCornerRadius() const;
        
        /**
         * Returns the background color of the left part of the popup.
         * @return The background color of the left part of the popup.
         */
        const Color& getLeftColor() const;
        /**
         * Returns the image of the left part of the popup.
         * @return The image of the left part of the popup.
         */
        const std::shared_ptr<Bitmap>& getLeftImage() const;
        /**
         * Returns the margins of the left part of the popup.
         * @return The margins of the left part of the popup.
         */
        const BalloonPopupMargins& getLeftMargins() const;
        
        /**
         * Returns the background color of the right part of the popup.
         * @return The background color of the right part of the popup.
         */
        const Color& getRightColor() const;
        /**
         * Returns the image of the right part of the popup.
         * @return The image of the right part of the popup.
         */
        const std::shared_ptr<Bitmap>& getRightImage() const;
        /**
         * Returns the margins of the right part of the popup.
         * @return The margins of the right part of the popup.
         */
        const BalloonPopupMargins& getRightMargins() const;
        
        /**
         * Returns the color of the title.
         * @return The color of the title.
         */
        const Color& getTitleColor() const;
        /**
         * Returns the name of the title font.
         * @return The name of the title font.
         */
        const std::string& getTitleFontName() const;
        /**
         * Returns the title field variable to use.
         * @return The title field variable.
         */
        const std::string& getTitleField() const;
        /**
         * Returns the size of the title font.
         * @return The size of the title font.
         */
        int getTitleFontSize() const;
        /**
         * Returns the margins of the title.
         * @return The margins of the title.
         */
        const BalloonPopupMargins& getTitleMargins() const;
        /**
         * Returns the state of the title wrap parameter.
         * @return True if long titles are wrapped, false if it is truncated.
         */
        bool isTitleWrap() const;
        
        /**
         * Returns the color of the description.
         * @return The color of the description.
         */
        const Color& getDescriptionColor() const;
        /**
         * Returns the name of the description font.
         * @return The name of the description font.
         */
        const std::string& getDescriptionFontName() const;
        /**
         * Returns the description field variable to use.
         * @return The description field variable.
         */
        const std::string& getDescriptionField() const;
        /**
         * Returns the size of the description font.
         * @return The size of the description font.
         */
        int getDescriptionFontSize() const;
        /**
         * Returns the margins of the description.
         * @return The margins of the description.
         */
        const BalloonPopupMargins& getDescriptionMargins() const;
        /**
         * Returns the state of the description wrap parameter.
         * @return True if long descriptions are wrapped, false if it is truncated.
         */
        bool isDescriptionWrap() const;
        
        /**
         * Returns the color of the stroke surrounding the popup.
         * @return The color of the stroke surrounding the popup.
         */
        const Color& getStrokeColor() const;
        /**
         * Returns the width of the stroke surrounding the popup.
         * @return The width of the stroke surrounding the popup in dp.
         */
        int getStrokeWidth() const;
        
        /**
         * Returns the width of the triangle at the bottom of the popup.
         * @return The width of the triangle at the bottom of the popup in dp.
         */
        int getTriangleWidth() const;
        /**
         * Returns the height of the triangle at the bottom of the popup.
         * @return The height of the triangle at the bottom of the popup in dp.
         */
        int getTriangleHeight() const;

    protected:
        Color _backgroundColor;
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
