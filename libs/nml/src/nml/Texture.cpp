#include "Texture.h"

#include "nmlpackage/NMLPackage.pb.h"

#include "rg_etc1.h"

#include "PVRTTexture.h"
#include "PVRTDecompress.h"

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#ifndef GL_TEXTURE_MAX_LEVEL
#define GL_TEXTURE_MAX_LEVEL 0x813D
#endif

#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif

#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

#ifndef GL_ETC1_RGB8_OES
#define GL_ETC1_RGB8_OES 0x8D64
#endif

#ifndef GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG 0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG 0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG 0x8C03
#endif

#include <cassert>
#include <mutex>
#include <memory>
#include <string>
#include <sstream>
#include <unordered_set>

namespace carto { namespace nmlgl {

    Texture::Texture(std::shared_ptr<nml::Texture> texture) :
        _refCount(0),
        _texture(texture),
        _glTextureId(0),
        _glContext(),
        _glSize(0)
    {
    }
    
    void Texture::create(const std::shared_ptr<GLContext>& gl) {
        if (_refCount++ > 0) {
            return;
        }
    
        uploadTexture(gl);
        _glContext = gl;
    }
    
    void Texture::dispose(const std::shared_ptr<GLContext>& gl)	{
        if (--_refCount > 0) {
            return;
        }
    
        if (_glContext.lock() == gl) {
            if (_glTextureId != 0) {
                glDeleteTextures(1, &_glTextureId);
            }
        }
        _glContext.reset();
        _glTextureId = 0;
    }
    
    void Texture::bind(const std::shared_ptr<GLContext>& gl) {
        if (_glContext.lock() != gl) {
            uploadTexture(gl);
            _glContext = gl;
        }
        glBindTexture(GL_TEXTURE_2D, _glTextureId);
    }
    
    int Texture::getTextureSize() const {
        if (!_texture)
            return 0;
    
        int size = 0;
        for (int i = 0; i < _texture->mipmaps_size(); i++) {
            size += _texture->mipmaps(i).size();
        }
        return size;
    }
    
    GLuint Texture::getSamplerWrapMode(int wrapMode) {
        switch (wrapMode) {
        case nml::Sampler::CLAMP:
            return GL_CLAMP_TO_EDGE;
        default:
            return GL_REPEAT; // ignore MIRROR, etc
        }
    }

    bool Texture::hasGLExtension(const char* ext) {
        static std::mutex mutex;
        static std::shared_ptr<std::unordered_set<std::string> > extensions;

        std::lock_guard<std::mutex> lock(mutex);
        if (!extensions) {
            extensions = std::make_shared<std::unordered_set<std::string> >();

            const char* extensionsString = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
            if (!extensionsString) {
                return false;
            }
            std::stringstream ss(extensionsString);
            std::string s;
            while (getline(ss, s, ' ')) {
                extensions->insert(s);
            }
        }
        return extensions->find(ext) != extensions->end();
    }
    
    void Texture::updateSampler(const std::shared_ptr<GLContext>& gl, bool hasSampler, nml::Sampler sampler, bool complete) {
        if (hasSampler) {
            switch (sampler.filter()) {
            case nml::Sampler::NEAREST:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                break;
            case nml::Sampler::BILINEAR:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                break;
            case nml::Sampler::TRILINEAR:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, complete ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                break;
            }
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, getSamplerWrapMode(sampler.wrap_s()));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, getSamplerWrapMode(sampler.wrap_t()));
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, complete ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }
        
        if (hasGLExtension("GL_EXT_texture_filter_anisotropic")) {
            GLint aniso = 0;
            glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
            if (aniso > 0) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
            }
        }
    }
    
    void Texture::updateMipLevel(const std::shared_ptr<GLContext>& gl, int level, const nml::Texture& texture) {
        GLint glFormat = -1, glFormatInternal = -1;
        std::string glTextureData = texture.mipmaps(level);
        switch (texture.format()) {
        case nml::Texture::LUMINANCE8:
            glFormat = glFormatInternal = GL_LUMINANCE;
            break;
        case nml::Texture::RGB8:
            glFormat = glFormatInternal = GL_RGB;
            break;
        case nml::Texture::RGBA8:
            glFormat = glFormatInternal = GL_RGBA;
            break;
        case nml::Texture::ETC1:
            if (hasGLExtension("GL_OES_compressed_ETC1_RGB8_texture")) {
                GLuint size = 8 * ((texture.width() + 3) >> 2) * ((texture.height() + 3) >> 2);
                size_t offset = 16;
                glCompressedTexImage2D(GL_TEXTURE_2D, level, GL_ETC1_RGB8_OES, texture.width(), texture.height(), 0, size, &glTextureData[offset]);
                return;
            }
            {
                nml::Texture textureCopy(texture);
                uncompressTexture(textureCopy);
                updateMipLevel(gl, level, textureCopy);
            }
            return;
        case nml::Texture::PVRTC:
            if (hasGLExtension("GL_IMG_texture_compression_pvrtc")) {
                const PVRTextureHeaderV3 *header = reinterpret_cast<const PVRTextureHeaderV3 *>(&glTextureData[0]);
                GLuint format = 0;
                switch (header->u64PixelFormat)
                {
                case ePVRTPF_PVRTCI_2bpp_RGB:
                    format = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
                    break;
                case ePVRTPF_PVRTCI_4bpp_RGB:
                    format = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
                    break;
                case ePVRTPF_PVRTCI_2bpp_RGBA:
                    format = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
                    break;
                case ePVRTPF_PVRTCI_4bpp_RGBA:
                    format = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
                    break;
                }
    
                // Workaround for Apple-specific limitation - textures have to be squares (sic!)
                if (texture.width() == texture.height()) {
                    GLuint size = static_cast<GLuint>(glTextureData.size()) - PVRTEX3_HEADERSIZE;
                    glCompressedTexImage2D(GL_TEXTURE_2D, level, format, texture.width(), texture.height(), 0, size, &glTextureData[PVRTEX3_HEADERSIZE]);
                    return;
                }
            }
            {
                nml::Texture textureCopy(texture);
                uncompressTexture(textureCopy);
                updateMipLevel(gl, level, textureCopy);
            }
            return;
        default:
            assert(false);
        }
    
        if (!glTextureData.empty()) {
            glTexImage2D(GL_TEXTURE_2D, level, glFormatInternal, texture.width(), texture.height(), 0, glFormat, GL_UNSIGNED_BYTE, glTextureData.data());
        }
    }
    
    void Texture::updateMipMaps(const std::shared_ptr<GLContext>& gl, const nml::Texture& texture) {
        for (int i = 0; i < texture.mipmaps_size(); i++) {
            updateMipLevel(gl, i, texture);
        }
    }
    
    void Texture::uploadTexture(const std::shared_ptr<GLContext>& gl) {
        if (!_texture) {
            return;
        }
    
        const nml::Texture& texture = *_texture;
        if (texture.width() < 1 || texture.height() < 1) {
            return;
        }
    
        glGenTextures(1, &_glTextureId);
    
        glBindTexture(GL_TEXTURE_2D, _glTextureId);
        updateMipMaps(gl, texture);
        updateSampler(gl, texture.has_sampler(), texture.sampler(), texture.mipmaps_size() > 1);
    }
    
    void Texture::uncompressTexture(nml::Texture& texture) {
        switch (texture.format()) {
        case nml::Texture::ETC1:
            for (int i = 0; i < texture.mipmaps_size(); i++) {
                std::string textureData = texture.mipmaps(i);
                int etc1Width = (texture.width() + 3) & ~3, etc1Height = (texture.height() + 3) & ~3;
                std::vector<unsigned int> etc1Image(texture.width() * texture.height());
                size_t offset = 16;
                for (int y = 0; y + 4 <= etc1Height; y += 4) {
                    for (int x = 0; x + 4 <= etc1Width; x += 4) {
                        unsigned int block[4 * 4];
                        rg_etc1::unpack_etc1_block(&textureData[offset], block);
                        offset += 4 * 4 / 2;
                        for (int yb = 0; yb < 4; yb++) {
                            if (y + yb >= texture.height()) {
                                continue;
                            }
                            for (int xb = 0; xb < 4; xb++) {
                                if (x + xb >= texture.width()) {
                                    continue;
                                }
                                etc1Image[(y + yb) * texture.width() + x + xb] = block[yb * 4 + xb];
                            }
                        }
                    }
                }
                textureData.clear();
                if (!etc1Image.empty()) {
                    textureData.assign(reinterpret_cast<const char *>(&etc1Image[0]), reinterpret_cast<const char *>(&etc1Image[0] + etc1Image.size()));
                }
                texture.set_mipmaps(i, textureData);
            }
            texture.set_format(nml::Texture::RGBA8);
            break;
        
        case nml::Texture::PVRTC:
            for (int i = 0; i < texture.mipmaps_size(); i++) {
                std::string textureData = texture.mipmaps(i);
                const PVRTextureHeaderV3 *header = reinterpret_cast<const PVRTextureHeaderV3 *>(&textureData[0]);
                bool bpp2 = header->u64PixelFormat == ePVRTPF_PVRTCI_2bpp_RGB || header->u64PixelFormat == ePVRTPF_PVRTCI_2bpp_RGBA;
                std::vector<unsigned long> pvrtcImage(texture.width() * texture.height());
                PVRTDecompressPVRTC(&textureData[PVRTEX3_HEADERSIZE], bpp2, texture.width(), texture.height(), reinterpret_cast<unsigned char *>(&pvrtcImage[0]));
    
                textureData.clear();
                if (!pvrtcImage.empty()) {
                    textureData.assign(reinterpret_cast<const char *>(&pvrtcImage[0]), reinterpret_cast<const char *>(&pvrtcImage[0] + pvrtcImage.size()));
                }
                texture.set_mipmaps(i, textureData);
            }
            texture.set_format(nml::Texture::RGBA8);
            break;
        
        default:
            break;
        }
    }
    
} }
