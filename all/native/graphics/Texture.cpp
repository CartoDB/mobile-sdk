#include "Texture.h"
#include "graphics/TextureManager.h"
#include "graphics/Bitmap.h"
#include "utils/Log.h"
#include "utils/GLUtils.h"
#include "utils/GeneralUtils.h"

#include <algorithm>

namespace carto {

    Texture::~Texture() {
    }
    
    bool Texture::isMipmaps() const {
        return _mipmaps;
    }
    
    bool Texture::isRepeat() const {
        return _repeat;
    }
        
    unsigned int Texture::getSize() const {
        return _sizeInBytes;
    }
        
    const cglib::vec2<float>& Texture::getTexCoordScale() const {
        return _texCoordScale;
    }
    
    GLuint Texture::getTexId() const {
        if (std::this_thread::get_id() != _textureManager->getGLThreadId()) {
            Log::Warn("Texture::getTexId: Method called from wrong thread!");
            return 0;
        }

        load();
        return _texId;
    }

    Texture::Texture(const std::shared_ptr<TextureManager>& textureManager, const std::shared_ptr<Bitmap>& bitmap, bool genMipmaps, bool repeat) :
        _bitmap(bitmap),
        _mipmaps(genMipmaps),
        _repeat(repeat),
        _sizeInBytes(0),
        _texCoordScale(1.0f, 1.0f),
        _texId(0),
        _textureManager(textureManager)
    {
        bool npot = !GeneralUtils::IsPow2(bitmap->getWidth()) || !GeneralUtils::IsPow2(bitmap->getHeight());
        if (npot && !GLUtils::isTextureNPOTRepeat()) {
            if (repeat) {
                int pow2Width  = GeneralUtils::UpperPow2(bitmap->getWidth());
                int pow2Height = GeneralUtils::UpperPow2(bitmap->getHeight());
                _bitmap = _bitmap->getResizedBitmap(pow2Width, pow2Height);
            }
            else if (genMipmaps) {
                int xPadding = GeneralUtils::UpperPow2(bitmap->getWidth())  - bitmap->getWidth();
                int yPadding = GeneralUtils::UpperPow2(bitmap->getHeight()) - bitmap->getHeight();
                _bitmap = _bitmap->getPaddedBitmap(xPadding, -yPadding);
                _texCoordScale = cglib::vec2<float>(static_cast<float>(bitmap->getWidth()) / _bitmap->getWidth(), static_cast<float>(bitmap->getHeight()) / _bitmap->getHeight());
            }
        }

        _sizeInBytes = static_cast<int>(_bitmap->getWidth() * _bitmap->getHeight() * _bitmap->getBytesPerPixel() * (_mipmaps ? MIPMAP_SIZE_MULTIPLIER : 1.0f));
    }

    void Texture::load() const {
        if (_texId == 0) {
            _texId = loadFromBitmap(*_bitmap, _mipmaps, _repeat);
        }
    }

    void Texture::unload() const {
        if (_texId != 0) {
            glDeleteTextures(1, &_texId);
            _texId = 0;

            GLUtils::checkGLError("Texture::unload()");
        }
    }
    
    GLuint Texture::loadFromBitmap(const Bitmap& bitmap, bool genMipmaps, bool repeat) const {
        if (bitmap.getColorFormat() == ColorFormat::COLOR_FORMAT_UNSUPPORTED) {
            Log::Error("Texture::loadFromBitmap: Failed to create texture from bitmap, unsupported color format");
            return 0;
        }
        
        GLuint texId = 0;
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);
    
        const std::vector<unsigned char>& pixelData = bitmap.getPixelData();
        const unsigned char* pixelDataPtr = &pixelData[0];
        glTexImage2D(GL_TEXTURE_2D, 0, bitmap.getColorFormat(), bitmap.getWidth(), bitmap.getHeight(),
                0, bitmap.getColorFormat(), GL_UNSIGNED_BYTE, pixelDataPtr);
        
        if (repeat) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
    
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
        if (!genMipmaps) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    
            if (GLUtils::isTextureFilterAnisotropic()) {
                GLint deviceMaxAnisotropy;
                glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &deviceMaxAnisotropy);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(MAX_ANISOTROPY, deviceMaxAnisotropy));
            }
    
            glGenerateMipmap(GL_TEXTURE_2D);
        }
    
        GLUtils::checkGLError("Texture::loadFromBitmap()");
    
        return texId;
    }
        
    const int Texture::MAX_ANISOTROPY = 4;
        
    const float Texture::MIPMAP_SIZE_MULTIPLIER = 1.33f;
    
}
