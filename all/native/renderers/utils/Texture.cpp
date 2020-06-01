#include "Texture.h"
#include "graphics/Bitmap.h"
#include "renderers/utils/GLResourceManager.h"
#include "utils/Log.h"
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
        
    std::size_t Texture::getSize() const {
        return _sizeInBytes;
    }
        
    const cglib::vec2<float>& Texture::getTexCoordScale() const {
        return _texCoordScale;
    }

    GLuint Texture::getTexId() const {
        return _texId;
    }

    Texture::Texture(const std::weak_ptr<GLResourceManager>& manager, const std::shared_ptr<Bitmap>& bitmap, bool genMipmaps, bool repeat) :
        GLResource(manager),
        _bitmap(bitmap),
        _mipmaps(genMipmaps),
        _repeat(repeat),
        _sizeInBytes(0),
        _texCoordScale(1.0f, 1.0f),
        _texId(0)
    {
        bool npot = !GeneralUtils::IsPow2(bitmap->getWidth()) || !GeneralUtils::IsPow2(bitmap->getHeight());
        if (npot && !GLContext::TEXTURE_NPOT_REPEAT) {
            if (repeat) {
                int pow2Width  = GeneralUtils::UpperPow2(bitmap->getWidth());
                int pow2Height = GeneralUtils::UpperPow2(bitmap->getHeight());
                _bitmap = _bitmap->getResizedBitmap(pow2Width, pow2Height);
            } else if (genMipmaps) {
                int xPadding = GeneralUtils::UpperPow2(bitmap->getWidth())  - bitmap->getWidth();
                int yPadding = GeneralUtils::UpperPow2(bitmap->getHeight()) - bitmap->getHeight();
                _bitmap = _bitmap->getPaddedBitmap(xPadding, -yPadding);
                _texCoordScale = cglib::vec2<float>(static_cast<float>(bitmap->getWidth()) / _bitmap->getWidth(), static_cast<float>(bitmap->getHeight()) / _bitmap->getHeight());
            }
        }

        _sizeInBytes = static_cast<std::size_t>((_mipmaps ? MIPMAP_SIZE_MULTIPLIER : 1.0) * _bitmap->getWidth() * _bitmap->getHeight() * _bitmap->getBytesPerPixel());
    }

    void Texture::create() {
        if (_texId == 0) {
            _texId = LoadFromBitmap(*_bitmap, _mipmaps, _repeat);

            GLContext::CheckGLError("Texture::create");
        }
    }

    void Texture::destroy() {
        if (_texId != 0) {
            glDeleteTextures(1, &_texId);
            _texId = 0;

            GLContext::CheckGLError("Texture::destroy");
        }
    }
    
    GLuint Texture::LoadFromBitmap(const Bitmap& bitmap, bool genMipmaps, bool repeat) {
        if (bitmap.getColorFormat() == ColorFormat::COLOR_FORMAT_UNSUPPORTED) {
            Log::Error("Texture::loadFromBitmap: Failed to create texture from bitmap, unsupported color format");
            return 0;
        }

        GLint oldTexId = 0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexId);
        
        GLuint texId = 0;
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);
    
        const std::vector<unsigned char>& pixelData = bitmap.getPixelData();
        glTexImage2D(GL_TEXTURE_2D, 0, bitmap.getColorFormat(), bitmap.getWidth(), bitmap.getHeight(),
                0, bitmap.getColorFormat(), GL_UNSIGNED_BYTE, pixelData.data());
        
        if (repeat) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
    
        bool npot = !GeneralUtils::IsPow2(bitmap.getWidth()) || !GeneralUtils::IsPow2(bitmap.getHeight());
        if (!genMipmaps || (npot && !GLContext::TEXTURE_NPOT_MIPMAPS)) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    
            if (GLContext::TEXTURE_FILTER_ANISOTROPIC) {
                GLint deviceMaxAnisotropy = 0;
                glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &deviceMaxAnisotropy);
                if (deviceMaxAnisotropy > 1) {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(MAX_ANISOTROPY, deviceMaxAnisotropy));
                }
            }
    
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        glBindTexture(GL_TEXTURE_2D, oldTexId);
    
        return texId;
    }
        
    const int Texture::MAX_ANISOTROPY = 8;
        
    const double Texture::MIPMAP_SIZE_MULTIPLIER = 1.33;
    
}
