/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VT_BITMAPMANAGER_H_
#define _CARTO_VT_BITMAPMANAGER_H_

#include "Bitmap.h"

#include <memory>
#include <string>
#include <map>
#include <mutex>

namespace carto { namespace vt {
    class BitmapManager {
    public:
        class BitmapLoader {
        public:
            virtual ~BitmapLoader() = default;

            virtual std::shared_ptr<Bitmap> load(const std::string& fileName) const = 0;
        };

        explicit BitmapManager(const std::shared_ptr<BitmapLoader>& loader);
        virtual ~BitmapManager() = default;

        std::shared_ptr<const Bitmap> getBitmap(const std::string& fileName) const;
        std::shared_ptr<const Bitmap> loadBitmap(const std::string& fileName);
        void storeBitmap(const std::string& fileName, const std::shared_ptr<const Bitmap>& bitmap);

        std::shared_ptr<const BitmapPattern> getBitmapPattern(const std::string& fileName) const;
        std::shared_ptr<const BitmapPattern> loadBitmapPattern(const std::string& fileName, float widthScale, float heightScale);
        void storeBitmapPattern(const std::string& fileName, const std::shared_ptr<const BitmapPattern>& bitmapPattern);

        static std::shared_ptr<const Bitmap> scale(const std::shared_ptr<const Bitmap>& bitmap, int width, int height);
        static std::shared_ptr<const Bitmap> scaleToPOT(const std::shared_ptr<const Bitmap>& bitmap);

    protected:
        std::shared_ptr<BitmapLoader> _bitmapLoader;
        std::map<std::string, std::shared_ptr<const Bitmap>> _bitmapMap;
        std::map<std::string, std::shared_ptr<const BitmapPattern>> _bitmapPatternMap;

        mutable std::mutex _mutex;
    };
} }

#endif