#include "VTBitmapLoader.h"
#include "core/BinaryData.h"
#include "graphics/Bitmap.h"
#include "utils/AssetPackage.h"
#include "utils/FileUtils.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <cstdint>

#define NANOSVG_IMPLEMENTATION
#include <nanosvg.h>
#define NANOSVGRAST_IMPLEMENTATION
#include <nanosvgrast.h>

namespace carto {
    
    VTBitmapLoader::VTBitmapLoader(const std::string& basePath, const std::shared_ptr<AssetPackage>& assetPackage) :
        _basePath(basePath),
        _assetPackage(assetPackage),
        _urlFileLoader("VTBitmapLoader", true)
    {
    }

    VTBitmapLoader::~VTBitmapLoader() {
    }
    
    std::shared_ptr<const vt::Bitmap> VTBitmapLoader::load(const std::string& url, float& resolution) const {
        std::shared_ptr<BinaryData> fileData;
        if (!_urlFileLoader.loadFile(url, fileData)) {
            std::string fileName = FileUtils::NormalizePath(_basePath + url);
            if (_assetPackage) {
                fileData = _assetPackage->loadAsset(fileName);
            }
            if (!fileData) {
                Log::Errorf("VTBitmapLoader: Failed to load bitmap: %s", fileName.c_str());
            }
        }
        if (!fileData) {
            return std::shared_ptr<vt::Bitmap>();
        }

        std::string::size_type extEndPos = url.rfind('?');
        std::string::size_type extBegPos = url.rfind('.', extEndPos);
        if (extBegPos != std::string::npos) {
            std::string ext = url.substr(extBegPos, extEndPos - extBegPos);
            if (ext == ".svg") {
                return loadSVG(*fileData->getDataPtr(), resolution);
            }
        }
        
        std::shared_ptr<Bitmap> sourceBitmap = Bitmap::CreateFromCompressed(fileData->data(), fileData->size());
        if (!sourceBitmap) {
            Log::Errorf("VTBitmapLoader: Failed to decode bitmap: %s", url.c_str());
            return std::shared_ptr<vt::Bitmap>();                
        }
        sourceBitmap = sourceBitmap->getRGBABitmap();

        std::vector<std::uint32_t> data(sourceBitmap->getWidth() * sourceBitmap->getHeight());
        for (unsigned int y = 0; y < sourceBitmap->getHeight(); y++) {
            int dataOffset = (sourceBitmap->getHeight() - 1 - y) * sourceBitmap->getWidth();
            int bitmapOffset = y * sourceBitmap->getWidth() * sourceBitmap->getBytesPerPixel();
            for (unsigned int x = 0; x < sourceBitmap->getWidth(); x++) {
                data[dataOffset++] = *reinterpret_cast<const std::uint32_t*>(&sourceBitmap->getPixelData()[bitmapOffset]);
                bitmapOffset += 4;
            }
        }

        resolution = 1.0f; // reset resolution
        return std::make_shared<vt::Bitmap>(sourceBitmap->getWidth(), sourceBitmap->getHeight(), std::move(data));
    }
    
    std::shared_ptr<const vt::Bitmap> VTBitmapLoader::loadSVG(const std::vector<unsigned char>& fileData, float& resolution) const {
        std::string source(reinterpret_cast<const char*>(fileData.data()), fileData.size());
        
        std::shared_ptr<NSVGimage> image(nsvgParse(const_cast<char*>(source.c_str()), "px", Const::UNSCALED_DPI * resolution), nsvgDelete);
        if (!image) {
            return std::shared_ptr<vt::Bitmap>();
        }
        
        int width = static_cast<int>(image->width * resolution);
        int height = static_cast<int>(image->height * resolution);

        std::shared_ptr<NSVGrasterizer> rast(nsvgCreateRasterizer(), nsvgDeleteRasterizer);
        if (!rast) {
            return std::shared_ptr<vt::Bitmap>();
        }

        std::vector<std::uint32_t> data(width * height);
        nsvgRasterize(rast.get(), image.get(), 0, 0, resolution, reinterpret_cast<unsigned char*>(data.data()), width, height, width * sizeof(std::uint32_t), 1);
        return std::make_shared<vt::Bitmap>(width, height, std::move(data));
    }

}
