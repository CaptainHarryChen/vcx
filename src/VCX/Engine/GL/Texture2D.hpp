#pragma once

#include "Engine/GL/Sampler.hpp"
#include "Engine/TextureND.hpp"

namespace VCX::Engine::GL {
    template<TextureFormat Format> inline constexpr GLenum InternalFormatEnumOf = 0;
    template<> inline constexpr GLenum InternalFormatEnumOf<Formats::R8G8B8>   = GL_RGB8;
    template<> inline constexpr GLenum InternalFormatEnumOf<Formats::R8G8B8A8> = GL_RGBA8;

    template<TextureFormat Format> inline constexpr GLenum FormatEnumOf = 0;
    template<> inline constexpr GLenum FormatEnumOf<Formats::R8G8B8>   = GL_RGB;
    template<> inline constexpr GLenum FormatEnumOf<Formats::R8G8B8A8> = GL_RGBA;

    template<TextureFormat Format> inline constexpr GLenum PixelTypeEnumOf = 0;
    template<> inline constexpr GLenum PixelTypeEnumOf<Formats::R8G8B8>   = GL_UNSIGNED_BYTE;
    template<> inline constexpr GLenum PixelTypeEnumOf<Formats::R8G8B8A8> = GL_UNSIGNED_BYTE;
    
    struct Texture2DTrait {
        static auto constexpr & CreateMany = glGenTextures;
        static auto constexpr & DeleteMany = glDeleteTextures;
        static auto constexpr & Bind       = glBindTexture;
        static GLenum constexpr BindTarget = GL_TEXTURE_2D;
    };

    class UniqueTexture2D : public Unique<Texture2DTrait> {
    public:
        explicit UniqueTexture2D(std::size_t const unit = 0) : _unit(unit) { }

        explicit UniqueTexture2D(SamplerOptions && options, std::size_t const unit = 0) :
            _unit(unit) {
            auto const useThis { Use() };
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GLenum(options.WrapU));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GLenum(options.WrapV));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GLenum(options.MinFilter));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GLenum(options.MagFilter));
        }
/*
        scope_t Use() const {
            glActiveTexture(GL_TEXTURE0 + _unit);
            ResourceMethods<Texture2DTrait>::Bind(Get());
            return scope::make_scope_exit([=]() {
                glActiveTexture(GL_TEXTURE0 + _unit);
                ResourceMethods<Texture2DTrait>::Unbind();
            });
        }
*/
        template<TextureFormat Format>
        void Resize(std::size_t const width, std::size_t const height) const {
            auto const useThis { Use() };
            glTexImage2D(
                GL_TEXTURE_2D, 0,
                InternalFormatEnumOf<Format>,
                width, height, 0,
                FormatEnumOf<Format>, PixelTypeEnumOf<Format>,
                nullptr);
        }

        template<TextureFormat Format>
        void Update(Engine::Texture2D<Format> const & texture) const {
            auto const useThis { Use() };
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(
                GL_TEXTURE_2D, 0,
                InternalFormatEnumOf<Format>,
                texture.GetSizeX(), texture.GetSizeY(), 0,
                FormatEnumOf<Format>, PixelTypeEnumOf<Format>,
                texture.GetBytes().data());
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        void GenerateMipmap() const {
            auto const useThis { Use() };
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        std::size_t GetUnit() const { return _unit; }
    
    private:
        std::size_t _unit = 0;
    };
}
