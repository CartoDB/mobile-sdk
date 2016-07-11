/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BITMAPUTILS_H_
#define _CARTO_BITMAPUTILS_H_

#include <memory>
#include <string>

#import <UIKit/UIKit.h>

namespace carto {
    class Bitmap;

    /**
     * A helper class for loading bitmaps and converting Bitmaps to UIImages an vice versa.
     */
    class BitmapUtils {
    public:
        /**
         * Loads the specified bitmap asset bundled with the application.
         * @param assetPath The asset path to the image to be loaded.
         * @return The loaded bitmap.
         */
        static std::shared_ptr<Bitmap> LoadBitmapFromAssets(const std::string& assetPath);
        
        /**
         * Loads bitmap from specified file.
         * @param filePath The path to the image to be loaded.
         * @return The loaded bitmap.
         */
        static std::shared_ptr<Bitmap> LoadBitmapFromFile(const std::string& filePath);
        
        /**
         * Creates a new Bitmap object from an existing UIImage.
         * @param image The reference UIImage.
         * @return The created bitmap.
         */
        static std::shared_ptr<Bitmap> CreateBitmapFromUIImage(const UIImage* image);
    	
        /**
         * Creates a new UIImage object from an existing Bitmap.
         * @param bitmap The reference bitmap.
         * @return The created UIImage.
         */
        static UIImage* CreateUIImageFromBitmap(const std::shared_ptr<Bitmap>& bitmap);

    private:
         BitmapUtils();
    };

}

#endif
