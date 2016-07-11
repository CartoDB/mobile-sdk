/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BITMAPUTILS_H_
#define _CARTO_BITMAPUTILS_H_

#include <memory>
#include <string>

#include <jni.h>

namespace carto {
    class Bitmap;

    /**
     * A helper class for loading bitmaps and converting Bitmaps to Android Bitmaps an vice versa.
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
         * Creates a new Bitmap object from an existing Android Bitmap.
         * @param androidBitmap The reference Android bitmap.
         * @return The created bitmap.
         */
        static std::shared_ptr<Bitmap> CreateBitmapFromAndroidBitmap(jobject androidBitmap);
        
        /**
         * Creates a new Android Bitmap from an existing Bitmap.
         * @param bitmap The existing bitmap.
         * @return The android Bitmap.
         */
        static jobject CreateAndroidBitmapFromBitmap(const std::shared_ptr<Bitmap>& bitmap);

    protected:
        BitmapUtils();
    };

}

#endif
